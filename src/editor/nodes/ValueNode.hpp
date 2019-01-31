#ifndef TWIST_VALUE_HPP
#define TWIST_VALUE_HPP

#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"

#include "twen/nodes/ValueNode.hpp"

static void Value_gui(Node* node) {
	ValueNode *n = dynamic_cast<ValueNode*>(node);

	ImGui::PushItemWidth(90);
	ImGui::InputFloat("Value", &n->value, 0.01f, 0.1f);
	ImGui::PopItemWidth();
}

#endif // TWIST_VALUE_HPP
