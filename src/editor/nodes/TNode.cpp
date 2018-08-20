#include "TNode.h"

void TNode::addInput(const std::string& label) {
	m_inputs.push_back(TValue(label));
}

void TNode::addOutput(const std::string& label) {
	m_outputs.push_back(TValue(label));
}

bool TNode::removeInput(int id) {
	m_inputs.erase(m_inputs.begin() + id);
	return true;
}

bool TNode::removeOutput(int id) {
	m_outputs.erase(m_outputs.begin() + id);
	return true;
}

void TNode::save(JSON& json) {
	json["pos"] = { m_bounds.x, m_bounds.y };
	json["open"] = open;
	json["id"] = m_id;
}
