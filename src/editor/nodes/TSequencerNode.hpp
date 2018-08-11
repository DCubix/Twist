#ifndef T_SEQUENCER_NODE_H
#define T_SEQUENCER_NODE_H

#include "TNode.h"

#define SEQUENCER_SIZE 8
class TSequencerNode : public TNode {
public:
	TSequencerNode()
		: TNode("Sequencer", 410, 150),
			noteIndex(0), out(0.0f), key(Notes::C)
	{
		addInput("Mod");
		addInput("Reset");
		addInput("Index");
		addInput("Gate");
		addInput("Key");
		addOutput("Freq");
		addOutput("Gate");
		std::memset(notes, 0, sizeof(Notes) * SEQUENCER_SIZE);
		std::memset(octs, 0, sizeof(int) * SEQUENCER_SIZE);
	}

	void gui() {
		static const char* NOTES[] = {
			"--\0",
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
				ImGui::Combo(id, (int*) &notes[i], NOTES, 13);
			}
			ImGui::PopItemWidth();
		ImGui::EndHorizontal();
		
		ImGui::BeginHorizontal(this);
			ImGui::PushItemWidth(32);
			for (int i = 0; i < SEQUENCER_SIZE; i++) {
				char id[5];
				id[0] = '#'; id[1] = '#'; id[2] = (i+1); id[3] = 'o'; id[4] = 0;
				ImGui::DragInt(id, &octs[i], 0.25f, -8, 8);
			}
			ImGui::PopItemWidth();
		ImGui::EndHorizontal();

		ImGui::Combo("##key_note", (int*) &key, NOTES, 13);
	}

	void solve() {
		if (getInputOr(1, 0.0f) > 0.0f) {
			noteIndex = 0;
		}

		int ni = noteIndex = (int) getInputOr(2, 0);
		bool gate = getInputOr(3, 0) > 0.0f ? true : false;

		Notes note = notes[ni % SEQUENCER_SIZE];

		// Transpose note
		int nkey = (int) getInputOr(4, (int)key);
		int fnote = (int) notes[0];
		int noteDiff = std::abs(nkey - fnote);
		int nnote = ((int)note + noteDiff);

		if (gate) {
			if (note != Notes::Silence)
				out = NOTE(nnote) * std::pow(2, octs[ni % SEQUENCER_SIZE]);
		} else {
			out = 0.0f;
		}

		setOutput(0, getInput(0) + out);
		setOutput(1, gate ? 1.0f : 0.0f);
	}

	virtual void save(JSON& json) {
		TNode::save(json);
		json["type"] = type();
		json["key"] = (int)key;

		std::array<int, SEQUENCER_SIZE> notes_;
		std::array<int, SEQUENCER_SIZE> octs_;
		std::copy(std::begin(octs), std::end(octs), std::begin(octs_));
		for (int i = 0; i < SEQUENCER_SIZE; i++) notes_[i] = (int) notes[i];
		json["notes"] = notes_;
		json["octs"] = octs_;
	}

	int noteIndex;
	float out;
	Notes notes[SEQUENCER_SIZE];
	int octs[SEQUENCER_SIZE];

	Notes key;

	static std::string type() { return "Sequencer"; }
};

#endif // T_SEQUENCER_NODE_H