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
	u32 pos{ 0 }, length{ 0 };
};

class SequencerNode : public Node {
	TWEN_NODE(SequencerNode, "Sequencer")
public:
	SequencerNode()
		: Node(), m_prevV(0), sel(-1)
	{}

	SequencerNode(JSON param)
		: Node(), m_prevV(0), sel(-1)
	{
		load(param);
	}

	Value sample(NodeGraph *graph) override {
		idx = graph->index();
		//u32(Utils::lerp(0, TWEN_NODE_BUFFER_SIZE, graph->time())) % TWEN_NODE_BUFFER_SIZE;
		u32 outNote = 0;
		float value = 0, vel = 0;

		for (auto&& n : notes) {
			if (idx >= n.pos && idx < n.pos + n.length) {
				outNote = u32(n.note) + (12 * n.octave);
				value = Utils::noteFrequency(outNote);
				vel = n.vel;
				m_gate = true;
				break;
			} else { m_gate = false; }
		}
		if (graph->time() >= 0.99f) {
			m_gate = false;
		}

		return Value(value, vel, m_gate);
	}

	void save(JSON& json) override {
		Node::save(json);
		JSON notes = JSON::array();
		int i = 0;
		for (auto&& n : this->notes) {
			JSON note;
			note["note"] = n.note;
			note["oct"] = n.octave;
			note["vel"] = n.vel;
			note["pos"] = n.pos;
			note["length"] = n.length;
			notes[i++] = note;
		}

		json["notes"] = notes;
	}

	void load(JSON json) override {
		Node::load(json);
		if (!json["notes"].is_array()) return;

		notes.clear();
		for (u32 i = 0; i < json["notes"].size(); i++) {
			JSON note = json["notes"][i];
			SNote n;
			n.pos = note["pos"];
			n.length = note["length"];
			n.vel = note["vel"];
			n.octave = note["oct"];
			n.note = note["note"];
			notes.push_back(n);
		}
	}

	Vec<SNote> notes;
	u32 idx;

	SNote note;
	i32 sel;
private:
	u32 m_prevV;
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

static bool outOfBounds(SNote n, const Vec<SNote>& notes) {
	bool oob = n.pos + n.length > TWIST_SEQUENCER_SIZE;
//	for (auto&& sn : notes) {
//		if (sn.pos == n.pos && sn.length == n.length && sn.note == n.note && sn.octave == n.octave)
//			continue;
//		oob = oob ||
//				(n.pos + n.length >= sn.pos || n.pos <= sn.pos + sn.length);
//	}
	return oob;
}

static void Sequencer_gui(Node* node) {
	SequencerNode *n = dynamic_cast<SequencerNode*>(node);

	SNote &note = n->note;
	i32 &sel = n->sel;

	const float width = 210.0f;

	static const char* NOTES[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

	const u32 col = IM_COL32(0, 200, 100, 255);
	const u32 colSel = IM_COL32(0, 250, 170, 255);
	const u32 border = IM_COL32(0, 0, 0, 255);
	const u32 borderLight = IM_COL32(100, 100, 100, 255);
	const u32 cursor = IM_COL32(200, 100, 0, 128);

	const float height = 16.0f;
	const float slotWidth = width / TWIST_SEQUENCER_SIZE;

	ImGuiIO& io = ImGui::GetIO();

	const ImVec2 wp = ImGui::GetCursorScreenPos();
	ImGui::InvisibleButton("##_sequencer_", ImVec2(width, height));

	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	const ImVec2 rect_max = ImVec2(width, height) + wp;

	bool is_active = ImGui::IsItemActive();
	bool is_released = !is_active && ImGui::IsItemActiveLastFrame();
	bool is_hovered = ImGui::IsItemHovered();
	bool is_rightclicked = ImGui::IsItemClicked(1);

	if (is_rightclicked && is_hovered) {
		bool selected = false;

		i32 i = 0;
		for (auto&& note : n->notes) {
			ImRect nr;
			nr.Min = ImVec2(note.pos * slotWidth, 0) + wp;
			nr.Max = ImVec2((note.pos * slotWidth + note.length * slotWidth) - 1, height) + wp;
			if (nr.Contains(ImGui::GetMousePos())) {
				sel = i;
				selected = true;
				break;
			}
			i++;
		}

		if (selected) {
			n->notes.erase(n->notes.begin() + i);
			sel = -1;
		}
	} else if (is_active && !ImGui::IsItemActiveLastFrame()) {
		bool selected = false;

		i32 i = 0;
		for (auto&& note : n->notes) {
			ImRect nr;
			nr.Min = ImVec2(note.pos * slotWidth, 0) + wp;
			nr.Max = ImVec2((note.pos * slotWidth + note.length * slotWidth) - 1, height) + wp;
			if (nr.Contains(ImGui::GetMousePos())) {
				sel = i;
				selected = true;
				break;
			}
			i++;
		}

		if (!selected) {
			if (sel != -1) {
				sel = -1;
			} else {
				i32 i = getClickedIndex(slotWidth, height, wp);
				if (i != -1) {
					note.length = 1;

					u32 prev = note.pos;
					note.pos = u32(i);
					if (outOfBounds(note, n->notes)) {
						note.pos = prev;
					}
				}
			}
		}
	} else if (is_active && std::abs(io.MouseDelta.x) > 0.0f) {
		i32 i = getClickedIndex(slotWidth, height, wp);
		if (sel >= 0) {
			SNote &sn = n->notes[sel];
			u32 prev = sn.pos;
			sn.pos = u32(i);
			if (outOfBounds(sn, n->notes)) {
				sn.pos = prev;
			}
		} else {
			if (i != -1) {
				if (u32(i) < note.pos) {
					u32 prev = note.pos;
					note.pos = u32(i);
					if (outOfBounds(note, n->notes)) {
						note.pos = prev;
					}
					note.length = (u32(i) - note.pos) + 1;
				} else {
					note.length = (u32(i) - note.pos) + 1;
				}
			}
		}
	} else if (is_released && is_hovered) {
		if (note.length > 0) {
			n->notes.push_back(note);
			sel = n->notes.size()-1;
			note.length = 0;
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
			borderLight
		);
	}

	for (auto&& note : n->notes) {
		ImRect nr;
		nr.Min = ImVec2(note.pos * slotWidth, 0) + wp;
		nr.Max = ImVec2((note.pos * slotWidth + note.length * slotWidth), height) + wp;
		draw_list->AddRectFilled(
			nr.Min, nr.Max,
			col
		);
		draw_list->AddRect(
			nr.Min, nr.Max,
			border
		);
	}

	if (note.length > 0) {
		ImRect nr;
		nr.Min = ImVec2(note.pos * slotWidth, 0) + wp;
		nr.Max = ImVec2(note.pos * slotWidth + note.length * slotWidth, height) + wp;
		draw_list->AddRectFilled(
			nr.Min, nr.Max,
			colSel
		);
	}

	if (sel >= 0) {
		SNote sn = n->notes[sel];
		ImRect nr;
		nr.Min = ImVec2(sn.pos * slotWidth, 0) + wp;
		nr.Max = ImVec2(sn.pos * slotWidth + sn.length * slotWidth, height) + wp;
		draw_list->AddRectFilled(
			nr.Min, nr.Max,
			colSel
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
