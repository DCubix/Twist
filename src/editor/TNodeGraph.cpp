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
	auto pos = m_nodes.find(id);
	if (pos == m_nodes.end())
		return nullptr;
	return pos->second.get();
}

NodeLink* TNodeGraph::link(u64 id) {
	return m_actualNodeGraph->link(id);
}

TNodeUI* TNodeGraph::addNode(int x, int y,
	const Str& type, JSON params, bool canundo
) {
	LogI("Adding editor node '", type, "' at x=", x, "y=", y);
	TNodeUI* n = new TNodeUI();
	n->node = m_actualNodeGraph->create(type, params);
	n->bounds.x = x;
	n->bounds.y = y;
	n->gridPos.x = x;
	n->gridPos.y = y;
	n->open = true;

	u64 id = n->node->id();
	m_nodes[id] = Ptr<TNodeUI>(std::move(n));

	m_saved = false;

	if (canundo) {
		m_undoRedo->performedAction<TAddNodeCommand>(this, id, x, y, type, params);
		LogI("Registered action: ", STR(TAddNodeCommand));
	}

	return n;
}

void TNodeGraph::deleteNode(u64 id, bool canundo) {
	LogI("Deleting editor node: ", id);
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
		LogI("Editor Node was deleted successfully.");
	} else {
		LogE("Editor Node not found.");
	}

	m_actualNodeGraph->remove(id);
	m_lock.unlock();

	if (canundo) {
		m_undoRedo->performedAction<TDeleteNodeCommand>(this, id, x, y, type, params);
		LogI("Registered action: ", STR(TDeleteNodeCommand));
	}

	m_saved = false;
}

u64 TNodeGraph::link(u64 inID, u32 inSlot, u64 outID, u32 outSlot, bool canundo) {
	LogI("Editor Linking...");
	u64 id = m_actualNodeGraph->link(inID, inSlot, outID, outSlot);

	if (canundo) {
		m_undoRedo->performedAction<TLinkCommand>(this, id, inID, inSlot, outID, outSlot);
		LogI("Registered action: ", STR(TLinkCommand));
	}

	return id;
}

void TNodeGraph::removeLink(u64 id, bool canundo) {
	LogI("Editor link removing...");
	NodeLink* lnk = m_actualNodeGraph->link(id);
	if (canundo) {
		m_undoRedo->performedAction<TUnLinkCommand>(
			this,
			id, lnk->inputID, lnk->inputSlot,
			lnk->outputID, lnk->outputSlot
		);
		LogI("Registered action: ", STR(TUnLinkCommand));
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

	for (auto&& nd : m_actualNodeGraph->nodes()) {
		Str key = std::to_string(nd->id());
		m_nodes[nd->id()] = Ptr<TNodeUI>(new TNodeUI());
		TNodeUI* no = m_nodes[nd->id()].get();
		no->bounds.x = int(json["nodes"][key]["pos"][0]);
		no->bounds.y = int(json["nodes"][key]["pos"][1]);
		no->open = json["nodes"][key]["open"].is_boolean() ? json["nodes"][key]["open"].get<bool>() : true;
		no->node = nd.get();
		no->gridPos.x = no->bounds.x;
		no->gridPos.y = no->bounds.y;
	}

	m_saved = true;
	m_fileName = fileName;
}

void TNodeGraph::save(const std::string& fileName) {
	JSON json;

	json["scrolling"] = { m_scrolling.x, m_scrolling.y };
	json["graphName"] = m_name;
	m_actualNodeGraph->toJSON(json);

	for (auto&& nd : m_nodes) {
		if (m_editor->snapToGrid()) {
			nd.second->bounds.x = nd.second->gridPos.x;
			nd.second->bounds.y = nd.second->gridPos.y;
		}
		Str key = std::to_string(nd.first);
		json["nodes"][key]["pos"][0] = nd.second->bounds.x;
		json["nodes"][key]["pos"][1] = nd.second->bounds.y;
		json["nodes"][key]["open"] = nd.second->open;
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
		char* title = new char[sle.second->name.size()];
		strcpy(title, sle.second->name.c_str());
		sln.push_back(title);
	}
	return sln;
}
