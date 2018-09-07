#ifndef T_PIANO_ROLL_NODE_H
#define T_PIANO_ROLL_NODE_H

#include <cstdint>
#include <iostream>
#include "TNode.h"
#include "../../TGen.h"

#define SNOTES 4
#define MAX_PIANO_KEYS 84
class TPianoRollNode : public TNode {
public:
	struct TNote {
		int position, length, note;
		ImRect rect;
	};

	enum TTimeSignature {
		ThreeFour = 0,
		FourFour,
		FiveFour
	};

	TPianoRollNode() : TNode("Piano Roll", 0, 0) {
		addInput("Mod");
		addInput("Index");
		addInput("Gate");
		addOutput("Freq");
		addOutput("Gate");
		keys.fill(false);
		prevKeys.fill(false);
	}

	void gui() {
		ImGuiStyle& style = ImGui::GetStyle();
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImDrawList* dl = ImGui::GetWindowDrawList();

		const float lineHeight = ImGui::GetItemsLineHeightWithSpacing() + style.ItemInnerSpacing.y;
		const float height = wheight == -1 ? lineHeight * 6 : wheight;
		const float heightMax = (MAX_PIANO_KEYS+14) * 6;

		const ImGuiID id0 = window->GetID("pr0");
		const ImGuiID id1 = window->GetID("pr1");

		const ImVec2 wp = ImGui::GetCursorScreenPos();

		ImGui::BeginHorizontal("##opts");

		static const char* TIME_SIGS[] = {
			"3/4\0",
			"4/4\0",
			"5/4\0",
			0
		};

		ImGui::Combo("Time Sig.##timeSig", (int*)&timeSignature, TIME_SIGS, 3);
		ImGui::DragInt("Meas.##measures", &measures, 0.05f, 0, 8);
		ImGui::PushItemWidth(32);
		if (ImGui::Button(wheight == -1 ? "+" : "-", ImVec2(32, 0))) {
			if (wheight == -1)
				wheight = lineHeight * 16;
			else
				wheight = -1;
		}
		ImGui::PopItemWidth();
		ImGui::EndHorizontal();

		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0, 0, 0, 0));
		ImGui::BeginChild(id0, ImVec2(350, height), true, ImGuiWindowFlags_HorizontalScrollbar);
			ImGui::KeyBed("##PRKeybed", keys.data(), MAX_PIANO_KEYS);
			ImGui::SameLine(0, 0);
			Grid(heightMax, wp);
		ImGui::EndChild();
		ImGui::PopStyleColor(3);
	}

	void solve() {
		int barsPerMeasure = 0;
		switch (timeSignature) {
			case ThreeFour: barsPerMeasure = 3; break;
			case FourFour: barsPerMeasure = 4; break;
			default: barsPerMeasure = 5; break;
		}

		int noteCount = measures * barsPerMeasure * SNOTES;

		noteIndex = int(getInputOr(1, 0));
		noteIndex = noteIndex % noteCount;

		for (int i = 0; i < FLT_ARR_MAX; i++) {
			setMultiOutput(1, i, 0.0f);
		}

		prevKeys.swap(keys);

		keys.fill(false);
		for (TNote note : notes) {
			if (noteIndex >= note.position && noteIndex < note.position + note.length) {
				keys[note.note] = true;
			}
		}

		for (int i = 0; i < FLT_ARR_MAX; i++)
			setMultiOutput(1, i, 0);

		int s = 0;
		for (int i = 0; i < MAX_PIANO_KEYS; i++) {
			if (keys[i]) {
				setMultiOutput(0, s % FLT_ARR_MAX, tgen::note(i));
				setMultiOutput(1, s % FLT_ARR_MAX, (1 + getInputOr(2, 1)) * 0.5f);
				s++;
			}
		}
	}

	void save(JSON& json) {
		TNode::save(json);
		json["type"] = type();
		json["measures"] = measures;
		json["timeSignature"] = (int)timeSignature;
		for (int i = 0; i < notes.size(); i++) {
			JSON& entry = json["notes"][i];
			entry["position"] = notes[i].position;
			entry["length"] = notes[i].length;
			entry["note"] = notes[i].note;
		}
	}

	std::array<bool, MAX_PIANO_KEYS> keys, prevKeys;

	int measures = 4, noteIndex = 0, wheight = -1;
	TTimeSignature timeSignature = TTimeSignature::FourFour;

	std::vector<TNote> notes;

	static std::string type() { return "Piano Roll"; }

private:
	bool Grid(float height, ImVec2 owp) {
		ImGuiStyle& style = ImGui::GetStyle();
		const ImGuiIO io = ImGui::GetIO();

		const float n1_16width = 10;
		const float n_height = 7;
		const int snotes = SNOTES;
		const float lineHeight = ImGui::GetItemsLineHeightWithSpacing() + style.ItemInnerSpacing.y;
		const float nheight = wheight == -1 ? lineHeight * 6 : wheight;

		const bool mouseDragged = ImGui::IsMouseDragging(0);
		const ImVec2 mp = ImGui::GetMousePos();

		int barsPerMeasure = 0;
		switch (timeSignature) {
			case ThreeFour: barsPerMeasure = 3; break;
			case FourFour: barsPerMeasure = 4; break;
			default: barsPerMeasure = 5; break;
		}

		int noteCount = measures * barsPerMeasure * snotes;
		float w = noteCount * n1_16width;

		const int gridColor0 = IM_COL32(120, 120, 120, 120);
		const int gridColor1 = IM_COL32(120, 120, 120, 220);
		const int gridColor2 = IM_COL32(150, 150, 150, 255);
		const int gridColor3 = IM_COL32(120, 255, 120, 128);
		const int noteColor = IM_COL32(0, 200, 100, 255);
		const int noteColorLight = IM_COL32(0, 255, 155, 255);
		const int noteColorDark = IM_COL32(0, 50, 10, 255);

		const bool oldActive = ImGui::IsAnyItemActive();

		const ImVec2 wp = ImGui::GetCursorScreenPos();
		ImGui::InvisibleButton("##pianoroll", ImVec2(w, height));
		ImDrawList* dl = ImGui::GetWindowDrawList();

		dl->PushClipRect(owp+ImVec2(1, 0), owp+ImVec2(350 - style.ScrollbarSize, nheight + style.ScrollbarSize + style.ItemInnerSpacing.y), true);

		dl->AddRectFilled(
			wp+ImVec2(1, 0), wp+ImVec2(w, height),
			IM_COL32(0, 0, 0, 255)
		);

		float x = 0;
		for (int n = 0; n < noteCount; n++) {
			dl->AddLine(
				ImVec2(x, 0) + wp,
				ImVec2(x, height) + wp,
				gridColor0
			);
			x += n1_16width;
		}

		float y = 0;
		for (int k = 0; k <= MAX_PIANO_KEYS; k++) {
			dl->AddLine(
				ImVec2(0, y) + wp,
				ImVec2(w, y) + wp,
				gridColor0
			);
			y += n_height;
		}

		x = 0;
		for (int n = 0; n <= barsPerMeasure * measures; n++) {
			dl->AddLine(
				ImVec2(x, 0) + wp,
				ImVec2(x, height) + wp,
				gridColor1, 2
			);
			x += n1_16width*snotes;
		}

		x = 0;
		for (int n = 1; n <= measures; n++) {
			dl->AddLine(
				ImVec2(x, 0) + wp,
				ImVec2(x, height) + wp,
				gridColor3, 1.5f
			);
			x += n1_16width * (barsPerMeasure * snotes);
		}

		movingNote = false;
		int n = 0;
		for (auto& e : notes) {
			float x = e.position * n1_16width;
			float y = (MAX_PIANO_KEYS - 1 - e.note) * n_height;
			float x1 = (e.position + e.length) * n1_16width;
			e.rect.Min = ImVec2(x, y) + wp;
			e.rect.Max = ImVec2(x1, y + n_height) + wp;
			dl->AddRectFilled(
				e.rect.Min,
				e.rect.Max,
				noteColorDark, 1.0f
			);
			dl->AddRectFilled(
				e.rect.Min,
				e.rect.Max - ImVec2(0, 1),
				n == selectedNote ? noteColorLight : noteColor, 1.0f
			);

			float nw = e.rect.Max.x - e.rect.Min.x;
			const ImRect rsz0 = ImRect(ImVec2(x, y) + wp, ImVec2(x+4, y+n_height) + wp);
			const ImRect rsz1 = ImRect(ImVec2(x + nw-4, y) + wp, ImVec2(x + nw, y+n_height) + wp);

			bool resz = (rsz0.Contains(mp) || rsz1.Contains(mp));

			if (!resizingNote) {
				if (ImGui::IsMouseClicked(0)) {
					resizingNote = true;
					resizeHandle = rsz0.Contains(mp) ? 0 : rsz1.Contains(mp) ? 1 : -1;
					if (resizeHandle == -1) resizingNote = false;
				}
			} else {
				if (ImGui::IsMouseReleased(0)) {
					resizingNote = false;
				}
			}

			if (resz) {
				ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
			} else {
				ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
			}

			// Resizing
			if (resizingNote && mouseDragged && n == selectedNote) {
				float _mx = mp.x - wp.x;
				float mx = (_mx / n1_16width);
				if (resizeHandle == 0) { // LEFT
					int delta = int(mx) - e.position;
					e.position += delta;
					if (e.position > 0 && e.position < MAX_PIANO_KEYS)
						e.length -= delta;
				} else if (resizeHandle == 1) {
					int delta = int(mx) - (e.position + e.length);
					e.length += delta;
				}
				e.position = ImClamp(e.position, 0, noteCount-1);
				e.length = ImClamp(e.length, 1, MAX_PIANO_KEYS-1);
			}
			
			if (mouseDragged && n == selectedNote && !resizingNote) {
				float mx = mp.x - wp.x;
				float my = mp.y - wp.y;

				e.position = int(mx / n1_16width);
				e.note = MAX_PIANO_KEYS - 1 - int(my / n_height);

				e.note = ImClamp(e.note, 0, MAX_PIANO_KEYS-1);
				e.position = ImClamp(e.position, 0, noteCount-1);

				movingNote = true;
			}
			n++;
		}

		float pX = noteIndex * n1_16width;
		dl->AddRectFilled(
			ImVec2(pX + 1, 0) + wp,
			ImVec2(pX + n1_16width, height) + wp,
			gridColor3
		);

		dl->PopClipRect();

		// Hovering notes
		hoveredNote = -1;
		n = 0;
		for (auto& e : notes) {
			if (e.rect.Contains(mp)) {
				hoveredNote = n;
				break;
			}
			n++;
		}

		// Adding notes
		if (ImGui::IsItemActive() && ImGui::IsMouseClicked(0) && !oldActive && hoveredNote == -1 && selectedNote == -1) {
			float mx = mp.x - wp.x;
			float my = mp.y - wp.y;
			uint16_t noteXID = uint16_t(mx / n1_16width);
			uint16_t noteID = MAX_PIANO_KEYS - 1 - uint16_t(my / n_height);
			if (noteXID >= 0 && noteXID < noteCount && noteID >= 0 && noteID < MAX_PIANO_KEYS) {
				int ID = (noteXID & 0xFFFF) << 16 | (noteID & 0xFFFF);
				notes.push_back({ noteXID, 1, noteID, { 0, 0, 0, 0 } });
			}
		} else if (ImGui::IsMouseClicked(0) && selectedNote != -1) {
			selectedNote = -1;
		}

		// Selecting notes
		if (ImGui::IsItemActive() && ImGui::IsMouseClicked(0) && hoveredNote != -1 && !movingNote) {
			selectedNote = hoveredNote;
		}

		// Deleting notes
		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1) && hoveredNote != -1) {
			float mx = mp.x - wp.x;
			float my = mp.y - wp.y;
			notes.erase(notes.begin() + hoveredNote);
			hoveredNote = -1;
		}

		return false;
	}

	int hoveredNote = -1, selectedNote = -1, resizeHandle = -1;
	bool movingNote = false, resizingNote = false;

};

#endif // T_PIANO_ROLL_NODE_H