#ifndef TWIST_NOTE_HPP
#define TWIST_NOTE_HPP

#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"

#include "twen/nodes/NoteNode.hpp"

static void Note_gui(Node* node) {
	NoteNode*n = dynamic_cast<NoteNode*>(node);
	static const char* NOTES[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

	ImGui::PushItemWidth(60);
	ImGui::Combo("Note", (int*)&n->note, NOTES, 12);
	ImGui::DragInt("Oct.", (int*)&n->oct, 0.1f, 0, 8);
	ImGui::PopItemWidth();
}

static void Hertz_gui(Node* node) {
	// HertzNode*n = dynamic_cast<HertzNode*>(node);
}

#endif // TWIST_NOTE_HPP
