#ifndef T_BUTTON_NODE_H
#define T_BUTTON_NODE_H

#include "TNode.h"
#include "imgui.h"

class TButtonNode : public TNode {
public:
	TButtonNode(bool on) : TNode("Button", 80, 120), enabled(on) {
		addOutput("Out");
	}

	void gui() {
		if (ImGui::Button(enabled ? "OFF" : "ON", ImVec2(32, 32))) {
			enabled = !enabled;
		}
	}

	void solve() {
		setOutput(0, enabled ? 1.0f : 0.0f);
	}

	virtual void save(JSON& json) {
		TNode::save(json);
		json["type"] = type();
		json["on"] = enabled;
	}

	bool enabled;

	static std::string type() { return "Button"; }
};

#endif // T_BUTTON_NODE_H