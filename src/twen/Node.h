#ifndef TWEN_NODE_H
#define TWEN_NODE_H

#include "intern/Utils.h"
#include "intern/Vector.h"
#include "intern/Dict.h"

#include <initializer_list>

#define STR(x) #x
#define TWEN_NODE(x, title) public: static Str type() { return STR(x); } \
					 				static TypeIndex typeID() { return Utils::getTypeIndex<x>(); } \
					 				static Str prettyName() { return title; }

#define FLOAT_ARRAY_MAX 10
using FloatArray = Vector<FLOAT_ARRAY_MAX>;

struct NodeLink {
	int inputID;
	int outputID;
	Str inputSlot;
	Str outputSlot;
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

	void save(JSON& json) {
		json["id"] = m_id;
		json["type"] = m_name;
		for (const Str& k : m_params.keys()) {
			NodeParam param = m_params[k];
			json["params"][k]["value"] = param.value;
			json["params"][k]["option"] = param.option;
		}
	}

	void load(JSON json) {
		m_id = json["id"].is_number_unsigned() ? json["id"].get<u64>() : m_id;
		m_name = json["type"].is_string() ? json["type"].get<Str>() : m_name;
		if (json["params"].is_object()) {
			for (auto e : json["params"].items()) {
				JSON param = e.value();
				m_params[e.key()].value = !param["value"].is_number_float() ? 0.0f : param["value"].get<float>();
				m_params[e.key()].option = !param["option"].is_number_unsigned() ? 0 : param["option"].get<u32>();
			}
		}
	}

	NodeSlot& outputs(const Str& name) { return m_outputs[name]; }
	NodeSlot& inputs(const Str& name) { return m_inputs[name]; }
	Dict<NodeSlot>& outputs() { return m_outputs; }
	Dict<NodeSlot>& inputs() { return m_inputs; }
	Dict<NodeParam>& params() { return m_params; }

	float& in(const Str& name, const Str& param = "", i32 index = 0) {
		if (!m_inputs[name].connected) {
			if (!param.empty() && m_params.has(param)) {
				return m_params[param].value;
			} else {
				return m_default;
			}
		}
		return m_inputs[name].values[index];
	}

	float& out(const Str& name, i32 index = 0) {
		if (!m_outputs[name].connected)
			return m_default;
		return m_outputs[name].values[index];
	}

	FloatArray& ins(const Str& name, const Str& param = "") {
		if (!m_inputs[name].connected) {
			if (!param.empty() && m_params.has(param)) {
				m_defaultArr.set(0.0f);
				m_defaultArr[0] = m_params[param].value;
				return m_defaultArr;
			} else {
				m_defaultArr.set(0.0f);
				return m_defaultArr;
			}
		}
		return m_inputs[name].values;
	}

	FloatArray& outs(const Str& name) {
		if (!m_outputs[name].connected) {
			m_defaultArr.set(0.0f);
			return m_defaultArr;
		}
		return m_outputs[name].values;
	}

	float& param(const Str& name) {
		return m_params[name].value;
	}

	u32& paramOption(const Str& name) {
		return m_params[name].option;
	}

	Vec<const char*> paramOptions(const Str& name) {
		Vec<const char*> ops;
		ops.resize(m_params[name].options.size());
		for (i32 i = 0; i < ops.size(); i++) {
			char* title = new char[m_params[name].options[i].size()];
			strcpy(title, m_params[name].options[i].c_str());
			ops[i] = title;
		}
		return ops;
	}

	u64 id() const { return m_id; }
	Str name() const { return m_name; }
	Str title() const { return m_title; }
	NodeGraph* parent() { return m_parent; }

protected:
	u64 m_id;

	bool m_solved;
	Dict<NodeSlot> m_inputs, m_outputs;
	Dict<NodeParam> m_params;

	NodeGraph* m_parent;

	Str m_name, m_title;
	float m_default;
	FloatArray m_defaultArr;

	void addInput(const Str& name);
	void addOutput(const Str& name);
	void removeInput(const Str& name);
	void removeOutput(const Str& name);

	void addParam(const Str& name, float value, float step = 1.0f, bool sameLine = false, i32 w=70) {
		m_params.add(name, NodeParam(NodeParam::None, 0.0f, 0.0f, value, step, Vec<Str>(), w));
		m_params[name].sameLine = sameLine;
	}

	void addParam(const Str& name,
				  float min, float max,
				  float value = 0.0f,
				  float step = 1.0f,
				  NodeParam::ParamType type = NodeParam::Range,
				  bool sameLine = false, i32 w=70)
	{
		m_params.add(name, NodeParam(type, min, max, value, step, Vec<Str>(), w));
		m_params[name].sameLine = sameLine;
	}

	void addParam(const Str& name,
				  const std::initializer_list<Str>& options,
				  u32 option = 0,
				  bool sameLine = false, i32 w=70)
	{
		m_params.add(name, NodeParam(NodeParam::Option, 0, 0, option, Vec<Str>(options), w));
		m_params[name].sameLine = sameLine;
	}
};

#endif // TWEN_NODE_H
