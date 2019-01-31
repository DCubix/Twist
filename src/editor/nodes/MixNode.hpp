#ifndef TWIST_MIX_HPP
#define TWIST_MIX_HPP

#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"

#include "twen/nodes/MixNode.hpp"

static void Mix_gui(Node* node) {
	MixNode*n = dynamic_cast<MixNode*>(node);

	ImGui::PushItemWidth(80);
	ImGui::Spacing();
	if (n->connected(2)) {
		ImGui::Text("Fac.: %.2f", n->get(2));
	} else {
		ImGui::SliderFloat("Fac.", &n->factor, 0, 1);
	}
	ImGui::PopItemWidth();
}

#endif // TWIST_MIX_HPP
