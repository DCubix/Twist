#include "NodeGraph.h"

#include <algorithm>
#include <fstream>

#include "sndfile.hh"
#include "nodes/StorageNodes.hpp"

#include "Node.h"
#include "nodes/OutNode.hpp"

NodeGraph::NodeGraph()
	: m_gain(1.0f), m_time(0.0f),
	  m_sampleRate(44100.0f), m_bpm(120.0f),
	  m_outputNode(nullptr),
	  m_bars(4), m_noteIndex(0)
{
	m_globalStorage.fill(0.0f);
}

float NodeGraph::time() {
	const float delay = (60000.0f / m_bpm) / 1000.0f;
	return Utils::remap(m_time, 0.0f, delay, 0.0f, 1.0f);
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
	const float delay = (60000.0f / m_bpm) / 1000.0f;

	m_time += step;
	if (m_time >= delay) {
		m_noteIndex++;
		m_noteIndex %= (m_bars * 4);
		m_time = 0.0f;
	}

	Value s = m_outputNode != nullptr ? m_outputNode->sample(this) : Value();
	return s.value;
}

void NodeGraph::addSample(const Str& fname, const Vec<float>& data, float sr) {
	u64 id = UID::getNew();

	Ptr<RawSample> entry = Ptr<RawSample>(new RawSample());
	entry->data = data;
	entry->sampleRate = sr;
	entry->name = fname;
	m_sampleLibrary[id] = std::move(entry);
	m_sampleNames.push_back(fname);
}

bool NodeGraph::addSample(const Str& fileName) {
	auto pos = fileName.find_last_of('/');
	if (pos == std::string::npos) {
		pos = fileName.find_last_of('\\');
	}

	float sr = 1;
	std::vector<float> sampleData;

	SndfileHandle snd = SndfileHandle(fileName);
	int maxSecs = 15;
	if (snd.samplerate() > 44100) {
		maxSecs = 10;
	}
	if (snd.frames() < snd.samplerate() * maxSecs) {
		sr = snd.samplerate();

		std::vector<float> samplesRaw;
		samplesRaw.resize(snd.frames() * snd.channels());
		snd.readf(samplesRaw.data(), samplesRaw.size());

		sampleData.resize(snd.frames());
		for (int i = 0; i < snd.frames(); i++) {
			sampleData[i] = samplesRaw[0 + i * 2] * 0.5f;
			sampleData[i] += samplesRaw[1 + i * 2] * 0.5f;
		}
	}

	if (sampleData.empty()) {
		return false;
	}

	addSample(fileName.substr(pos+1), sampleData, sr);

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
	auto pos = m_sampleLibrary.find(id);
	if (pos == m_sampleLibrary.end())
		return nullptr;
	return pos->second.get();
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
