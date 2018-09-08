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

	void solve() {
		int ni = noteIndex = (int) in("Index");
		int nid = ni % SEQUENCER_SIZE;
		Note note = notes[nid];

		// Transpose note
		int nkey = (int) in("Key", "Key");

		int fnote = (int) notes[0];
		int noteDiff = (nkey - fnote);
		int nnote = ((int)note + noteDiff);
		int oct = octs[nid] + Utils::octave(nnote);

		if (!enabled[nid]) {
			globGate = true;
			out("Gate") = 0.0f;
		}

		out("Nt") = nnote + (oct * 12);

		if (prevNoteIndex != noteIndex) {
			globGate = true;
			prevNoteIndex = noteIndex;
		}

		if (globGate) {
			out("Gate") = 0.0f;
			globGate = false;
		} else {
			out("Gate") = 1.0f;
		}
	}

	bool globGate = false;
	int noteIndex = 0, prevNoteIndex = 12;

	Note notes[SEQUENCER_SIZE];
	int octs[SEQUENCER_SIZE];
	bool enabled[SEQUENCER_SIZE];

};

#endif // TWEN_SEQUENCER_NODE_H
