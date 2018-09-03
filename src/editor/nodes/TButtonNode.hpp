#ifndef T_BUTTON_NODE_H
#define T_BUTTON_NODE_H

#include "TNode.h"
#include "imgui.h"

class TButtonNode : public TNode {
public:
	TButtonNode() : TNode("Button", 80, 120) {
		addOutput("Out");
	}

	void gui() {
		enabled = ImGui::RubberButton("##button");
	}

	void solve() {
		setOutput(0, 0.0f, true);
		if (enabled)
			setOutput(0, 1.0f, true);
	}

	void save(JSON& json) {
		TNode::save(json);
		json["type"] = type();
	}

	bool enabled;
	static std::string type() { return "Button"; }
};

#endif // T_BUTTON_NODE_H
