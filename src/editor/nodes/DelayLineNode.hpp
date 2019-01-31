#ifndef TWIST_DELAY_LINE_HPP
#define TWIST_DELAY_LINE_HPP

#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"

#include "twen/nodes/DelayLineNode.hpp"

static void DelayLine_gui(Node* node) {
	DelayLineNode *n = dynamic_cast<DelayLineNode*>(node);

	ImGui::PushItemWidth(80);
	ImGui::DragFloat("Feedback", &n->feedBack, 0.1f, 0.0f, 1.0f);
	ImGui::DragFloat("Delay", &n->delay, 0.1f, 0.0f, 10.0f);
	ImGui::PopItemWidth();
}

#endif // TWIST_DELAY_LINE_HPP
