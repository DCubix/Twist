#ifndef TWIST_BUTTON_H
#define TWIST_BUTTON_H

#define IMGUI_INCLUDE_IMGUI_USER_H
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"

#include "twen/NodeGraph.h"

class ButtonNode : public Node {
	TWEN_NODE(ButtonNode, "Button")
public:
	ButtonNode() : Node(), active(false) {	}

	float sample(NodeGraph *graph) override {
		return active ? 1.0f : 0.0f;
	}

	bool active;
};

static void Button_gui(Node* node) {
	ButtonNode *n = dynamic_cast<ButtonNode*>(node);

	ImGui::PushItemWidth(50);
	if (ImGui::RubberButton("##_button")) {
		n->active = true;
	} else {
		n->active = false;
	}
	ImGui::PopItemWidth();
}

#endif // TWIST_BUTTON_H
