#include "TNodeGraph.h"

#include "TCommands.h"
#include "TNodeEditor.h"

#include "sndfile.hh"

TNodeGraph::TNodeGraph(NodeGraph* ang) {
	m_actualNodeGraph = Ptr<NodeGraph>(std::move(ang));
	m_name = "Untitled";
	m_undoRedo = Ptr<TUndoRedo>(new TUndoRedo());
}

TNodeUI* TNodeGraph::node(u64 id) {
	if (m_nodes.find(id) == m_nodes.end())
		return nullptr;
	return m_nodes[id].get();
}

NodeLink* TNodeGraph::link(u64 id) {
	if (m_actualNodeGraph->links().find(id) == m_actualNodeGraph->links().end())
		return nullptr;
	return m_actualNodeGraph->links()[id].get();
}

TNodeUI* TNodeGraph::addNode(
	int x, int y,
	const Str& type, JSON params,
	u64 id, bool canundo
) {
	TNodeUI* n = new TNodeUI();
	n->node = m_actualNodeGraph->create(type, params, id);
	n->bounds.x = x;
	n->bounds.y = y;
	n->gridPos.x = x;
	n->gridPos.y = y;
	n->open = true;

	m_nodes[n->node->id()] = Ptr<TNodeUI>(std::move(n));

	m_saved = false;

	if (canundo) {
		m_undoRedo->performedAction<TAddNodeCommand>(this, id, x, y, type, params);
	}

	return n;
}

void TNodeGraph::deleteNode(u64 id, bool canundo) {
	// Save node
	TNodeUI* nd = node(id);
	Str type = nd->node->name();
	int x = nd->bounds.x, y = nd->bounds.y;
	JSON params;
	params["pos"][0] = x;
	params["pos"][1] = y;
	params["open"][1] = nd->open;

	m_lock.lock();
	auto pos = m_nodes.find(id);
	if (pos != m_nodes.end()) {
		m_nodes.erase(pos);
	}
	m_lock.unlock();

	if (canundo) {
		m_undoRedo->performedAction<TDeleteNodeCommand>(this, id, x, y, type, params);
	}

	m_saved = false;
}

u64 TNodeGraph::link(u64 inID, const Str& inSlot, u64 outID, const Str& outSlot, bool canundo) {
	u64 id = m_actualNodeGraph->link(inID, inSlot, outID, outSlot);

	if (canundo) {
		m_undoRedo->performedAction<TLinkCommand>(this, id, inID, inSlot, outID, outSlot);
	}

	return id;
}

void TNodeGraph::removeLink(u64 id, bool canundo) {
	NodeLink* lnk = m_actualNodeGraph->link(id);
	if (canundo) {
		m_undoRedo->performedAction<TUnLinkCommand>(
			this,
			id, lnk->inputID, lnk->inputSlot,
			lnk->outputID, lnk->outputSlot
		);
	}

	m_actualNodeGraph->removeLink(id);

	m_saved = false;
}

void TNodeGraph::selectAll() {
	for (auto& e : m_nodes) {
		e.second->selected = true;
	}
}

void TNodeGraph::unselectAll() {
	for (auto& e : m_nodes) {
		e.second->selected = false;
	}
}

u64 TNodeGraph::getActiveNode() {
	for (auto& e : m_nodes) {
		if (e.second->selected) return e.first;
	}
	return 0;
}

void TNodeGraph::load(const Str& fileName) {
	JSON json;
	std::ifstream fp(fileName);
	fp >> json;
	fp.close();

	m_nodes.clear();
	m_solvedNodes.clear();

	m_scrolling = ImVec2(json["scrolling"][0], json["scrolling"][1]);
	m_name = json["graphName"];
	m_actualNodeGraph->fromJSON(json);

	int i = 0;
	for (auto&& nd : m_actualNodeGraph->nodes()) {
		m_nodes[nd.first] = Ptr<TNodeUI>(new TNodeUI());
		TNodeUI* no = m_nodes[nd.first].get();
		no->bounds.x = int(json["nodes"][i]["pos"][0]);
		no->bounds.y = int(json["nodes"][i]["pos"][1]);
		no->open = json["nodes"][i]["open"];
		i++;
	}

	m_saved = true;
	m_fileName = fileName;
}

void TNodeGraph::save(const std::string& fileName) {
	JSON json;

	json["scrolling"] = { m_scrolling.x, m_scrolling.y };
	m_actualNodeGraph->toJSON(json);

	int i = 0;
	for (auto&& nd : m_nodes) {
		if (m_editor->snapToGrid()) {
			nd.second->bounds.x = nd.second->gridPos.x;
			nd.second->bounds.y = nd.second->gridPos.y;
		}
		json["nodes"][i]["pos"][0] = nd.second->bounds.x;
		json["nodes"][i]["pos"][1] = nd.second->bounds.y;
		json["nodes"][i]["open"] = nd.second->open;
		i++;
	}

	std::ofstream fp(fileName);
	fp << json.dump(1, '\t');
	fp.close();

	m_saved = true;
	m_fileName = fileName;
}

Vec<const char*> TNodeGraph::getSampleNames() {
	Vec<const char*> sln;
	for (auto& sle : m_actualNodeGraph->sampleLibrary()) {
		sln.push_back(sle.second->name.c_str());
	}
	return sln;
}