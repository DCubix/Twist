#include "TNodeGraph.h"

TNodeFactories TNodeFactory::factories;

#include "nodes/TOutNode.hpp"
#include "nodes/TStorageNodes.hpp"

TNodeGraph::TNodeGraph() {
	m_name = "Untitled Node Graph";
	m_type = TGraphType::Normal;
	m_globalStorage.fill(0.0f);

}

TIntList TNodeGraph::getAllLinksRelatedToNode(int id) {
	TIntList links;
	for (int i = 0; i < m_links.size(); i++) {
		auto& lnk = m_links[i];
		auto pos = std::find(links.begin(), links.end(), i);
		if ((lnk->inputID == id || lnk->outputID == id) && pos == links.end()) {
			links.push_back(i);
		}
	}
	return links;
}

TIntList TNodeGraph::getNodeLinks(int id) {
	TIntList links;
	for (int i = 0; i < m_links.size(); i++) {
		TLink* lnk = m_links[i].get();
		if (lnk == nullptr) continue;
		if (lnk->inputID == id) {
			links.push_back(i);
		}
	}
	return links;
}

TIntList TNodeGraph::getNodeInputs(int id) {
	TIntList ins;
	for (auto& lnk : m_links) {
		if (lnk->outputID == id && node(lnk->inputID) != nullptr) {
			ins.push_back(lnk->inputID);
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
	nodes.insert(nodes.end(), ids.begin(), ids.end());

	for (int id : ids) {
		for (int in : getNodeInputs(id)) {
			if (node(in) == nullptr) continue;
			nodes.push_back(in);

			std::vector<int> rec = buildNodes(in);
			for (int rnd : rec) {
				if (node(rnd) == nullptr) continue;
				if (std::find(nodes.begin(), nodes.end(), rnd) == nodes.end()) {
					nodes.push_back(rnd);
				}
			}
		}
	}

	std::reverse(nodes.begin(), nodes.end());

	return nodes;
}

void TNodeGraph::solveNodes() {
	// // Evaluate writer nodes
	// for (auto& nd : m_nodes) {
	// 	if (nd.second->m_type != TWriterNode::type()) continue;
	// 	TIntList swn = buildNodes(nd.second->id());
	// 	m_solvedWriterNodes[nd.second->id()] = swn;
	// }

	TIntList outNodes;
	for (auto& nd : m_nodes) {
		if (nd.second->m_type != TWriterNode::type()) continue;
		outNodes.push_back(nd.second->id());
	}

	if (m_type == TGraphType::Module) {
		outNodes.push_back(outputsNode());
	} else {
		outNodes.push_back(outputNode());
	}
	m_solvedNodes = buildNodes(outNodes);
}

TNode* TNodeGraph::node(int id) {
	if (m_nodes.find(id) == m_nodes.end())
		return nullptr;
	return m_nodes[id].get();
}

void TNodeGraph::solveNodes(const TIntList& solved) {
	for (int id : solved) {
		TNode* nd = node(id);
		if (nd == nullptr) continue;

		nd->solve();

		for (int linkID : getNodeLinks(id)) {
			TLink* link = m_links[linkID].get();
			if (link == nullptr) continue;
			TNode* tgt = node(link->outputID);
			tgt->setInput(link->outputSlot, nd->getOutput(link->inputSlot));
		}
	}
}

float TNodeGraph::solve() {
	for (int id : m_solvedNodes) {
		TNode* nd = node(id);
		if (nd == nullptr) continue;
		// if (nd->m_type == TWriterNode::type()) continue;
		
		nd->solve();

		for (int linkID : getNodeLinks(id)) {
			TLink* link = m_links[linkID].get();
			if (link == nullptr) continue;
			TNode* tgt = node(link->outputID);
			tgt->setInput(link->outputSlot, nd->getOutput(link->inputSlot));
		}
	}

	// // Evaluate writer nodes
	// for (auto& e : m_solvedWriterNodes) {
	// 	solveNodes(e.second);
	// }

	if (m_type == TGraphType::Normal) {
		TNode* out = m_nodes[outputNode()].get();
		if (out->m_type == TOutNode::type()) {
			return out->getInput(0);
		}
	}

	return 0.0f;
}

int TNodeGraph::getID() {
	int id = 0;
	for (auto& n : m_nodes) {
		id = std::max(id, n.second->id());
	}
	return id + 1;
}

TNode* TNodeGraph::addNode(int x, int y, const std::string& type) {
	JSON params;
	return addNode(x, y, type, params);
}

TNode* TNodeGraph::addNode(int x, int y, const std::string& type, JSON& params) {
	TNodeCtor* ctor = TNodeFactory::factories[type];
	if (ctor == nullptr) return nullptr;

	std::unique_ptr<TNode> node = std::unique_ptr<TNode>(ctor(params));
	TNode* tnode = node.get();
	node->m_id = getID();
	node->m_bounds.x = x;
	node->m_bounds.y = y;
	node->m_parent = this;
	node->m_type = type;
	m_nodes.insert({ node->m_id, std::move(node) });

	if (type == TOutputsNode::type()) {
		m_outputsNode = tnode->m_id;
	}
	if (type == TInputsNode::type()) {
		m_inputsNode = tnode->m_id;
	}
	if (type == TOutNode::type()) {
		m_outputNode = tnode->m_id;
	}

	m_saved = false;

	return tnode;
}

void TNodeGraph::deleteNode(int id) {
	auto pos = m_nodes.find(id);
	if (pos != m_nodes.end()) {
		m_nodes.erase(pos);
	}

	TIntList brokenLinks;
	int i = 0;
	for (auto& link : m_links) {
		TLink* lnk = link.get();
		if (lnk == nullptr || node(lnk->outputID) == nullptr || node(lnk->inputID) == nullptr) {
			brokenLinks.push_back(i);
		}
		i++;
	}

	for (int lnk : brokenLinks) {
		m_links.erase(m_links.begin() + lnk);
	}

	solveNodes();

	m_saved = false;
}

void TNodeGraph::link(int inID, int inSlot, int outID, int outSlot) {
	TLink* link = new TLink();
	link->inputID = inID;
	link->inputSlot = inSlot;
	link->outputID = outID;
	link->outputSlot = outSlot;

	node(outID)->inputs()[outSlot].connected = true;
	node(inID)->outputs()[inSlot].connected = true;

	m_links.push_back(std::unique_ptr<TLink>(link));

	m_saved = false;

	solveNodes();
}

void TNodeGraph::load(const std::string& fileName) {
	JSON json;
	std::ifstream fp(fileName);
	fp >> json;
	fp.close();

	m_nodes.clear();
	m_links.clear();
	m_solvedNodes.clear();

	m_scrolling = ImVec2(json["scrolling"][0], json["scrolling"][1]);
	m_type = (TGraphType)json["graphType"];
	m_name = json["graphName"];
	// m_outputsNode = json["outputsNode"].is_number_integer() ? json["outputsNode"].get<int>() : 0;
	// m_inputsNode = json["inputsNode"].is_number_integer() ? json["inputsNode"].get<int>() : 0;
	// m_outputNode = json["outNode"].is_number_integer() ? json["outNode"].get<int>() : 0;

	for (int i = 0; i < json["nodes"].size(); i++) {
		JSON& node = json["nodes"][i];
		std::string type = node["type"].get<std::string>();
		TNode* nd = addNode(node["pos"][0], node["pos"][1], type, node);
		if (node["id"].is_number_integer())
			nd->m_id = node["id"];
	}

	for (int i = 0; i < json["links"].size(); i++) {
		JSON lnk = json["links"][i];
		if (lnk.is_null()) continue;
		link(lnk["inID"], lnk["inSlot"], lnk["outID"], lnk["outSlot"]);
	}

	m_saved = true;
	m_fileName = fileName;

	solveNodes();
}

void TNodeGraph::save(const std::string& fileName) {
	std::ofstream fp(fileName);
	JSON json;

	json["graphType"] = (int) m_type;
	json["graphName"] = m_name;
	json["scrolling"] = { m_scrolling.x, m_scrolling.y };

	// if (m_type == TGraphType::Normal) {
	// 	json["outNode"] = m_outputNode;
	// } else {
	// 	json["outputsNode"] = m_outputsNode;
	// 	json["inputsNode"] = m_inputsNode;
	// }

	int i = 0;
	for (auto& nd : m_nodes) {
		nd.second->save(json["nodes"][i++]);
	}

	i = 0;
	for (auto& link : m_links) {
		if (link.get() == nullptr) continue;

		JSON& links = json["links"][i++];
		links["inID"] = link->inputID;
		links["inSlot"] = link->inputSlot;
		links["outID"] = link->outputID;
		links["outSlot"] = link->outputSlot;
	}
	fp << json.dump(1, '\t', true);
	fp.close();

	m_saved = true;
	m_fileName = fileName;
}