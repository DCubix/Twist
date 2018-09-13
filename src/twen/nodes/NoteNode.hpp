#ifndef TWEN_NOTE_NODE_H
#define TWEN_NOTE_NODE_H

#include "../Node.h"

class NoteNode : public Node {
	TWEN_NODE(NoteNode, "Note")
public:
	NoteNode(Note note, float oct=0) : Node() {
		addOutput("Nt");

		addParam("Note", { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }, note);
		addParam("Oct", 0.0f, 5.0f, oct, 1.0f, NodeParam::IntRange);
	}

	void solve() {
		int note = (int) paramOption(0);
		int oct = (int) param(1);
		out(0) = (int)note + (oct * 12);
	}
};

class FreqNode : public Node {
	TWEN_NODE(FreqNode, "Frequency")
public:
	FreqNode() : Node() {
		addInput("Nt");
		addOutput("Freq");
	}

	void solve() {
		for (u32 i = 0; i < FLOAT_ARRAY_MAX; i++)
			out(0, i) = Utils::noteFrequency(int(in(0, 0, i)));
	}

};

#endif // TWEN_NOTE_NODE_H
