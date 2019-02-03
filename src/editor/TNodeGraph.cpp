#include "TNodeGraph.h"

#include <algorithm>
#include <iomanip>
#include <utility>

#include "TCommands.h"
#include "TNodeEditor.h"

#include "nodes/OutNode.hpp"

#include "sndfile.hh"

TNodeGraph::TNodeGraph(NodeGraph* ang, int outX, int outY) {
	m_actualNodeGraph = Ptr<NodeGraph>(std::move(ang));
	m_name = "Untitled";
	m_undoRedo = Ptr<TUndoRedo>(new TUndoRedo());

	JSON json;
	json["gain"] = 1.0f;
	TNode *out = addNode(outX, outY, "OutNode", json, false);
	out->closeable = false;
}

TNode* TNodeGraph::addNode(int x, int y,
	const Str& type, JSON params, bool canundo
) {
	LogI("Adding editor node '", type, "' at x=", x, "y=", y);
	TNode* n = new TNode();
	n->node = m_actualNodeGraph->add(NodeBuilder::createNode(type, params));
	n->closeable = true;
	n->bounds.x = x;
	n->bounds.y = y;
	n->gridPos.x = x;
	n->gridPos.y = y;
	n->open = true;

	m_lock.lock();
	m_tnodes.insert({ n->node, Ptr<TNode>(n) });
	m_lock.unlock();

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
	m_lock.lock();
	Connection* conn = m_actualNodeGraph->connect(from->node, to->node, slot);
	m_lock.unlock();

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
	m_lock.lock();
	m_actualNodeGraph->disconnect(conn);
	m_lock.unlock();

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

	float bpm = json.value("bpm", 120.0f);
	u32 bars = json.value("bars", 4);

	m_actualNodeGraph->bpm(bpm);
	m_actualNodeGraph->bars(bars);

	m_undoRedo.reset(new TUndoRedo());

	TNode* outNode = m_tnodes.begin()->second.get();
	outNode->bounds.x = json["outPos"][0];
	outNode->bounds.y = json["outPos"][1];
	outNode->gridPos.x = json["outPos"][0];
	outNode->gridPos.y = json["outPos"][1];

	// Load the samples
	JSON samples = json["samples"];
	if (samples.is_array()) {
		for (u32 i = 0; i < samples.size(); i++) {
			JSON jsample = samples[i];

			Vec<float> data = jsample["data"];
			Str sampleName = jsample["sampleName"];
			float sampleRate = jsample["sampleRate"];

			m_actualNodeGraph->addSample(sampleName, data, sampleRate);
		}
	}

	// Load the nodes
	JSON nodes = json["nodes"];
	Map<u32, TNode*> idnodeMap;
	if (!nodes.is_array()) return;

	idnodeMap[0] = outNode;

	u32 nodeID = 1;
	for (u32 i = 0; i < nodes.size(); i++) {
		JSON node = nodes[i];

		Str type = node["type"];
		float x = node["pos"][0];
		float y = node["pos"][1];

		TNode* n = addNode(x, y, type, node, false);
		n->node->load(node);
		n->open = node["open"];
		n->selected = node["selected"];
		idnodeMap[nodeID++] = n;
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

	TNode* outNode = m_tnodes.begin()->second.get();
	json["outPos"] = { outNode->gridPos.x, outNode->gridPos.y };

	json["bpm"] = m_actualNodeGraph->bpm();
	json["bars"] = m_actualNodeGraph->bars();

	// Save the Nodes
	JSON nodes = JSON::array();
	Map<Node*, u32> nodeidMap;

	u32 i = 0;
	for (auto&& [k, v] : m_tnodes) {
		if (k->getType() == OutNode::typeID()) continue;

		JSON node; k->save(node);
		node["pos"] = { v->gridPos.x, v->gridPos.y };
		node["open"] = v->open;
		node["selected"] = v->selected;
		nodes[i] = node;
		nodeidMap[k] = i+1;
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

	// Save samples
	JSON samples = JSON::array();
	for (auto&& [id, sample] : m_actualNodeGraph->sampleLibrary()) {
		JSON jsample;
		jsample["data"] = sample->data;
		jsample["sampleSize"] = sample->data.size();
		jsample["sampleName"] = id;
		jsample["sampleRate"] = sample->sampleRate;
		samples.push_back(jsample);
	}
	json["samples"] = samples;
}
