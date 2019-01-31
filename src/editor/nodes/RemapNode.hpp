#ifndef TWIST_REMAP_HPP
#define TWIST_REMAP_HPP

#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"

#include "twen/nodes/RemapNode.hpp"

static void Remap_gui(Node* node) {
	RemapNode *n = dynamic_cast<RemapNode*>(node);

	ImGui::PushItemWidth(90);
	float from[2] = { n->fromMin, n->fromMax }, to[2] = { n->toMin, n->toMax };
	if (ImGui::InputFloat2("From", from)) {
		n->fromMin = from[0];
		n->fromMax = from[1];
	}
	if (ImGui::InputFloat2("To", to)) {
		n->toMin = to[0];
		n->toMax = to[1];
	}
	ImGui::PopItemWidth();
}

#endif // TWIST_REMAP_HPP
