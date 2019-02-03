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

	void store(u32 loc, Value value) { m_globalStorage[loc] = value; }
	Value load(u32 loc) const { return m_globalStorage[loc]; }

	bool addSample(const Str& fileName);
	void removeSample(const Str& name);
	RawSample* getSample(const Str& name);
	Map<Str, Ptr<RawSample>>& sampleLibrary() { return m_sampleLibrary; }
	Vec<Str> getSampleNames();

	float bpm() const { return m_bpm; }
	void bpm(float bpm) { m_bpm = bpm; }

	u32 index() const { return m_noteIndex; }

	u32 bars() const { return m_bars; }
	void bars(u32 b) { m_bars = b; }

	float sampleRate() const { return m_sampleRate; }

	float time();
	float sample();

	void reset();

	void addSample(const Str& fname, const Vec<float>& data, float sr);
private:
	Node *m_outputNode;

	Vec<Ptr<Node>> m_nodes;
	Vec<Ptr<Connection>> m_connections;

	std::mutex m_lock;

	float m_gain, m_time, m_sampleRate, m_bpm;
	u32 m_noteIndex, m_bars;

	Arr<Value, TWEN_GLOBAL_STORAGE_SIZE> m_globalStorage;

	Map<Str, Ptr<RawSample>> m_sampleLibrary;

};

#endif // TWEN_NODE_GRAPH_H
