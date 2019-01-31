#include "TNodeGraph.h"

#include <algorithm>
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
	// TODO: Implement Loading
	m_saved = true;
	m_fileName = fileName;
}

void TNodeGraph::save(const std::string& fileName) {
	// TODO: Implement Saving

	m_saved = true;
	m_fileName = fileName;
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

void TNode::save(JSON& json) {
	json["type"] = node->name();
	json["open"] = open;
	json["pos"] = { bounds.x, bounds.y };
	json["selected"] = selected;
}

void TNode::load(JSON json) {
	open = json["open"];
	selected = json["selected"];
	bounds.x = json["pos"][0];
	bounds.y = json["pos"][1];
}
