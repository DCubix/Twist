#ifndef T_OUT_NODE_H
#define T_OUT_NODE_H

#include "TNode.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#define WAVEFORM_LENGTH 160

class TOutNode : public TNode {
public:
	TOutNode() : TNode("Output", 140, 140), volume(1.0f) {
		addInput("In");
		std::memset(waveForm, 0, WAVEFORM_LENGTH * sizeof(float));
	}

	void gui() {
		ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(0, 0, 0, 255));
		if (ImGui::BeginChild("##wf", ImVec2(120, 80))) {
			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			float fac = ImGui::GetWindowWidth() / WAVEFORM_LENGTH;
			float hh = ImGui::GetWindowHeight() * 0.5f;
			ImVec2 prev = ImVec2(0, hh);
			for (int i = 0; i < WAVEFORM_LENGTH; i++) {
				ImVec2 pos = ImVec2(i * fac, waveForm[i] * hh + hh) + ImGui::GetWindowPos();
				draw_list->AddLine(prev, pos, IM_COL32(100, 250, 100, 200), 1.0f);
				prev = pos;
			}
		}
		ImGui::EndChild();
		ImGui::PopStyleColor();
		ImGui::SliderFloat("", &volume, 0.0f, 1.0f);
	}

	void solve() {
	}

	float volume;
	float waveForm[WAVEFORM_LENGTH];

	static std::string type() { return "Out"; }
};

#endif // T_OUT_NODE_H