#include "NodeGraph.h"

#include <algorithm>
#include <fstream>

#include "sndfile.hh"
#include "nodes/StorageNodes.hpp"

Vec<u64> NodeGraph::getAllLinksRelatedToNode(u64 node) {
	Vec<u64> links;
	for (auto&& lnk : m_links) {
		if (lnk->inputID == node) {
			m_lock.lock();
			auto pos = std::find(links.begin(), links.end(), lnk->id);
			if (pos == links.end()) {
				links.push_back(lnk->id);
			}
			m_lock.unlock();
		}
	}
	return links;
}

Vec<Node*> NodeGraph::getNodeInputs(Node* node) {
	Vec<Node*> ins;
	for (auto&& lnk : m_links) {
		if (lnk->outputID == node->id()) {
			m_lock.lock();
			ins.push_back(get<Node>(lnk->inputID));
			m_lock.unlock();
		}
	}
	return ins;
}

Vec<Node*> NodeGraph::buildNodes(Node* node) {
	Vec<Node*> lst;
	lst.push_back(node);
	return buildNodes(lst);
}

Vec<Node*> NodeGraph::buildNodes(const Vec<Node*>& nodes) {
	if (nodes.empty())
		return Vec<Node*>();

	Vec<Node*> nodesG;
	m_lock.lock();
	nodesG.insert(nodesG.end(), nodes.begin(), nodes.end());
	m_lock.unlock();

	for (Node* node : nodes) {
		for (Node* in : getNodeInputs(node)) {
			if (in == nullptr) continue;

			m_lock.lock();
			nodesG.push_back(in);
			m_lock.unlock();

			for (Node* rnd : buildNodes(in)) {
				if (rnd == nullptr) continue;

				m_lock.lock();
				if (std::find(nodesG.begin(), nodesG.end(), rnd) == nodesG.end()) {
					nodesG.push_back(rnd);
				}
				m_lock.unlock();
			}
		}
	}

	m_lock.lock();
	std::reverse(nodesG.begin(), nodesG.end());
	m_lock.unlock();

	return nodesG;
}

void NodeGraph::solveNodes() {
	Vec<Node*> outNodes;
	for (auto&& nd : m_nodes) {
		if (nd->name() != WriterNode::type()) continue;
		m_lock.lock();
		outNodes.push_back(nd.get());
		m_lock.unlock();
	}

	// if (m_type == TGraphType::Module) {
	// 	TNode* n = node(outputsNode());
	// 	if (n != nullptr && n->m_type == TOutputsNode::type())
	// 		outNodes.push_back(outputsNode());
	// } else {
	Node* n = get<Node>(outputNode());
	if (n != nullptr && n->name() == OutNode::type())
		outNodes.push_back(n);
	// }

	m_solvedNodes = buildNodes(outNodes);
}

void NodeGraph::solveNodes(const Vec<Node*>& solved) {
	for (Node* nd : solved) {
		nd->m_solved = false;
	}

	for (Node* nd : solved) {
		if (!nd->m_solved && nd->enabled()) {
			nd->solve();
			nd->m_solved = true;
		}

		for (Ptr<NodeLink>& lnk : m_links) {
			if (lnk.get() == nullptr) continue;

			if (lnk->inputID == nd->id()) {
				Node* tgt = get<Node>(lnk->outputID);
				if (tgt == nullptr) continue;

				if (nd->enabled() && tgt->inputs()[lnk->outputSlot].connected) {
					tgt->ins(lnk->outputSlot).set(nd->outs(lnk->inputSlot));
				} else {
					nd->outs(lnk->inputSlot).set(nd->ins(0));
					tgt->ins(lnk->outputSlot).set(nd->outs(lnk->inputSlot));
				}
			}
		}
	}
}

void NodeGraph::remove(u64 id) {
	Vec<u64> brokenLinks = getAllLinksRelatedToNode(id);

	std::sort(m_nodes.begin(), m_nodes.end(), NodeSortComparator);
	auto pos = std::lower_bound(m_nodes.begin(), m_nodes.end(), id, NodeComparator);
	if (pos != m_nodes.end()) {
		m_lock.lock();
		m_nodes.erase(pos);
		m_lock.unlock();
	}

	// Find some more broken links...
	for (auto&& lnk : m_links) {
		if (get<Node>(lnk->inputID) == nullptr || get<Node>(lnk->outputID) == nullptr) {
			if (std::find(brokenLinks.begin(), brokenLinks.end(), lnk->id) == brokenLinks.end())
				brokenLinks.push_back(lnk->id);
		}
	}

	std::sort(brokenLinks.begin(), brokenLinks.end());
	for (int i = brokenLinks.size() - 1; i >= 0; i--) {
		m_links.erase(m_links.begin() + brokenLinks[i]);
	}

	solveNodes();
}

u64 NodeGraph::link(u64 inID, u32 inSlot, u64 outID, u32 outSlot) {
	NodeLink* link = new NodeLink();
	link->inputID = inID;
	link->inputSlot = inSlot;
	link->outputID = outID;
	link->outputSlot = outSlot;
	link->id = m_links.size();

	get<Node>(outID)->inputs()[outSlot].connected = true;
	get<Node>(inID)->outputs()[inSlot].connected = true;

	u64 id = link->id;
	m_lock.lock();
	m_links.push_back(Ptr<NodeLink>(std::move(link)));
	m_lock.unlock();

	solveNodes();

	return id;
}

void NodeGraph::removeLink(u64 id) {
	NodeLink* lnk = link(id);
	if (lnk == nullptr) return;

	get<Node>(lnk->outputID)->inputs()[lnk->outputSlot].connected = false;
	get<Node>(lnk->inputID)->outputs()[lnk->inputSlot].connected = false;

	std::sort(m_links.begin(), m_links.end(), LinkSortComparator);
	auto pos = std::lower_bound(m_links.begin(), m_links.end(), id, LinkComparator);

	if (pos != m_links.end()) {
		m_lock.lock();
		m_links.erase(pos);
		m_lock.unlock();
	}
}

NodeLink* NodeGraph::link(u64 id) {
	auto pos = std::lower_bound(m_links.begin(), m_links.end(), id, LinkComparator);
	if (pos == m_links.end())
		return nullptr;
	return pos->get();
}

float NodeGraph::solve() {
	solveNodes(m_solvedNodes);

	// if (m_type == GraphType::Normal) {
	Node* out = get<Node>(outputNode());
	if (out != nullptr && out->name() == OutNode::type()) {
		return out->in(0);
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

	if (json["nodes"].is_object()) {
		for (auto&& o : json["nodes"].items()) {
			JSON node = o.value();
			if (node.is_null()) continue;

			Str type = node["type"].get<Str>();
			create(type, node)->load(node);
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
	for (auto&& nd : m_nodes) {
		nd->save(json["nodes"][std::to_string(nd->id())]);
	}

	int i = 0;
	for (auto&& link : m_links) {
		if (link.get() == nullptr) continue;
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
