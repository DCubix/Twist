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
		enabled = false;
		if (ImGui::Button("PRESS", ImVec2(32, 32))) {
			enabled = true;
		}
	}

	void solve() {
		setOutput(0, enabled ? 1.0f : 0.0f);
	}

	void save(JSON& json) {
		TNode::save(json);
		json["type"] = type();
		json["on"] = enabled;
	}

	bool enabled;
	static std::string type() { return "Button"; }
};

#endif // T_BUTTON_NODE_H
