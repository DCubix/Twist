#ifndef T_NODE_H
#define T_NODE_H

#include <string>

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
}

struct TValue {
	float value;
	std::string label;
	bool connected;

	TValue() {}
	TValue(const std::string& label) : label(label), value(0), connected(false) {}
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
	TNode() : m_bounds(ImVec4(0, 0, 1, 1)), m_title("TNode") {}
	TNode(const std::string& title, int width, int height)
		 : m_bounds(ImVec4(0, 0, width, height)), m_title(title)
	{}

	virtual ~TNode() {}
	virtual void setup() {}
	virtual void gui() {}
	virtual void solve() {}

	virtual void save(JSON& json);

	TNodeGraph* parent() { return m_parent; }

	const ImVec4& bounds() const { return m_bounds; }
	ImVec4& bounds() { return m_bounds; }

	std::vector<TValue>& inputs() { return m_inputs; }
	std::vector<TValue>& outputs() { return m_outputs; }

	void setOutput(int id, float value) { m_outputs[id].value = value; }
	void setInput(int id, float value) { m_inputs[id].value = value; }

	float getOutput(int id) { return m_outputs[id].value; }
	float getInput(int id) { return m_inputs[id].value; }

	float getInputOr(int id, float def=0.0f) {
		if (!m_inputs[id].connected)
			return def;
		return m_inputs[id].value;
	}

	ImVec2 inputSlotPos(int s, float x=1) const {
		return ImVec2(m_bounds.x*x, m_bounds.y*x + m_bounds.w * ((float)s + 1) / ((float)m_inputs.size() + 1));
	}
	ImVec2 outputSlotPos(int s, float x=1) const {
		return ImVec2(m_bounds.x*x + m_bounds.z, m_bounds.y*x + m_bounds.w * ((float)s + 1) / ((float)m_outputs.size() + 1));
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
	ImRect m_selectionBounds;
	bool m_selected = false;

	std::vector<TValue> m_inputs;
	std::vector<TValue> m_outputs;

	TNodeGraph* m_parent;
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
