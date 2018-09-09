#ifndef TWEN_NODE_H
#define TWEN_NODE_H

#include "intern/Utils.h"
#include "intern/Vector.h"

#include <initializer_list>

#define STR(x) #x
#define TWEN_NODE(x, title) public: static Str type() { return STR(x); } \
									static TypeIndex typeID() { return Utils::getTypeIndex<x>(); } \
									static Str prettyName() { return title; }

#define FLOAT_ARRAY_MAX 10
using FloatArray = Vector<FLOAT_ARRAY_MAX>;

struct NodeLink {
	u64 id;
	u64 inputID;
	u64 outputID;
	u32 inputSlot;
	u32 outputSlot;
};

struct NodeSlot {
	FloatArray values;
	bool connected;
	u32 id;

	FloatArray& operator* () { return values; }

	NodeSlot() : connected(false), id(0) { values.set(0.0f); }
};

struct NodeParam {
	enum ParamType {
		None = 0,
		Range,
		KnobRange,
		DragRange,
		Option
	};

	ParamType type;

	float min, max, step;
	float value;

	u32 option;
	Vec<Str> options;

	bool sameLine = false;
	i32 itemWidth = 100;

	NodeParam() {}
	NodeParam(
		ParamType type,
		float min, float max,
		float value, float step,
		const Vec<Str>& options,
		i32 itemWidth=100
	) : type(type), min(min), max(max), value(value), step(step), options(options), itemWidth(itemWidth)
	{ }

	NodeParam(
		ParamType type,
		float min, float max,
		u32 option,
		const Vec<Str>& options,
		i32 itemWidth=100
	) : type(type), min(min), max(max), option(option), options(options), itemWidth(itemWidth)
	{ }

	NodeParam(
		ParamType type,
		float min, float max,
		float value, float step, u32 option,
		const Vec<Str>& options,
		i32 itemWidth=100
	) : type(type), min(min), max(max), value(value), option(option), options(options), step(step), itemWidth(itemWidth)
	{ }
};

class NodeGraph;
class Node {
	friend class NodeGraph;
	friend class NodeBuilder;

	TWEN_NODE(Node, "Node")

public:
	Node();

	virtual void solve() {}

	virtual void save(JSON& json);
	virtual void load(JSON json);

	Vec<NodeSlot>& outputs() { return m_outputs; }
	Vec<NodeSlot>& inputs() { return m_inputs; }
	Vec<NodeParam>& params() { return m_params; }

	float& in(u32 input, u32 param = 0, u32 slot = 0);
	Str inName(u32 input);

	float& out(u32 output, u32 slot = 0);
	Str outName(u32 output);

	FloatArray& ins(u32 input, u32 param = 0, bool fill=false);
	FloatArray& outs(u32 output);

	float& param(u32 param);
	u32& paramOption(u32 param);
	Vec<const char*> paramOptions(u32 param);
	Str paramName(u32 param);

	u64 id() const { return m_id; }
	Str name() const { return m_name; }
	Str title() const { return m_title; }
	NodeGraph* parent() { return m_parent; }
	bool enabled() const { return m_enabled; }
	void enabled(bool e) { m_enabled = e; }

protected:
	bool isParamValid(u32 param) {
		return param >= 0 && param < m_params.size() && !m_params.empty();
	}

	bool isOutputValid(u32 output) {
		return output >= 0 && output < m_outputs.size() && !m_outputs.empty();
	}

	bool isInputValid(u32 input) {
		return input >= 0 && input < m_inputs.size() && !m_inputs.empty();
	}

	u64 m_id;

	bool m_solved, m_enabled = true;

	UMap<u32, Str> m_inputNames, m_outputNames;
	Vec<NodeSlot> m_inputs, m_outputs;

	UMap<u32, Str> m_paramNames;
	Vec<NodeParam> m_params;

	NodeGraph* m_parent;

	Str m_name, m_title;
	float m_default;
	FloatArray m_defaultArr;

	void addInput(const Str& name);
	void addOutput(const Str& name);
//	void removeInput(const Str& name);
//	void removeOutput(const Str& name);

	void addParam(const Str& name, float value, float step = 1.0f, bool sameLine = false, i32 w=70);

	void addParam(const Str& name,
				  float min, float max,
				  float value = 0.0f,
				  float step = 1.0f,
				  NodeParam::ParamType type = NodeParam::Range,
				  bool sameLine = false, i32 w=70);

	void addParam(const Str& name,
				  const std::initializer_list<Str>& options,
				  u32 option = 0,
				  bool sameLine = false, i32 w=70);
};

#endif // TWEN_NODE_H
