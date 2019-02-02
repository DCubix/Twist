#ifndef TWIST_OSCILLATOR_HPP
#define TWIST_OSCILLATOR_HPP

#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"

#include "twen/nodes/OscillatorNode.hpp"

static void Oscillator_gui(Node* node) {
	OscillatorNode *n = dynamic_cast<OscillatorNode*>(node);

	static const char* WF[] = { "Sine", "Square", "Saw", "Triangle", "Noise" };

	ImGui::PushItemWidth(80);
	ImGui::Combo("WaveForm", (int*)&n->waveForm, WF, 5);
	if (n->connected(1)) {
		ImGui::Text("Freq.: %.2f", n->in(1).value());
	} else {
		ImGui::DragFloat("Freq.", &n->frequency, 0.1f, 30.0f, 15000.0f);
	}
	ImGui::PopItemWidth();
}

#endif // TWIST_OSCILLATOR_HPP
