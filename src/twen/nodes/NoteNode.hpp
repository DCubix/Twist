#ifndef TWEN_NOTE_NODE_H
#define TWEN_NOTE_NODE_H

#include "../Node.h"

class NoteNode : public Node {
	TWEN_NODE(NoteNode)
public:
	NoteNode(Note note = Note::C, int oct=0)
		: Node(), note(note), oct(oct)
	{
		addOutput("Nt");
	}

	void solve() {
		setOutput("Nt", (int)note + (oct * 12));
	}

	Note note;
	int oct;
};

class FreqNode : public Node {
	TWEN_NODE(FreqNode)
public:
	FreqNode() : Node() {
		addInput("Nt");
		addOutput("Freq");
	}

	void solve() {
		FloatArray in = inputs("Nt");
		FloatArray out;
		for (int i = 0; i < FLOAT_ARRAY_MAX; i++)
			out[i] = Utils::noteFrequency(int(in[i]));
		setMultiOutputValues("Freq", out);
	}

};

#endif // TWEN_NOTE_NODE_H
