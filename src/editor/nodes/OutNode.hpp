#ifndef TWIST_OUT_HPP
#define TWIST_OUT_HPP

#define IMGUI_INCLUDE_IMGUI_USER_H
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"

#include "twen/nodes/OutNode.hpp"

static void Out_gui(Node* node) {
	OutNode *n = dynamic_cast<OutNode*>(node);
	ImGui::Knob("Gain", &n->gain, 0.0f, 1.0f);
	ImGui::SameLine();
	ImGui::AudioView(
		"##OutNode",
		64,
		n->buffer().data(), n->buffer().size(),
		0, 56
	);
}

#endif // TWIST_OUT_HPP
