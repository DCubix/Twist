#include "Node.h"

Node::Node()
 :	m_id(UID::getNew()),
	m_solved(false)
{

}

void Node::addInput(const Str& name) {
	m_inputs[name] = FloatArray();
	m_inputs[name].set(0.0f);
}

void Node::addOutput(const Str& name) {
	m_outputs[name] = FloatArray();
	m_outputs[name].set(0.0f);
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