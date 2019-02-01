#include "TNodeGraph.h"

#include <algorithm>
#include <iomanip>
#include <utility>

#include "TCommands.h"
#include "TNodeEditor.h"

#include "sndfile.hh"

TNodeGraph::TNodeGraph(NodeGraph* ang) {
	m_actualNodeGraph = Ptr<NodeGraph>(std::move(ang));
	m_name = "Untitled";
	m_undoRedo = Ptr<TUndoRedo>(new TUndoRedo());
}

TNode* TNodeGraph::addNode(int x, int y,
	const Str& type, JSON params, bool canundo
) {
	LogI("Adding editor node '", type, "' at x=", x, "y=", y);
	TNode* n = new TNode();
	n->node = m_actualNodeGraph->add(NodeBuilder::createNode(type, params));
	n->bounds.x = x;
	n->bounds.y = y;
	n->gridPos.x = x;
	n->gridPos.y = y;
	n->open = true;

	m_tnodes.insert({ n->node, Ptr<TNode>(n) });

	m_saved = false;

	if (canundo) {
		m_undoRedo->performedAction<TAddNodeCommand>(this, m_tnodes[n->node].get(), x, y, type, params);
		LogI("Registered action: ", STR(TAddNodeCommand));
	}

	return m_tnodes[n->node].get();
}

void TNodeGraph::removeNode(TNode *nd, bool canundo) {
	LogI("Deleting editor node: ", nd->node->name());

	// Save node
	Str type = nd->node->name();
	int x = nd->bounds.x, y = nd->bounds.y;

	JSON params;
	params["pos"][0] = x;
	params["pos"][1] = y;
	params["open"][1] = nd->open;

	m_lock.lock();

	m_actualNodeGraph->remove(nd->node);

	auto pos = std::find_if(
		m_tnodes.begin(),
		m_tnodes.end(),
		[&nd](const std::pair<Node* const, Ptr<TNode>>& ptr){
			return ptr.second.get() == nd;
		}
	);

	if (pos != m_tnodes.end()) {
		m_tnodes.erase(pos);
		LogI("Editor Node was deleted successfully.");
	} else {
		LogE("Editor Node not found.");
	}

	m_lock.unlock();

	if (canundo) {
		m_undoRedo->performedAction<TDeleteNodeCommand>(this, nd, x, y, type, params);
		LogI("Registered action: ", STR(TDeleteNodeCommand));
	}

	m_saved = false;
}

Connection* TNodeGraph::connect(TNode *from, TNode *to, u32 slot, bool canundo) {
	LogI("Editor Linking");
	Connection* conn = m_actualNodeGraph->connect(from->node, to->node, slot);

	if (canundo) {
		m_undoRedo->performedAction<TLinkCommand>(this, conn, from->node, to->node, slot);
		LogI("Registered action: ", STR(TLinkCommand));
	}

	return conn;
}

void TNodeGraph::disconnect(Connection* conn, bool canundo) {
	LogI("Editor link removing...");
	if (canundo) {
		m_undoRedo->performedAction<TUnLinkCommand>(
			this,
			conn, conn->from, conn->to, conn->toSlot
		);
		LogI("Registered action: ", STR(TUnLinkCommand));
	}
	m_actualNodeGraph->disconnect(conn);

	m_saved = false;
}

void TNodeGraph::selectAll() {
	for (auto& e : m_tnodes) {
		e.second->selected = true;
	}
}

void TNodeGraph::unselectAll() {
	for (auto& e : m_tnodes) {
		e.second->selected = false;
	}
}

TNode* TNodeGraph::getActiveNode() {
	for (auto& e : m_tnodes) {
		if (e.second->selected) return e.second.get();
	}
	return nullptr;
}

void TNodeGraph::load(const Str& fileName) {
	JSON json;

	std::ifstream fp(fileName);
	if (fp.good()) {
		fp >> json;
		fromJSON(json);
		fp.close();

		m_saved = true;
		m_fileName = fileName;
	}
}

void TNodeGraph::save(const std::string& fileName) {
	JSON json; toJSON(json);

	std::ofstream fp(fileName);
	if (fp.good()) {
		fp << std::setw(4) << json << std::endl;
		fp.close();

		m_saved = true;
		m_fileName = fileName;
	}
}

void TNodeGraph::fromJSON(JSON json) {
	m_name = json["title"];
	m_scrolling.x = json["scroll"][0];
	m_scrolling.y = json["scroll"][1];

	m_tnodes.clear();
	m_undoRedo.reset(new TUndoRedo());

	// Load the nodes
	JSON nodes = json["nodes"];
	Map<u32, TNode*> idnodeMap;
	if (!nodes.is_array()) return;

	for (u32 i = 0; i < nodes.size(); i++) {
		JSON node = nodes[i];

		Str type = node["type"];
		float x = node["pos"][0];
		float y = node["pos"][1];

		TNode* n = addNode(x, y, type, node, false);
		n->node->load(node);
		n->open = node["open"];
		n->selected = node["selected"];
		idnodeMap[i] = n;
	}

	// Connect the nodes
	JSON connections = json["connections"];
	if (!connections.is_array()) return;

	for (u32 i = 0; i < connections.size(); i++) {
		JSON conn = connections[i];

		TNode *from = idnodeMap[conn["from"].get<u32>()];
		TNode *to = idnodeMap[conn["to"].get<u32>()];
		u32 slot = conn["slot"];
		connect(from, to, slot, false);
	}
}

void TNodeGraph::toJSON(JSON& json) {
	json["title"] = m_name;
	json["scroll"] = { m_scrolling.x, m_scrolling.y };

	// Save the Nodes
	JSON nodes = JSON::array();
	Map<Node*, u32> nodeidMap;

	u32 i = 0;
	for (auto&& [k, v] : m_tnodes) {
		JSON node; k->save(node);
		node["pos"] = { v->bounds.x, v->bounds.y };
		node["open"] = v->open;
		node["selected"] = v->selected;
		nodes[i] = node;
		nodeidMap[k] = i;
		i++;
	}
	json["nodes"] = nodes;

	// Save the Connections
	i = 0;
	JSON connections = JSON::array();

	for (auto&& conn : m_actualNodeGraph->connections()) {
		JSON jconn;
		jconn["from"] = nodeidMap[conn->from];
		jconn["to"] = nodeidMap[conn->to];
		jconn["slot"] = conn->toSlot;
		connections[i++] = jconn;
	}
	json["connections"] = connections;
}

Vec<const char*> TNodeGraph::getSampleNames() {
	Vec<const char*> sln;
	for (auto& sle : m_actualNodeGraph->sampleLibrary()) {
		char* title = new char[sle.second->name.size()];
		strcpy(title, sle.second->name.c_str());
		sln.push_back(title);
	}
	return sln;
}
