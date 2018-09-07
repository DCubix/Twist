#ifndef TWEN_NODE_H
#define TWEN_NODE_H

#include "intern/Utils.h"
#include "intern/Vector.h"

#define STR(x) #x
#define TWEN_NODE(x) public: static Str type() { return STR(x); } \
					 		 static TypeIndex typeID() { return Utils::getTypeIndex<x>(); } \
					 		 Str name() { return x::type(); }

#define FLOAT_ARRAY_MAX 10
using FloatArray = Vector<FLOAT_ARRAY_MAX>;

struct NodeLink {
	int inputID;
	int outputID;
	Str inputSlot;
	Str outputSlot;
};

class NodeGraph;
class Node {
	friend class NodeGraph;

	TWEN_NODE(Node)

public:
	Node();

	virtual void solve() {}

	float getMultiOutput(const Str& name, int pos) { return m_outputs[name][pos]; }
	float getMultiInput(const Str& name, int pos) { return m_inputs[name][pos]; }
	float setMultiOutput(const Str& name, int pos, float value) { m_outputs[name][pos] = value; }
	float setMultiInput(const Str& name, int pos, float value) { m_inputs[name][pos] = value; }

	FloatArray& outputs(const Str& name) { return m_outputs[name]; }
	FloatArray& inputs(const Str& name) { return m_inputs[name]; }
	Map<Str, FloatArray>& outputs() { return m_outputs; }
	Map<Str, FloatArray>& inputs() { return m_inputs; }

	float setMultiOutputValues(const Str& name, const FloatArray& v) { m_outputs[name] = v; }
	float setMultiInputValues(const Str& name, const FloatArray& v) { m_inputs[name] = v; }

	float getOutput(const Str& name) { return getMultiOutput(name, 0); }
	float getInput(const Str& name) { return getMultiInput(name, 0); }

	void setOutput(const Str& name, float value, bool fill=false) {
		if (fill) {
			m_outputs[name].set(value);
		} else {
			setMultiOutput(name, 0, value);
		}
	}
	void setInput(const Str& name, float value) { setMultiInput(name, 0, value); }

	u64 id() const { return m_id; }
	NodeGraph* parent() { return m_parent; }

protected:
	u64 m_id;

	bool m_solved;
	Map<Str, FloatArray> m_inputs, m_outputs;
	FloatArray m_defaultValues;

	NodeGraph* m_parent;

	void addInput(const Str& name);
	void addOutput(const Str& name);
	void removeInput(const Str& name);
	void removeOutput(const Str& name);
};

#endif // TWEN_NODE_H
