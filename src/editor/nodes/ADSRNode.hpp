#ifndef TWIST_ADSR_HPP
#define TWIST_ADSR_HPP

#define IMGUI_INCLUDE_IMGUI_USER_H
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"

#include "twen/nodes/ADSRNode.hpp"

static void ADSR_gui(Node* node) {
	ADSRNode *n = dynamic_cast<ADSRNode*>(node);

	ImGui::Knob("A", &n->a, 0.0f, 20.0f);
	ImGui::SameLine();
	ImGui::Knob("D", &n->d, 0.0f, 20.0f);
	ImGui::SameLine();
	ImGui::Knob("S", &n->s, 0.0f, 1.0f);
	ImGui::SameLine();
	ImGui::Knob("R", &n->r, 0.0f, 20.0f);
}

#endif // TWIST_ADSR_HPP
