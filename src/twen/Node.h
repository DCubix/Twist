#ifndef TWEN_NODE_H
#define TWEN_NODE_H

#include "intern/Utils.h"
#include "intern/Vector.h"

#include <initializer_list>

#define STR(x) #x
#define TWEN_NODE(x, title) public: static Str type() { return STR(x); } \
									static TypeIndex typeID() { return Utils::getTypeIndex<x>(); } \
									static Str prettyName() { return title; }

#define TWEN_NODE_BUFFER_SIZE 256

class Node;
struct Connection {
	Node *from, *to;
	u32 toSlot;
};

struct Value {
	float value, velocity;
	bool gate;

	Value(float val = 0.0f, float vel = 1.0f, bool gate = true)
		: value(val), velocity(vel), gate(gate)
	{}
};

struct NodeInput {
	Value data;
	bool connected;
	u32 id;

	NodeInput(float value = 0.0f) : connected(false), id(0), data(value) {}

	float& value() { return data.value; }
	float& velocity() { return data.velocity; }
	bool& gate() { return data.gate; }
};

class NodeGraph;
class Node {
	friend class NodeGraph;
	friend class NodeBuilder;

	TWEN_NODE(Node, "Node")

public:
	Node();

	virtual Value sample(NodeGraph *graph) { return 0.0f; }

	virtual void save(JSON& json);
	virtual void load(JSON json);

	bool connected(u32 i) const { return m_inputs[i].connected; }
	NodeInput& in(u32 i) { return m_inputs[i]; }

	Vec<NodeInput> inputs() const { return m_inputs; }
	Vec<Str> inNames() const { return m_inputNames; }

	Str name() const { return m_name; }
	Str typeName() const { return m_typeName; }
	TypeIndex getType() const { return m_type; }

	Arr<float, TWEN_NODE_BUFFER_SIZE> buffer() { return m_buffer; }

	NodeGraph* graph() { return m_graph; }

protected:
	Str m_name, m_typeName;
	TypeIndex m_type;

	NodeGraph *m_graph;

	Vec<Str> m_inputNames;
	Vec<NodeInput> m_inputs;

	Arr<float, TWEN_NODE_BUFFER_SIZE> m_buffer;
	u32 m_bufferPos;

	bool m_solved;
	Value m_lastSample;

	void addInput(const Str& name, float def = 0.0f);
	void updateBuffer(float val);
};

#endif // TWEN_NODE_H
