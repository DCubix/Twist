#include "NodeGraph.h"

#include <algorithm>
#include <fstream>

#include "sndfile.hh"
#include "nodes/StorageNodes.hpp"

Vec<u64> NodeGraph::getAllLinksRelatedToNode(u64 id) {
	Vec<u64> links;
	for (auto&& e : m_links) {
		NodeLink* lnk = e.second.get();
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

Vec<u64> NodeGraph::getNodeInputs(u64 id) {
	Vec<u64> ins;
	for (auto& e : m_links) {
		NodeLink* lnk = e.second.get();
		if (lnk->outputID == id && get<Node>(lnk->inputID) != nullptr) {
			m_lock.lock();
			ins.push_back(lnk->inputID);
			m_lock.unlock();
		}
	}
	return ins;
}

Vec<u64> NodeGraph::buildNodes(u64 id) {
	Vec<u64> lst;
	lst.push_back(id);
	return buildNodes(lst);
}

Vec<u64> NodeGraph::buildNodes(const Vec<u64>& ids) {
	if (ids.empty())
		return Vec<u64>();

	Vec<u64> nodes;
	m_lock.lock();
	nodes.insert(nodes.end(), ids.begin(), ids.end());
	m_lock.unlock();

	for (int id : ids) {
		for (int in : getNodeInputs(id)) {
			if (get<Node>(in) == nullptr) continue;

			m_lock.lock();
			nodes.push_back(in);
			m_lock.unlock();

			Vec<u64> rec = buildNodes(in);
			for (int rnd : rec) {
				if (get<Node>(rnd) == nullptr) continue;

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

void NodeGraph::solveNodes() {
	Vec<u64> outNodes;
	for (auto& nd : m_nodes) {
		if (nd.second->name() != WriterNode::type()) continue;
		m_lock.lock();
		outNodes.push_back(nd.second->id());
		m_lock.unlock();
	}

	// if (m_type == TGraphType::Module) {
	// 	TNode* n = node(outputsNode());
	// 	if (n != nullptr && n->m_type == TOutputsNode::type())
	// 		outNodes.push_back(outputsNode());
	// } else {
	Node* n = get<Node>(outputNode());
	if (n != nullptr && n->name() == OutNode::type())
		outNodes.push_back(outputNode());
	// }

	m_solvedNodes = buildNodes(outNodes);
}

void NodeGraph::solveNodes(const Vec<u64>& solved) {
	for (int id : solved) {
		Node* nd = get<Node>(id);
		if (nd == nullptr) continue;
		nd->m_solved = false;
	}

	for (int id : solved) {
		Node* nd = get<Node>(id);
		if (nd == nullptr) continue;

		if (!nd->m_solved) {
			nd->solve();
			nd->m_solved = true;
		}
			
		for (auto&& e : m_links) {
			NodeLink* lnk = e.second.get();
			if (lnk == nullptr) continue;

			if (lnk->inputID == id) {
				Node* tgt = get<Node>(lnk->outputID);
				if (tgt == nullptr) continue;

				tgt->ins(lnk->outputSlot).set(nd->outs(lnk->inputSlot));
			}
		}
	}
}

void NodeGraph::remove(u64 id) {
	Vec<u64> brokenLinks = getAllLinksRelatedToNode(id);

	m_lock.lock();
	auto pos = m_nodes.find(id);
	if (pos != m_nodes.end()) {
		m_nodes.erase(pos);
	}
	m_lock.unlock();

	// Find some more broken links...
	for (auto&& e : m_links) {
		NodeLink* lnk = e.second.get();
		if (get<Node>(lnk->inputID) == nullptr || get<Node>(lnk->outputID) == nullptr) {
			if (std::find(brokenLinks.begin(), brokenLinks.end(), e.first) == brokenLinks.end())
				brokenLinks.push_back(e.first);
		}
	}

	std::sort(brokenLinks.begin(), brokenLinks.end());
	for (int i = brokenLinks.size() - 1; i >= 0; i--) {
		m_links.erase(brokenLinks[i]);
	}

	solveNodes();
}

u64 NodeGraph::link(u64 inID, const Str& inSlot, u64 outID, const Str& outSlot) {
	NodeLink* link = new NodeLink();
	link->inputID = inID;
	link->inputSlot = inSlot;
	link->outputID = outID;
	link->outputSlot = outSlot;

	get<Node>(outID)->inputs(outSlot).connected = true;
	get<Node>(inID)->outputs(inSlot).connected = true;

	u64 id = UID::getNew();
	m_lock.lock();
	m_links[id] = Ptr<NodeLink>(std::move(link));
	m_lock.unlock();

	solveNodes();

	return id;
}

void NodeGraph::removeLink(u64 id) {
	NodeLink* lnk = link(id);
	if (lnk == nullptr) return;

	get<Node>(lnk->outputID)->inputs()[lnk->outputSlot].connected = false;
	get<Node>(lnk->inputID)->outputs()[lnk->inputSlot].connected = false;

	m_lock.lock();
	m_links.erase(id);
	m_lock.unlock();
}

NodeLink* NodeGraph::link(u64 id) {
	if (m_links.find(id) == m_links.end())
		return nullptr;
	return m_links[id].get();
}

float NodeGraph::solve() {
	solveNodes(m_solvedNodes);

	// if (m_type == GraphType::Normal) {
	Node* out = get<Node>(outputNode());
	if (out != nullptr && out->name() == OutNode::type()) {
		return out->in("In");
	}
	// }

	return 0.0f;
}

void NodeGraph::addSample(const Str& fname, const Vec<float>& data, float sr, float dur) {
	u64 id = UID::getNew();

	Ptr<RawSample> entry = Ptr<RawSample>(new RawSample());
	entry->data = data;
	entry->sampleRate = sr;
	entry->duration = dur;
	entry->name = fname;
	m_sampleLibrary[id] = std::move(entry);
	m_sampleNames.push_back(fname);
}

bool NodeGraph::addSample(const Str& fileName) {
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

void NodeGraph::removeSample(u64 id) {
	auto pos = m_sampleLibrary.find(id);
	if (pos == m_sampleLibrary.end())
		return;

	Str name = m_sampleLibrary[id]->name;
	auto npos = std::find(m_sampleNames.begin(), m_sampleNames.end(), name);
	if (npos != m_sampleNames.end()) {
		m_sampleNames.erase(npos);
	}
	m_sampleLibrary.erase(id);
}

RawSample* NodeGraph::getSample(u64 id) {
	if (m_sampleLibrary.empty())
		return nullptr;
	if (m_sampleLibrary.find(id) == m_sampleLibrary.end())
		return nullptr;
	return m_sampleLibrary[id].get();
}

u64 NodeGraph::getSampleID(const Str& name) {
	for (auto& se : m_sampleLibrary) {
		if (se.second->name == name) {
			return se.first;
		}
	}
	return 0;
}

Vec<Str> NodeGraph::getSampleNames() {
	return m_sampleNames;
}

void NodeGraph::fromJSON(JSON json) {
	m_nodes.clear();
	m_links.clear();
	m_solvedNodes.clear();
	m_sampleLibrary.clear();
	m_globalStorage.fill(0.0f);

	// m_type = (GraphType) json["graphType"];

	if (json["nodes"].is_array()) {
		for (int i = 0; i < json["nodes"].size(); i++) {
			JSON& node = json["nodes"][i];
			if (node.is_null()) continue;

			Str type = node["type"].get<Str>();

			u64 id = node["id"].is_number_integer() ? node["id"].get<u64>() : -1;
			Node* no = create(type, node, id);
			no->load(node);
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

	solveNodes();
}

void NodeGraph::toJSON(JSON& json) {
	int i = 0;
	for (auto& nd : m_nodes) {
		nd.second->save(json["nodes"][i++]);
	}

	i = 0;
	for (auto& e : m_links) {
		NodeLink* link = e.second.get();
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
		je["samples"] = se.second->data;
		je["sampleRate"] = se.second->sampleRate;
		je["duration"] = se.second->duration;
	}

}