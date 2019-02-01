#ifndef TWEN_NODE_GRAPH_H
#define TWEN_NODE_GRAPH_H

#include "intern/Utils.h"
#include "NodeRegistry.h"

#include <mutex>

#define TWEN_GLOBAL_STORAGE_SIZE 128

struct RawSample {
	Vec<float> data;
	float sampleRate;
	Str name;
};

class Node;
class NodeGraph {
public:
	NodeGraph();

	Node* add(Node *node);
	void remove(Node *node);

	Connection* getConnection(Node *node);
	bool isConnected(Node *node);
	bool isConnectedTo(Node *node, Node *to);
	Connection* connect(Node *from, Node *to, u32 slot);
	void disconnect(Connection *conn);
	Vec<Ptr<Connection>>& connections() { return m_connections; }

	Node* outputNode() { return m_outputNode; }

	void store(u32 loc, float value) { m_globalStorage[loc] = value; }
	float load(u32 loc) const { return m_globalStorage[loc]; }

	bool addSample(const Str& fileName);
	void removeSample(u64 id);
	u64 getSampleID(const Str& name);
	RawSample* getSample(u64 id);
	Map<u64, Ptr<RawSample>>& sampleLibrary() { return m_sampleLibrary; }
	Vec<Str> getSampleNames();

	float bpm() const { return m_bpm; }
	void bpm(float bpm) { m_bpm = bpm; }

	float sampleRate() const { return m_sampleRate; }

	float time();
	float sample();

private:
	void addSample(const Str& fname, const Vec<float>& data, float sr);

	Node *m_outputNode;

	Vec<Ptr<Node>> m_nodes;
	Vec<Ptr<Connection>> m_connections;

	std::mutex m_lock;

	float m_gain, m_time, m_sampleRate, m_bpm;

	Arr<float, TWEN_GLOBAL_STORAGE_SIZE> m_globalStorage;

	Vec<Str> m_sampleNames;
	Map<u64, Ptr<RawSample>> m_sampleLibrary;

};

#endif // TWEN_NODE_GRAPH_H
