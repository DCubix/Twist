#ifndef T_VALUE_NODE_H
#define T_VALUE_NODE_H

#include "TNode.h"
#include "imgui.h"

class TValueNode : public TNode {
public:
	TValueNode(float v) : TNode("Value", 140, 90), value(v) {
		addOutput("Out");
	}

	void gui() {
		ImGui::DragFloat("Value", &value, 0.01f, 0.0f, 1.0f);
	}

	void solve() {
		setOutput(0, value);
	}

	void save(JSON& json) {
		TNode::save(json);
		json["type"] = type();
		json["value"] = value;
	}

	float value;

	static std::string type() { return "Value"; }
};

#endif // T_VALUE_NODE_H
