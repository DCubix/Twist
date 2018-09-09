#include "Node.h"

Node::Node()
 :	m_id(0),
	m_solved(false),
	m_default(0.0f)
{
	m_defaultArr.set(0.0f);
}

void Node::save(JSON& json) {
//	json["id"] = m_id;
	json["type"] = m_name;
	json["enabled"] = m_enabled;
	for (u32 i = 0; i < m_params.size(); i++) {
		NodeParam param = m_params[i];
		json["params"][i]["value"] = param.value;
		json["params"][i]["option"] = param.option;
	}
}

void Node::load(JSON json) {
//	m_id = json["id"].is_number_unsigned() ? json["id"].get<u64>() : m_id;
//	m_name = json["type"].is_string() ? json["type"].get<Str>() : m_name;
	m_enabled = json["enabled"].is_boolean() ? json["enabled"].get<bool>() : m_enabled;
	if (json["params"].is_array()) {
		for (i32 i = 0; i < json["params"].size(); i++) {
			JSON param = json["params"][i];
			m_params[i].value = !param["value"].is_number_float() ? 0.0f : param["value"].get<float>();
			m_params[i].option = !param["option"].is_number_unsigned() ? 0 : param["option"].get<u32>();
		}
	}
}

float& Node::in(u32 input, u32 param, u32 slot) {
	LogAssert(isInputValid(input), "Invalid input index.");
	if (!m_inputs[input].connected) {
		if (isParamValid(param))
			return m_params[param].value;
		else
			return m_default;
	}
	return m_inputs[input].values[slot];
}

Str Node::inName(u32 input) {
	LogAssert(isInputValid(input), "Invalid input index.");
	return m_inputNames[input];
}

float& Node::out(u32 output, u32 slot) {
	LogAssert(isOutputValid(output), "Invalid output index.");
	if (!m_outputs[output].connected)
		return m_default;
	return m_outputs[output].values[slot];
}

Str Node::outName(u32 output) {
	LogAssert(isOutputValid(output), "Invalid output index.");
	return m_outputNames[output];
}

FloatArray& Node::ins(u32 input, u32 param, bool fill) {
	LogAssert(isInputValid(input), "Invalid input index.");
	if (!m_inputs[input].connected) {
		m_defaultArr.set(0.0f);
		if (isParamValid(param)) {
			if (!fill)
				m_defaultArr[0] = m_params[param].value;
			else
				m_defaultArr.set(m_params[param].value);
		}
		return m_defaultArr;
	}
	return m_inputs[input].values;
}

FloatArray& Node::outs(u32 output) {
	LogAssert(isOutputValid(output), "Invalid output index.");
	if (!m_outputs[output].connected) {
		m_defaultArr.set(0.0f);
		return m_defaultArr;
	}
	return m_outputs[output].values;
}

float& Node::param(u32 param) {
	LogAssert(isParamValid(param), "Invalid parameter.");
	return m_params[param].value;
}

u32& Node::paramOption(u32 param) {
	LogAssert(isParamValid(param), "Invalid parameter.");
	return m_params[param].option;
}

Vec<const char*> Node::paramOptions(u32 param) {
	LogAssert(isParamValid(param), "Invalid parameter.");
	Vec<const char*> ops;
	ops.resize(m_params[param].options.size());
	for (i32 i = 0; i < ops.size(); i++) {
		char* title = new char[m_params[param].options[i].size()];
		strcpy(title, m_params[param].options[i].c_str());
		ops[i] = title;
	}
	return ops;
}

Str Node::paramName(u32 param) {
	LogAssert(isParamValid(param), "Invalid parameter.");
	return m_paramNames[param];
}

void Node::addInput(const Str& name) {
	m_inputNames[m_inputs.size()] = name;
	m_inputs.push_back(NodeSlot());
	m_inputs.back().values.set(0.0f);
	m_inputs.back().id = m_inputs.size()-1;
}

void Node::addOutput(const Str& name) {
	m_outputNames[m_outputs.size()] = name;
	m_outputs.push_back(NodeSlot());
	m_outputs.back().values.set(0.0f);
	m_outputs.back().id = m_outputs.size()-1;
}

//void Node::removeInput(const Str& name) {
//	if (m_inputs.find(name) == m_inputs.end())
//		return;
//	m_inputs.erase(name);
//}

//void Node::removeOutput(const Str& name) {
//	if (m_outputs.find(name) == m_outputs.end())
//		return;
//	m_outputs.erase(name);
//}

void Node::addParam(const Str& name, float value, float step, bool sameLine, i32 w) {
	m_paramNames[m_params.size()] = name;
	m_params.push_back(NodeParam(NodeParam::None, 0.0f, 0.0f, value, step, Vec<Str>(), w));
	m_params.back().sameLine = sameLine;
}

void Node::addParam(const Str& name, float min, float max, float value, float step, NodeParam::ParamType type, bool sameLine, i32 w) {
	m_paramNames[m_params.size()] = name;
	m_params.push_back(NodeParam(type, min, max, value, step, Vec<Str>(), w));
	m_params.back().sameLine = sameLine;
}

void Node::addParam(const Str& name, const std::initializer_list<Str>& options, u32 option, bool sameLine, i32 w) {
	m_paramNames[m_params.size()] = name;
	m_params.push_back(NodeParam(NodeParam::Option, 0, 0, option, Vec<Str>(options), w));
	m_params.back().sameLine = sameLine;
}
