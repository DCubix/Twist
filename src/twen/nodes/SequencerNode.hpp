#ifndef TWEN_SEQUENCER_NODE_H
#define TWEN_SEQUENCER_NODE_H

#include "../Node.h"

#define SEQUENCER_SIZE 8
#define SEQUENCER_SIZE_VISIBLE SEQUENCER_SIZE

class SequencerNode : public Node {
	TWEN_NODE(SequencerNode, "Sequencer")
public:
	SequencerNode(u32 key=0) : Node(), noteIndex(0) {
		addInput("Index");
		addInput("Key");

		addOutput("Nt");
		addOutput("Gate");

		std::memset(octs, 0, sizeof(int) * SEQUENCER_SIZE);
		std::memset(enabled, 1, sizeof(bool) * SEQUENCER_SIZE);
		for (int i = 0; i < SEQUENCER_SIZE; i++) {
			notes[i] = Note::C;
		}

		addParam("Key", { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }, key);
	}

	void save(JSON& json) {
		Node::save(json);
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

	void load(JSON json) {
		Node::load(json);
		if (json["notes"].is_array()) {
			for (int i = 0; i < json["notes"].size(); i++) {
				notes[i] = json["notes"][i];
			}
		}
		if (json["octs"].is_array()) {
			for (int i = 0; i < json["octs"].size(); i++) {
				octs[i] = json["octs"][i];
			}
		}
		if (json["enabled"].is_array()) {
			for (int i = 0; i < json["enabled"].size(); i++) {
				enabled[i] = json["enabled"][i].get<bool>();
			}
		}
	}

	void solve() {
		int ni = noteIndex = (int) in(0);
		int nid = ni % SEQUENCER_SIZE;
		Note note = notes[nid];

		// Transpose note
		int nkey = (int) in(1, 0);

		int fnote = (int) notes[0];
		int noteDiff = (nkey - fnote);
		int nnote = ((int)note + noteDiff);
		int oct = octs[nid] + Utils::octave(nnote);

		if (!enabled[nid]) {
			globGate = true;
			out(1) = 0.0f;
		}

		out(0) = nnote + (oct * 12);

		if (prevNoteIndex != noteIndex) {
			globGate = true;
			prevNoteIndex = noteIndex;
		}

		if (globGate) {
			out(1) = 0.0f;
			globGate = false;
		} else {
			out(1) = 1.0f;
		}
	}

	bool globGate = false;
	int noteIndex = 0, prevNoteIndex = 12;

	Note notes[SEQUENCER_SIZE];
	int octs[SEQUENCER_SIZE];
	bool enabled[SEQUENCER_SIZE];

};

#endif // TWEN_SEQUENCER_NODE_H
