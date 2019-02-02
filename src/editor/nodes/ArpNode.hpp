#ifndef TWIST_ARP_HPP
#define TWIST_ARP_HPP

#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"

#include "twen/nodes/ArpNode.hpp"

static void Arp_gui(Node* node) {
	ArpNode *n = dynamic_cast<ArpNode*>(node);

	static const char* NOTES[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
	static const char* CHORDS[] = { "maj", "min", "sus2", "sus4", "maj7", "min7", "9th", "oct", "#5", "dim", "6th" };
	static const char* DIRS[] = { "Up", "Down", "Up+Down", "Random" };

	ImGui::PushItemWidth(60);
	ImGui::Combo("Note", (int*)&n->note, NOTES, 12); ImGui::SameLine();
	ImGui::DragInt("Oct.", (int*)&n->oct, 0.1f, 0, 8);
	ImGui::Combo("Chord", (int*)&n->chord, CHORDS, 11); ImGui::SameLine();
	ImGui::Combo("Dir.", (int*)&n->direction, DIRS, 4);
	ImGui::PopItemWidth();
}

#endif // TWIST_ARP_HPP
