#ifndef T_SEQUENCER_NODE_H
#define T_SEQUENCER_NODE_H

#include "TNode.h"

#define SEQUENCER_SIZE 8
class TSequencerNode : public TNode {
public:
	TSequencerNode()
		: TNode("Sequencer", 410, 150),
			noteIndex(0), out(0.0f), key((int) Notes::C)
	{
		addInput("Mod");
		addInput("Index");
		addInput("Key");

		addOutput("Freq");
		addOutput("Nt");
		addOutput("Gate");

		std::memset(octs, 0, sizeof(int) * SEQUENCER_SIZE);
		std::memset(enabled, 1, sizeof(bool) * SEQUENCER_SIZE);
		for (int i = 0; i < SEQUENCER_SIZE; i++) {
			notes[i] = Notes::C;
		}
	}

	void gui() {
		static const char* NOTES[] = {
			"C\0",
			"C#\0",
			"D\0",
			"D#\0",
			"E\0",
			"F\0",
			"F#\0",
			"G\0",
			"G#\0",
			"A\0",
			"A#\0",
			"B\0",
			0
		};

		ImGui::BeginHorizontal(this);
			for (int i = 0; i < SEQUENCER_SIZE; i++) {
				bool pushed = false;
				if (i == (noteIndex % SEQUENCER_SIZE)) {
					pushed = true;
					ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(120, 250, 120, 255));
				}
				if (ImGui::Button(" ", ImVec2(32, 0))) {
					noteIndex = i;
				}
				if (pushed) {
					ImGui::PopStyleColor();
				}
			}
		ImGui::EndHorizontal();

		ImGui::BeginHorizontal(this);
			ImGui::PushItemWidth(32);
			for (int i = 0; i < SEQUENCER_SIZE; i++) {
				char id[4];
				id[0] = '#'; id[1] = '#'; id[2] = (i+1); id[3] = 0;
				ImGui::Combo(id, (int*) &notes[i], NOTES, 12);
			}
			ImGui::PopItemWidth();
		ImGui::EndHorizontal();

		ImGui::BeginHorizontal(this);
			ImGui::PushItemWidth(32);
			for (int i = 0; i < SEQUENCER_SIZE; i++) {
				char id[5];
				id[0] = '#'; id[1] = '#'; id[2] = (i+1); id[3] = 'o'; id[4] = 0;
				ImGui::DragInt(id, &octs[i], 0.1f, -5, 5);
			}
			ImGui::PopItemWidth();
		ImGui::EndHorizontal();

		ImGui::BeginHorizontal(this);
			ImGui::PushItemWidth(32);
			for (int i = 0; i < SEQUENCER_SIZE; i++) {
				char id[5];
				id[0] = '#'; id[1] = '#'; id[2] = (i+1); id[3] = 's'; id[4] = 0;
				ImGui::ToggleButton(id, &enabled[i]);
			}
			ImGui::PopItemWidth();
		ImGui::EndHorizontal();

		ImGui::Combo("##key_note", (int*) &key, NOTES, 12);
	}

	void solve() {
		int ni = noteIndex = (int) getInputOr(1, 0);
		int nid = ni % SEQUENCER_SIZE;
		Notes note = notes[nid];

		// Transpose note
		int nkey = (int) getInputOr(2, key);

		int fnote = (int) notes[0];
		int noteDiff = (nkey - fnote);
		int nnote = ((int)note + noteDiff);
		int oct = octs[nid] + tgen::octave(nnote);

		if (enabled[nid]) {
			out = tgen::note(nnote, oct);
		} else {
			out = 0.0f;
			globGate = true;
			setOutput(2, 0.0f);
		}

		setOutput(0, getInput(0) + out);
		setOutput(1, nnote + (oct * 12));

		if (prevNoteIndex != noteIndex) {
			globGate = true;
			prevNoteIndex = noteIndex;
		}

		if (globGate) {
			setOutput(2, 0.0f);
			globGate = false;
		} else {
			setOutput(2, 1.0f);
		}
	}

	void save(JSON& json) {
		TNode::save(json);
		json["type"] = type();
		json["key"] = key;

		std::array<int, SEQUENCER_SIZE> notes_;
		std::array<int, SEQUENCER_SIZE> octs_;
		std::array<bool, SEQUENCER_SIZE> silen_;
		for (int i = 0; i < SEQUENCER_SIZE; i++) {
			notes_[i] = (int) notes[i];
			octs_[i] = octs[i];
			silen_[i] = enabled[i];
		}
		json["notes"] = notes_;
		json["octs"] = octs_;
		json["enabled"] = silen_;
	}

	bool globGate = false;
	int noteIndex = 0, prevNoteIndex = 12;
	float out;

	Notes notes[SEQUENCER_SIZE];
	int octs[SEQUENCER_SIZE];
	bool enabled[SEQUENCER_SIZE];

	int key;

	static std::string type() { return "Sequencer"; }
};

#endif // T_SEQUENCER_NODE_H
