#include "Node.h"

Node::Node()
 :	m_id(UID::getNew()),
	m_solved(false),
	m_default(0.0f)
{
	m_defaultArr.set(0.0f);
}

void Node::addInput(const Str& name) {
	m_inputs[name] = NodeSlot();
	m_inputs[name].values.set(0.0f);
	m_inputs[name].id = m_inputs.size()-1;
}

void Node::addOutput(const Str& name) {
	m_outputs[name] = NodeSlot();
	m_outputs[name].values.set(0.0f);
	m_outputs[name].id = m_outputs.size()-1;
}

void Node::removeInput(const Str& name) {
	auto pos = m_inputs.find(name);
	if (pos == m_inputs.end())
		return;
	m_inputs.erase(pos);
}

void Node::removeOutput(const Str& name) {
	auto pos = m_outputs.find(name);
	if (pos == m_outputs.end())
		return;
	m_outputs.erase(pos);
}