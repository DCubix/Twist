#ifndef TWIST_READER_HPP
#define TWIST_READER_HPP

#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"

#include "twen/nodes/StorageNodes.hpp"

static void Reader_gui(Node* node) {
	ReaderNode *n = dynamic_cast<ReaderNode*>(node);

	ImGui::PushItemWidth(70);
	ImGui::DragInt("Slot", (int*)&n->slot, 0.1f, 0, TWEN_GLOBAL_STORAGE_SIZE);
	ImGui::PopItemWidth();
}

#endif // TWIST_READER_HPP
