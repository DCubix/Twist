#include "TNodeGraph.h"

TNodeFactories TNodeFactory::factories;

#include "nodes/TOutNode.hpp"
#include "nodes/TStorageNodes.hpp"
#include "TCommands.h"

#include "sndfile.hh"

TNodeGraph::TNodeGraph() {
	m_name = "Untitled Node Graph";
	m_type = TGraphType::Normal;
	m_undoRedo = std::unique_ptr<TUndoRedo>(new TUndoRedo());
	m_globalStorage.fill(0.0f);
}

TIntList TNodeGraph::getAllLinksRelatedToNode(int id) {
	TIntList links;
	for (auto&& e : m_links) {
		TLink* lnk = e.second.get();
		if (lnk->inputID == id) {
			m_lock.lock();
			auto pos = std::find(links.begin(), links.end(), e.first);
			if (pos == links.end()) {
				links.push_back(e.first);
			}
			m_lock.unlock();
		}
	}
	return links;
}

TIntList TNodeGraph::getNodeInputs(int id) {
	TIntList ins;
	for (auto& e : m_links) {
		TLink* lnk = e.second.get();
		if (lnk->outputID == id && node(lnk->inputID) != nullptr) {
			m_lock.lock();
			ins.push_back(lnk->inputID);
			m_lock.unlock();
		}
	}
	return ins;
}

TIntList TNodeGraph::buildNodes(int id) {
	TIntList lst;
	lst.push_back(id);
	return buildNodes(lst);
}

TIntList TNodeGraph::buildNodes(const TIntList& ids) {
	if (ids.empty())
		return std::vector<int>();

	std::vector<int> nodes;
	m_lock.lock();
	nodes.insert(nodes.end(), ids.begin(), ids.end());
	m_lock.unlock();

	for (int id : ids) {
		for (int in : getNodeInputs(id)) {
			if (node(in) == nullptr) continue;
			m_lock.lock();
			nodes.push_back(in);
			m_lock.unlock();

			std::vector<int> rec = buildNodes(in);
			for (int rnd : rec) {
				if (node(rnd) == nullptr) continue;
				m_lock.lock();
				if (std::find(nodes.begin(), nodes.end(), rnd) == nodes.end()) {
					nodes.push_back(rnd);
				}
				m_lock.unlock();
			}
		}
	}

	m_lock.lock();
	std::reverse(nodes.begin(), nodes.end());
	m_lock.unlock();

	return nodes;
}

void TNodeGraph::solveNodes() {
	TIntList outNodes;
	for (auto& nd : m_nodes) {
		if (nd.second->m_type != TWriterNode::type()) continue;
		m_lock.lock();
		outNodes.push_back(nd.second->id());
		m_lock.unlock();
	}

	if (m_type == TGraphType::Module) {
		TNode* n = node(outputsNode());
		if (n != nullptr && n->m_type == TOutputsNode::type())
			outNodes.push_back(outputsNode());
	} else {
		TNode* n = node(outputNode());
		if (n != nullptr && n->m_type == TOutNode::type())
			outNodes.push_back(outputNode());
	}

	m_solvedNodes = buildNodes(outNodes);
}

TNode* TNodeGraph::node(int id) {
	if (m_nodes.find(id) == m_nodes.end())
		return nullptr;
	return m_nodes[id].get();
}

TLink* TNodeGraph::link(int id) {
	if (m_links.find(id) == m_links.end())
		return nullptr;
	return m_links[id].get();
}

void TNodeGraph::solveNodes(const TIntList& solved) {
	for (int id : solved) {
		TNode* nd = node(id);
		if (nd == nullptr) continue;
		nd->m_solved = false;
	}

	for (int id : solved) {
		TNode* nd = node(id);
		if (nd == nullptr) continue;

		if (!nd->m_solved) {
			nd->solve();
			nd->m_solved = true;
		}
			
		for (auto&& e : m_links) {
			TLink* lnk = e.second.get();
			if (lnk == nullptr) continue;
			if (lnk->inputID == id) {
				TNode* tgt = node(lnk->outputID);
				if (tgt == nullptr) continue;

				for (int k = 0; k < TNODE_MAX_SIMULTANEOUS_VALUES_PER_SLOT; k++)
					tgt->setMultiInput(lnk->outputSlot, k, nd->getMultiOutput(lnk->inputSlot, k));
			}
		}
	}
}

float TNodeGraph::solve() {
	solveNodes(m_solvedNodes);

	if (m_type == TGraphType::Normal) {
		TNode* out = node(outputNode());
		if (out != nullptr && out->m_type == TOutNode::type()) {
			return out->getInput(0);
		}
	}

	return 0.0f;
}

int TNodeGraph::getID() {
	int id = 0;
	for (auto& n : m_nodes) {
		if (n.second == nullptr) continue;
		id = std::max(id, n.second->id());
	}
	return id + 1;
}

int TNodeGraph::getLinkID() {
	int id = 0;
	for (auto& n : m_links) {
		if (n.second == nullptr) continue;
		id = std::max(id, n.first);
	}
	return id + 1;
}

TNode* TNodeGraph::addNode(int x, int y, const std::string& type) {
	JSON params;
	return addNode(x, y, type, params);
}

TNode* TNodeGraph::addNode(int x, int y, const std::string& type, JSON& params, int id, bool canundo) {
	TNodeCtor* ctor = TNodeFactory::factories[type];
	if (ctor == nullptr) return nullptr;

	std::unique_ptr<TNode> n = std::unique_ptr<TNode>(ctor(params));
	int nid = id == -1 ? getID() : id;
	n->m_id = nid;
	n->m_bounds.x = x;
	n->m_bounds.y = y;
	n->m_gridPosition.x = x;
	n->m_gridPosition.y = y;
	n->m_parent = this;
	n->m_type = type;

	m_nodes.insert({ nid, std::move(n) });

	if (type == TOutputsNode::type()) {
		m_outputsNode = nid;
	}
	if (type == TInputsNode::type()) {
		m_inputsNode = nid;
	}
	if (type == TOutNode::type()) {
		m_outputNode = nid;
	}

	TNode* nd = node(nid);
	nd->setup();

	m_saved = false;

	solveNodes();

	if (canundo)
		m_undoRedo->performedAction<TAddNodeCommand>(this, nid,	x, y, type, params);

	return nd;
}

void TNodeGraph::deleteNode(int id, bool canundo) {
	TIntList brokenLinks = getAllLinksRelatedToNode(id);

	TNode* nd = node(id);
	std::string type = nd->type();
	int x = nd->m_bounds.x, y = nd->m_bounds.y;
	JSON params;
	nd->save(params);

	m_lock.lock();
	auto pos = m_nodes.find(id);
	if (pos != m_nodes.end()) {
		m_nodes.erase(pos);
	}
	m_lock.unlock();

	// Find some more broken links...
	for (auto&& e : m_links) {
		TLink* lnk = e.second.get();
		if (node(lnk->inputID) == nullptr || node(lnk->outputID) == nullptr) {
			if (std::find(brokenLinks.begin(), brokenLinks.end(), e.first) == brokenLinks.end())
				brokenLinks.push_back(e.first);
		}
	}

	std::sort(brokenLinks.begin(), brokenLinks.end());
	for (int i = brokenLinks.size() - 1; i >= 0; i--) {
		m_links.erase(brokenLinks[i]);
	}

	solveNodes();

	if (canundo)
		m_undoRedo->performedAction<TDeleteNodeCommand>(this, id, x, y, type, params);

	m_saved = false;
}

int TNodeGraph::link(int inID, int inSlot, int outID, int outSlot, bool canundo) {
	TLink* link = new TLink();
	link->inputID = inID;
	link->inputSlot = inSlot;
	link->outputID = outID;
	link->outputSlot = outSlot;

	node(outID)->inputs()[outSlot].connected = true;
	node(inID)->outputs()[inSlot].connected = true;

	m_lock.lock();
	int id = getLinkID();
	m_links[id] = std::unique_ptr<TLink>(link);
	m_lock.unlock();

	m_saved = false;

	solveNodes();

	if (canundo)
		m_undoRedo->performedAction<TLinkCommand>(this, id, inID, inSlot, outID, outSlot);

	return id;
}

void TNodeGraph::removeLink(int id, bool canundo) {
	TLink* lnk = link(id);
	if (lnk == nullptr) return;

	node(lnk->outputID)->inputs()[lnk->outputSlot].connected = false;
	node(lnk->inputID)->outputs()[lnk->inputSlot].connected = false;

	if (canundo) {
		m_undoRedo->performedAction<TUnLinkCommand>(
			this,
			id, lnk->inputID, lnk->inputSlot,
			lnk->outputID, lnk->outputSlot
		);
	}

	m_lock.lock();
	m_links.erase(id);
	m_lock.unlock();

	m_saved = false;
}

void TNodeGraph::load(const std::string& fileName) {
	JSON json;
	std::ifstream fp(fileName);
	fp >> json;
	fp.close();

	m_nodes.clear();
	m_links.clear();
	m_solvedNodes.clear();
	m_sampleLibrary.clear();
	m_globalStorage.fill(0.0f);

	m_scrolling = ImVec2(json["scrolling"][0], json["scrolling"][1]);
	m_type = (TGraphType)json["graphType"];
	m_name = json["graphName"];
	// m_outputsNode = json["outputsNode"].is_number_integer() ? json["outputsNode"].get<int>() : 0;
	// m_inputsNode = json["inputsNode"].is_number_integer() ? json["inputsNode"].get<int>() : 0;
	// m_outputNode = json["outNode"].is_number_integer() ? json["outNode"].get<int>() : 0;

	if (json["nodes"].is_array()) {
		for (int i = 0; i < json["nodes"].size(); i++) {
			JSON& node = json["nodes"][i];
			if (node.is_null()) continue;

			std::string type = node["type"].get<std::string>();

			int id = node["id"].is_number_integer() ? node["id"].get<int>() : -1;
			addNode(node["pos"][0], node["pos"][1], type, node, id);
		}
	}

	if (json["links"].is_array()) {
		for (int i = 0; i < json["links"].size(); i++) {
			JSON lnk = json["links"][i];
			if (lnk.is_null()) continue;

			link(lnk["inID"], lnk["inSlot"], lnk["outID"], lnk["outSlot"]);
		}
	}

	// Load sample lib
	if (json["sampleLibrary"].is_array()) {
		for (int i = 0; i < json["sampleLibrary"].size(); i++) {
			JSON libe = json["sampleLibrary"][i];
			if (libe.is_null()) continue;

			addSample(libe["name"], libe["samples"], libe["sampleRate"], libe["duration"]);
		}
	}

	m_saved = true;
	m_fileName = fileName;

	solveNodes();
}

void TNodeGraph::selectAll() {
	for (auto& e : m_nodes) {
		e.second->m_selected = true;
	}
}

void TNodeGraph::unselectAll() {
	for (auto& e : m_nodes) {
		e.second->m_selected = false;
	}
}

int TNodeGraph::getActiveNode() {
	for (auto& e : m_nodes) {
		if (e.second->m_selected) return e.first;
	}
	return -1;
}

void TNodeGraph::save(const std::string& fileName) {
	std::ofstream fp(fileName);
	JSON json;

	json["graphType"] = (int) m_type;
	json["graphName"] = m_name;
	json["scrolling"] = { m_scrolling.x, m_scrolling.y };

	int i = 0;
	for (auto& nd : m_nodes) {
		nd.second->save(json["nodes"][i++]);
	}

	i = 0;
	for (auto& e : m_links) {
		TLink* link = e.second.get();
		if (link == nullptr) continue;
		JSON& links = json["links"][i++];
		links["inID"] = link->inputID;
		links["inSlot"] = link->inputSlot;
		links["outID"] = link->outputID;
		links["outSlot"] = link->outputSlot;
	}

	// Save sample library
	i = 0;
	for (auto& se : m_sampleLibrary) {
		if (se.second.get() == nullptr) continue;
		JSON& je = json["sampleLibrary"][i++];
		je["name"] = se.second->name;
		je["samples"] = se.second->sampleData;
		je["sampleRate"] = se.second->sampleRate;
		je["duration"] = se.second->duration;
	}

	fp << json.dump(1, '\t');
	fp.close();

	m_saved = true;
	m_fileName = fileName;
}

void TNodeGraph::addSample(const std::string& fname, const std::vector<float>& data, float sr, float dur) {
	int id = 0;
	for (auto& se : m_sampleLibrary) {
		id = std::max(id, se.first);
	}
	id++;

	std::unique_ptr<TSampleLibEntry> entry = std::unique_ptr<TSampleLibEntry>(new TSampleLibEntry());
	entry->sampleData = data;
	entry->sampleRate = sr;
	entry->duration = dur;
	entry->name = fname;
	m_sampleLibrary[id] = std::move(entry);
}

bool TNodeGraph::addSample(const std::string& fileName) {
	auto pos = fileName.find_last_of('/');
	if (pos == std::string::npos) {
		pos = fileName.find_last_of('\\');
	}

	float sr = 1, dur = 0;
	std::vector<float> sampleData;

	SndfileHandle snd = SndfileHandle(fileName);
	int maxSecs = 10;
	if (snd.samplerate() > 44100) {
		maxSecs = 5;
	}
	if (snd.frames() < snd.samplerate() * maxSecs) {
		sr = snd.samplerate();
		dur = float(double(snd.frames()) / sr);

		std::vector<float> samplesRaw;
		samplesRaw.resize(snd.frames() * snd.channels());

		snd.readf(samplesRaw.data(), samplesRaw.size());
		
		sampleData.resize(snd.frames());
		for (int i = 0; i < snd.frames(); i++) {
			sampleData[i] = samplesRaw[i];
		}
	}

	if (dur <= 0.0f || sampleData.empty()) {
		return false;
	}

	addSample(fileName.substr(pos+1), sampleData, sr, dur);

	return true;
}

void TNodeGraph::removeSample(int id) {
	auto pos = m_sampleLibrary.find(id);
	if (pos == m_sampleLibrary.end())
		return;
	m_sampleLibrary.erase(id);
}

TSampleLibEntry* TNodeGraph::getSample(int id) {
	if (m_sampleLibrary.find(id) == m_sampleLibrary.end())
		return nullptr;
	return m_sampleLibrary[id].get();
}

int TNodeGraph::getSampleID(const std::string& name) {
	for (auto& se : m_sampleLibrary) {
		if (se.second->name == name) {
			return se.first;
		}
	}
	return 0;
}

std::vector<const char*> TNodeGraph::getSampleNames() {
	std::vector<const char*> sln;
	for (auto& sle : m_sampleLibrary) {
		sln.push_back(sle.second->name.c_str());
	}
	return sln;
}