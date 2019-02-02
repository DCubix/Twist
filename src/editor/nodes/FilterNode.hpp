#ifndef TWIST_FILTER_HPP
#define TWIST_FILTER_HPP

#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"

#include "twen/nodes/FilterNode.hpp"

static void Filter_gui(Node* node) {
	FilterNode *n = dynamic_cast<FilterNode*>(node);

	static const char* FILTERS[] = { "Low-Pass", "High-Pass" };

	ImGui::PushItemWidth(80);
	ImGui::Combo("Filter", (int*)&n->filter, FILTERS, 2);
	if (n->connected(1)) {
		ImGui::Text("CutOff: %.2f", n->in(1).value());
	} else {
		ImGui::DragFloat("CutOff", &n->cutOff, 1.0f, 20.0f, 20000.0f);
	}
	ImGui::PopItemWidth();
}

#endif // TWIST_FILTER_HPP
