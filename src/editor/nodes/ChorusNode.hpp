#ifndef TWIST_CHORUS_HPP
#define TWIST_CHORUS_HPP

#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"

#include "twen/nodes/ChorusNode.hpp"

static void Chorus_gui(Node* node) {
	ChorusNode *n = dynamic_cast<ChorusNode*>(node);

	ImGui::PushItemWidth(90);
	ImGui::DragFloat("Rate", &n->rate, 0.1f, 0.0f, 6.0f);
	ImGui::DragFloat("Depth", &n->depth, 0.1f, 0.0f, 1.0f);
	ImGui::DragFloat("Delay", &n->delay, 0.1f, 0.0f, 1.0f);
	ImGui::PopItemWidth();
}

#endif // TWIST_CHORUS_HPP
