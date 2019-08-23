#include "NodeGraph.h"

#include <algorithm>
#include <fstream>

#include "TAudio.h"
#include "nodes/StorageNodes.hpp"

#include "Node.h"
#include "nodes/OutNode.hpp"

NodeGraph::NodeGraph()
	: m_gain(1.0f), m_time(0.0f),
	  m_sampleRate(44100.0f), m_bpm(120.0f),
	  m_outputNode(nullptr),
	  m_bars(4), m_noteIndex(0)
{
	m_globalStorage.fill(Value());
}

float NodeGraph::time() {
	return m_time / delay();
}

Node* NodeGraph::add(Node *node) {
	if (node == nullptr) return nullptr;

	node->m_graph = this;
	m_nodes.push_back(Ptr<Node>(node));
	if (node->getType() == OutNode::typeID()) {
		m_outputNode = m_nodes.back().get();
	}
	return m_nodes.back().get();
}

void NodeGraph::remove(Node *node) {
	if (node == nullptr) return;

	auto pos = std::find_if(
		m_nodes.begin(),
		m_nodes.end(),
		[&](const Ptr<Node>& ptr){
			return ptr.get() == node;
		}
	);

	if (pos != m_nodes.end()) {
		u32 i = 0;
		Vec<u32> toRemove;
		for (auto&& conn : m_connections) {
			if (conn->from == node || conn->to == node) {
				toRemove.push_back(i);
			}
			i++;
		}

		std::sort(toRemove.begin(), toRemove.end());
		std::reverse(toRemove.begin(), toRemove.end());
		for (u32 idx : toRemove) {
			m_connections[idx]->to->m_inputs[m_connections[idx]->toSlot].connected = false;
			m_connections.erase(m_connections.begin() + idx);
		}

		m_nodes.erase(pos);
	}
}

Connection* NodeGraph::getConnection(Node *node) {
	for (auto&& conn : m_connections) {
		if (conn->from == node || conn->to == node) return conn.get();
	}
	return nullptr;
}

bool NodeGraph::isConnected(Node *node) {
	return getConnection(node) != nullptr;
}

bool NodeGraph::isConnectedTo(Node *node, Node *to) {
	for (auto&& conn : m_connections) {
		if (conn->from == node || conn->to == to ||
			conn->to == node || conn->from == to)
			return true;
	}
	return false;
}

Connection* NodeGraph::connect(Node *from, Node *to, u32 slot) {
	Connection *conn = new Connection();
	conn->from = from;
	conn->to = to;
	conn->toSlot = slot;
	m_connections.push_back(Ptr<Connection>(conn));

	to->m_inputs[slot].connected = true;

	return m_connections.back().get();
}

void NodeGraph::disconnect(Connection *conn) {
	if (conn == nullptr) return;

	auto pos = std::find_if(
		m_connections.begin(),
		m_connections.end(),
		[&](const Ptr<Connection>& ptr){
			return ptr.get() == conn;
		}
	);

	if (pos != m_connections.end()) {
		conn->to->m_inputs[conn->toSlot].connected = false;
		m_connections.erase(pos);
	}
}

float NodeGraph::sample() {
	if (m_outputNode == nullptr) {
		for (auto&& node : m_nodes) {
			if (node->getType() == OutNode::typeID()) {
				m_outputNode = node.get();
				break;
			}
		}
	}

	// Reset
	for (auto&& node : m_nodes) {
		node->m_solved = false;
	}

	u32 i = 0;
	Vec<u32> toRemove;
	for (auto&& conn : m_connections) {
		// Cleanup
		if (conn->from == nullptr || conn->to == nullptr) {
			toRemove.push_back(i);
			continue;
		}
		i++;
		//

		Value sample = conn->from->m_lastSample;
		if (!conn->from->m_solved) {
			sample = conn->from->sample(this);
			conn->from->m_solved = true;
			conn->from->m_lastSample = sample;
		}

		// If it's a writer node, solve "to"
		if ((conn->to->getType() == WriterNode::typeID() || conn->to->getType() == OutNode::typeID()) &&
			!conn->to->m_solved) {
			Value tosample = conn->to->sample(this);
			conn->to->m_solved = true;
			conn->to->m_lastSample = tosample;

			conn->to->updateBuffer(tosample.value * tosample.velocity * float(tosample.gate));
		}

		conn->from->updateBuffer(sample.value * sample.velocity * float(sample.gate));

		conn->to->in(conn->toSlot).data = sample;
	}

	// Cleanup
	if (!toRemove.empty()) {
		std::sort(toRemove.begin(), toRemove.end());
		std::reverse(toRemove.begin(), toRemove.end());
		for (u32 idx : toRemove) {
			m_connections.erase(m_connections.begin() + idx);
			LogI("Cleaned up invalid connection ", idx);
		}
	}
	//

	const float step = (1.0f / m_sampleRate) * 4.0f;

	m_time += step;
	if (m_time >= delay()) {
		m_noteIndex++;
		m_noteIndex %= (m_bars * 4);
		m_time = 0.0f;
	}

	Value smp = m_outputNode != nullptr ? m_outputNode->sample(this) : Value();
	return smp.value;
}

void NodeGraph::reset() {
	m_noteIndex = 0;
	m_time = 0.0f;
}

void NodeGraph::addSample(const Str& fname, const Vec<float>& data, float sr) {
	Ptr<RawSample> entry = Ptr<RawSample>(new RawSample());
	entry->data = data;
	entry->sampleRate = sr;
	entry->name = fname;
	m_sampleLibrary[fname] = std::move(entry);
}

bool NodeGraph::addSample(const Str& fileName) {
	auto pos = fileName.find_last_of('/');
	if (pos == std::string::npos) {
		pos = fileName.find_last_of('\\');
	}

	float sr = 1;
	std::vector<float> sampleData;

	TAudioFile snd(fileName);
	int maxSecs = 15;
	if (snd.sampleRate() > 44100) {
		maxSecs = 10;
	}
	if (snd.frames() < snd.sampleRate() * maxSecs) {
		sr = snd.sampleRate();

		std::vector<float> samplesRaw;
		samplesRaw.resize(snd.frames() * snd.channels());

		u32 interleavedFrames = snd.readf(&samplesRaw[0], samplesRaw.size());

		sampleData.resize(interleavedFrames);
		std::fill(sampleData.begin(), sampleData.end(), 0.0f);

		u32 s = 0;
		for (int i = 0; i < interleavedFrames; i++) {
			sampleData[i] = samplesRaw[i * snd.channels()];
		}
	}

	if (sampleData.empty()) {
		return false;
	}

	addSample(fileName.substr(pos+1), sampleData, sr);

	return true;
}

void NodeGraph::removeSample(const Str& name) {
	auto pos = m_sampleLibrary.find(name);
	if (pos == m_sampleLibrary.end())
		return;
	m_sampleLibrary.erase(pos);
}

RawSample* NodeGraph::getSample(const Str& name) {
	if (m_sampleLibrary.empty())
		return nullptr;
	auto pos = m_sampleLibrary.find(name);
	if (pos == m_sampleLibrary.end())
		return nullptr;
	return pos->second.get();
}

Vec<Str> NodeGraph::getSampleNames() {
	Vec<Str> sampleNames;
	sampleNames.reserve(m_sampleLibrary.size());
	for (auto&& [k, v] : m_sampleLibrary)
		sampleNames.push_back(k);
	return sampleNames;
}
