#ifndef TWIST_MATH_HPP
#define TWIST_MATH_HPP

#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"

#include "twen/nodes/MathNode.hpp"

static void Math_gui(Node* node) {
	MathNode *n = dynamic_cast<MathNode*>(node);

	static const char* OPS[] = { "Add", "Subtract", "Multiply", "Negate", "Average" };

	ImGui::PushItemWidth(80);
	ImGui::Combo("Op", (int*)&n->op, OPS, 5);
	if (n->connected(0)) {
		ImGui::Text("A: %.2f", n->in(0).value());
	} else {
		ImGui::InputFloat("A", &n->a, 0.01f, 0.1f);
	}

	if (n->connected(1)) {
		ImGui::Text("B: %.2f", n->in(1).value());
	} else {
		ImGui::InputFloat("B", &n->b, 0.01f, 0.1f);
	}
	ImGui::PopItemWidth();
}

#endif // TWIST_MATH_HPP
