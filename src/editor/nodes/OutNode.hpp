#ifndef TWIST_OUT_HPP
#define TWIST_OUT_HPP

#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"

#include "twen/nodes/OutNode.hpp"

static void Out_gui(Node* node) {
	OutNode *n = dynamic_cast<OutNode*>(node);
	ImGui::SliderFloat("Gain", &n->gain, 0, 1);
}

#endif // TWIST_OUT_HPP
