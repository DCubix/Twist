#ifndef TWIST_SEQUENCER_HPP
#define TWIST_SEQUENCER_HPP

#define IMGUI_INCLUDE_IMGUI_USER_H
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"

#include "twen/NodeGraph.h"

#define TWIST_SEQUENCER_SIZE 16

struct SNote {
	Note note{ Note::C };
	u32 octave{ 3 };
	float vel{ 1.0f };
	bool active{ false };
};

class SequencerNode : public Node {
	TWEN_NODE(SequencerNode, "Sequencer")
public:
	inline SequencerNode()
		: Node()
	{
		addInput("Base");
	}

	inline SequencerNode(JSON param)
		: SequencerNode()
	{
		load(param);
	}

	inline Value sample(NodeGraph *graph) override {
		int baseNote = connected(0) ? int(in(0).value()) : 0;
		bool baseGate = connected(0) ? in(0).gate() : true;
		float baseVel = connected(0) ? in(0).velocity() : 1.0f;

		idx = graph->index() % TWIST_SEQUENCER_SIZE;

		u32 outNote = 0;
		float value = 0, vel = 0;

		SNote curr = notes[idx];
		if (curr.active) {
			outNote = u32(curr.note) + (12 * curr.octave) + baseNote;
			value = outNote;
			vel = curr.vel * baseVel;
			// m_gate = baseGate;
			m_gate = true;
		} else {
			m_gate = false;
		}

		if (graph->time() >= 0.99f) {
			m_gate = false;
		}

		return Value(value, vel, m_gate);
	}

	inline void save(JSON& json) override {
		Node::save(json);
		JSON notes = JSON::array();
		for (u32 i = 0; i < TWIST_SEQUENCER_SIZE; i++) {
			JSON note;
			SNote n = this->notes[i];
			note["note"] = n.note;
			note["oct"] = n.octave;
			note["vel"] = n.vel;
			note["active"] = n.active;
			notes[i] = note;
		}

		json["notes"] = notes;
	}

	inline void load(JSON json) override {
		Node::load(json);
		if (!json["notes"].is_array()) return;

		for (u32 i = 0; i < json["notes"].size(); i++) {
			JSON note = json["notes"][i];
			SNote n;
			n.vel = note["vel"];
			n.octave = note["oct"];
			n.note = note["note"];
			n.active = note["active"];
			notes[i] = n;
		}
	}

	Arr<SNote, TWIST_SEQUENCER_SIZE> notes;
	u32 idx;

	i32 sel = -1;
private:
	bool m_gate = false;

};

static i32 getClickedIndex(float w, float h, ImVec2 wp) {
	for (u32 i = 0; i < TWIST_SEQUENCER_SIZE; i++) {
		ImRect srect;
		srect.Min = ImVec2(i * w, 0) + wp;
		srect.Max = ImVec2(i * w + w, h) + wp;
		if (srect.Contains(ImGui::GetMousePos())) {
			return i;
		}
	}
	return -1;
}

static void Sequencer_gui(Node* node) {
	SequencerNode *n = dynamic_cast<SequencerNode*>(node);

	i32 &sel = n->sel;

	const float width = 210.0f;

	static const char* NOTES[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

	const u32 col = IM_COL32(0, 200, 100, 255);
	const u32 colT = IM_COL32(0, 200, 100, 80);
	const u32 colSel = IM_COL32(250, 20, 20, 180);
	const u32 border = IM_COL32(0, 0, 0, 255);
	const u32 borderLight = IM_COL32(100, 100, 100, 255);
	const u32 borderLightAlpha = IM_COL32(100, 100, 100, 128);
	const u32 cursor = IM_COL32(200, 100, 0, 128);

	const float height = 16.0f;
	const float slotWidth = width / TWIST_SEQUENCER_SIZE;

	ImGuiIO& io = ImGui::GetIO();

	const ImVec2 wp = ImGui::GetCursorScreenPos();
	ImGui::InvisibleButton("##_sequencer_", ImVec2(width, height));

	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	const ImVec2 rect_max = ImVec2(width, height) + wp;

	bool is_active = ImGui::IsItemActive();
	bool is_rightclick = ImGui::IsItemClicked(1);

	if (is_active) {
		bool selected = false;

		u32 idx = 0;
		for (u32 i = 0; i < TWIST_SEQUENCER_SIZE; i++) {
			ImRect nr;
			nr.Min = ImVec2(i * slotWidth, 0) + wp;
			nr.Max = ImVec2(i * slotWidth + slotWidth, height) + wp;
			if (nr.Contains(ImGui::GetMousePos())) {
				idx = i;
				selected = true;
				break;
			}
		}

		if (selected) {
			sel = i32(idx);
			n->notes[idx].active = true;
		}
	} else if (is_rightclick) {
		bool selected = false;

		u32 idx = 0;
		for (u32 i = 0; i < TWIST_SEQUENCER_SIZE; i++) {
			ImRect nr;
			nr.Min = ImVec2(i * slotWidth, 0) + wp;
			nr.Max = ImVec2(i * slotWidth + slotWidth, height) + wp;
			if (nr.Contains(ImGui::GetMousePos())) {
				idx = i;
				selected = true;
				break;
			}
		}

		if (selected) {
			sel = -1;
			n->notes[idx].active = false;
		}
	}

	draw_list->AddRectFilled(
		wp,
		rect_max,
		IM_COL32(0, 0, 0, 255)
	);

	draw_list->PushClipRect(wp, rect_max, true);

	for (u32 i = 0; i < TWIST_SEQUENCER_SIZE; i+=2) {
		ImRect nr;
		nr.Min = ImVec2(i * slotWidth, 0) + wp;
		nr.Max = ImVec2(i * slotWidth + slotWidth, height) + wp;
		draw_list->AddRectFilled(
			nr.Min, nr.Max,
			borderLightAlpha
		);
	}

	for (u32 i = 0; i < TWIST_SEQUENCER_SIZE; i++) {
		if (!n->notes[i].active) continue;

		ImRect nrf;
		nrf.Min = ImVec2(i * slotWidth, 0) + wp;
		nrf.Max = ImVec2(i * slotWidth + slotWidth, height) + wp;
		draw_list->AddRectFilled(
			nrf.Min, nrf.Max,
			colT
		);

		// float h = (height * (1.0f - n->notes[i].vel));

		ImRect nr;
		nr.Min = ImVec2(i * slotWidth, 0.0f) + wp;
		nr.Max = ImVec2(i * slotWidth + slotWidth, height) + wp;
		draw_list->AddRectFilled(
			nr.Min, nr.Max,
			col
		);
		draw_list->AddRect(
			nr.Min, nr.Max,
			border
		);
	}

	if (sel >= 0) {
		ImRect nr;
		nr.Min = ImVec2(sel * slotWidth + 2, 2) + wp;
		nr.Max = ImVec2(sel * slotWidth + slotWidth - 2, height - 2) + wp;
		draw_list->AddRect(
			nr.Min, nr.Max,
			colSel,
			0.0f, 0, 2.0f
		);
	}

	ImRect nr;
	nr.Min = ImVec2(n->idx * slotWidth, 0) + wp;
	nr.Max = ImVec2(n->idx * slotWidth + slotWidth, height) + wp;
	draw_list->AddRectFilled(
		nr.Min, nr.Max,
		cursor
	);

	draw_list->PopClipRect();

	draw_list->AddRect(
		wp,
		rect_max,
		borderLight
	);

	if (sel != -1) {
		SNote &sn = n->notes[sel];
		ImGui::PushItemWidth(width / 4);
		ImGui::Combo("Note", (int*)&sn.note, NOTES, 12); ImGui::SameLine();
		ImGui::DragInt("Oct.", (int*)&sn.octave, 0.1f, 0, 8);
		ImGui::PopItemWidth();

		ImGui::PushItemWidth(width);
		ImGui::SliderFloat("Vel.", &sn.vel, 0.0f, 1.0f);
		ImGui::PopItemWidth();
	}
}

#endif // TWIST_SEQUENCER_HPP
