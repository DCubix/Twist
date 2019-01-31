#include "Node.h"

#include "intern/Log.h"

#include "NodeGraph.h"

Node::Node()
 :	m_solved(false),
	m_bufferPos(0),
	m_type(Utils::getTypeIndex<Node>()),
	m_lastSample(0.0f)
{
	m_buffer.fill(0.0f);
}

void Node::addInput(const Str& name, float def) {
	m_inputs.push_back(NodeInput(def));
	m_inputNames.push_back(name);
}

void Node::updateBuffer(float val) {
	m_buffer[m_bufferPos++ % TWEN_NODE_BUFFER_SIZE] = val;
}
