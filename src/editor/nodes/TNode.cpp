#include "TNode.h"

void TNode::addInput(const std::string& label) {
	m_inputs.push_back(TValue(label));
}

void TNode::addOutput(const std::string& label) {
	m_outputs.push_back(TValue(label));
}

void TNode::save(JSON& json) {
	json["pos"] = { m_bounds.x, m_bounds.y };
	json["open"] = open;
	json["id"] = m_id;
}
