#ifndef TWEN_NOTE_NODE_H
#define TWEN_NOTE_NODE_H

#include "../Node.h"

class NoteNode : public Node {
	TWEN_NODE(NoteNode, "Note")
public:
	NoteNode(Note note, float oct=0) : Node() {
		addOutput("Nt");

		addParam("Note", { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }, note);
		addParam("Oct", 0.0f, 5.0f, oct, 1.0f, NodeParam::DragRange);
	}

	void solve() {
		int note = (int) param("Note");
		int oct = (int) param("Oct");
		out("Nt") = (int)note + (oct * 12);
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
		FloatArray in = ins("Nt");
		FloatArray _out;
		for (int i = 0; i < FLOAT_ARRAY_MAX; i++)
			_out[i] = Utils::noteFrequency(int(in[i]));
		outs("Freq") = _out;
	}

};

#endif // TWEN_NOTE_NODE_H
