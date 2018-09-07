#ifndef T_NODE_H
#define T_NODE_H

#include <string>
#include <array>

#define _USE_MATH_DEFINES
#include <cmath>

#include "../json.hpp"
using JSON = nlohmann::json;

#define IMGUI_INCLUDE_IMGUI_USER_H
#include "imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

namespace tmath {
	static float lerp(float a, float b, float t) {
		return (1.0f - t) * a + b * t;
	}

	static float remap(float value, float from1, float to1, float from2, float to2) {
		return (value - from1) / (to1 - from1) * (to2 - from2) + from2;
	}

	static float cyclef(float f) {
		auto m2 = std::fmod(f, 2.0f);
		return m2 < 1.0 ? m2 : 2 - m2;
	}
}

#define FLT_ARR_MAX 8
using FloatArray = std::array<float,8>;
struct TValue {
	FloatArray values;

	std::string label;
	bool connected;

	TValue() {}
	TValue(const std::string& label) : label(label), connected(false) {
		values.fill(0.0f);
	}
};

struct TNodeType {
	static std::string type() { return "NONE"; }
};

class TNodeEditor;
class TNodeGraph;
class TNode : public TNodeType {
	friend class TNodeEditor;
	friend class TNodeGraph;
public:
	TNode() : m_bounds(ImVec4(0, 0, 1, 1)), m_title("TNode") { m_defaultValues.fill(0.0f); }
	TNode(const std::string& title, int width, int height)
		 : m_bounds(ImVec4(0, 0, width, height)), m_title(title)
	{
		m_defaultValues.fill(0.0f);
	}

	virtual ~TNode() {}
	virtual void setup() {}
	virtual void gui() {}
	virtual void solve() {}

	virtual void save(JSON& json);

	TNodeGraph* parent() { return m_parent; }

	const ImVec4& bounds() const { return m_bounds; }
	ImVec4& bounds() { return m_bounds; }
	ImVec2& gridPosition() { return m_gridPosition; }

	std::vector<TValue>& inputs() { return m_inputs; }
	std::vector<TValue>& outputs() { return m_outputs; }

	// Multi-value
	float getMultiOutput(int id, int pos) const { return m_outputs[id].values[pos]; }
	float getMultiInput(int id, int pos) const { return m_inputs[id].values[pos]; }
	float getMultiInputOr(int id, int pos, float def=0.0f) const {
		if (!m_inputs[id].connected)
			return def;
		return m_inputs[id].values[pos];
	}

	FloatArray& getMultiOutputValues(int id, float def=0.0f) {
		if (!m_outputs[id].connected) {
			m_defaultValues.fill(def);
			return m_defaultValues;
		}
		return m_outputs[id].values;
	}
	FloatArray& getMultiInputValues(int id, float def=0.0f) {
		if (!m_inputs[id].connected) {
			m_defaultValues.fill(def);
			return m_defaultValues;
		}
		return m_inputs[id].values;
	}

	float setMultiOutputValues(int id, const FloatArray& v) {
		m_outputs[id].values = v;
	}
	float setMultiInputValues(int id, const FloatArray& v) {
		m_inputs[id].values = v;
	}

	float setMultiOutput(int id, int pos, float value) { m_outputs[id].values[pos] = value; }
	float setMultiInput(int id, int pos, float value) { m_inputs[id].values[pos] = value; }

	float getOutput(int id) const { return getMultiOutput(id, 0); }
	float getInput(int id) const { return getMultiInput(id, 0); }
	float getInputOr(int id, float def=0.0f) const { return getMultiInputOr(id, 0, def); }

	void setOutput(int id, float value, bool fill=false) {
		if (fill) {
			m_outputs[id].values.fill(value);
		} else {
			setMultiOutput(id, 0, value);
		}
	}
	void setInput(int id, float value) { setMultiInput(id, 0, value); }

	ImVec2 inputSlotPos(int s, float x=1, bool snap=false) const {
		ImVec2 p = snap ? m_gridPosition : ImVec2(m_bounds.x, m_bounds.y);
		return ImVec2(p.x*x, p.y*x + m_bounds.w * ((float)s + 1) / ((float)m_inputs.size() + 1));
	}
	ImVec2 outputSlotPos(int s, float x=1, bool snap=false) const {
		ImVec2 p = snap ? m_gridPosition : ImVec2(m_bounds.x, m_bounds.y);
		return ImVec2(p.x*x + m_bounds.z, p.y*x + m_bounds.w * ((float)s + 1) / ((float)m_outputs.size() + 1));
	}

	ImVec2 size() const { return ImVec2(m_bounds.z, m_bounds.w); }
	bool selected() const { return m_selected; }
	int id() const { return m_id; }

	static std::string type() { return "Node"; }

	bool open = true;

protected:
	void addInput(const std::string& label);
	void addOutput(const std::string& label);

	bool removeInput(int id);
	bool removeOutput(int id);

	int m_id;
	std::string m_title, m_type;
	ImVec4 m_bounds;
	ImVec2 m_gridPosition;
	ImRect m_selectionBounds;
	bool m_selected = false, m_solved = false;

	std::vector<TValue> m_inputs;
	std::vector<TValue> m_outputs;

	TNodeGraph* m_parent;

	FloatArray m_defaultValues;
};

struct TLink {
	int inputID, inputSlot;
	int outputID, outputSlot;
};

struct TLinking {
	int inputID, inputSlot;
	bool active;
	TNode* node;
};

#endif // T_NODE_H
