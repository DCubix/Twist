#ifndef TWEN_SEQUENCER_NODE_H
#define TWEN_SEQUENCER_NODE_H

#include "../Node.h"

#define SEQUENCER_SIZE 8
#define SEQUENCER_SIZE_VISIBLE SEQUENCER_SIZE

class SequencerNode : public Node {
	TWEN_NODE(SequencerNode)
public:
	SequencerNode()
		: Node(),
		noteIndex(0), out(0.0f)
	{
		addInput("Index");
		addInput("Key");

		addOutput("Nt");
		addOutput("Gate");

		std::memset(octs, 0, sizeof(int) * SEQUENCER_SIZE);
		std::memset(enabled, 1, sizeof(bool) * SEQUENCER_SIZE);
		for (int i = 0; i < SEQUENCER_SIZE; i++) {
			notes[i] = Note::C;
		}
	}

	void solve() {
		int ni = noteIndex = (int) getInput("Index");
		int nid = ni % SEQUENCER_SIZE;
		Note note = notes[nid];

		// Transpose note
		int nkey = (int) getInput("Key");

		int fnote = (int) notes[0];
		int noteDiff = (nkey - fnote);
		int nnote = ((int)note + noteDiff);
		int oct = octs[nid] + Utils::octave(nnote);

		if (!enabled[nid]) {
			globGate = true;
			setOutput("Gate", 0.0f);
		}

		setOutput("Nt", nnote + (oct * 12));

		if (prevNoteIndex != noteIndex) {
			globGate = true;
			prevNoteIndex = noteIndex;
		}

		if (globGate) {
			setOutput("Gate", 0.0f);
			globGate = false;
		} else {
			setOutput("Gate", 1.0f);
		}
	}

	bool globGate = false;
	int noteIndex = 0, prevNoteIndex = 12;
	float out;

	Note notes[SEQUENCER_SIZE];
	int octs[SEQUENCER_SIZE];
	bool enabled[SEQUENCER_SIZE];

};

#endif // TWEN_SEQUENCER_NODE_H
