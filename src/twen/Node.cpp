#include "Node.h"

Node::Node()
 :	m_id(UID::getNew()),
	m_solved(false),
	m_default(0.0f)
{
	m_defaultArr.set(0.0f);
}

void Node::addInput(const Str& name) {
	m_inputs.add(name, NodeSlot());
	m_inputs[name].values.set(0.0f);
	m_inputs[name].id = m_inputs.size()-1;
}

void Node::addOutput(const Str& name) {
	m_outputs.add(name, NodeSlot());
	m_outputs[name].values.set(0.0f);
	m_outputs[name].id = m_outputs.size()-1;
}

void Node::removeInput(const Str& name) {
	if (!m_inputs.has(name))
		return;
	m_inputs.remove(name);
}

void Node::removeOutput(const Str& name) {
	if (!m_outputs.has(name))
		return;
	m_outputs.remove(name);
}