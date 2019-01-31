#ifndef TWEN_NOTE_NODE_H
#define TWEN_NOTE_NODE_H

#include "../NodeGraph.h"

class NoteNode : public Node {
	TWEN_NODE(NoteNode, "Note")
public:
	NoteNode(Note note, u32 oct=0)
		: Node(), note(note), oct(oct)
	{}

	float sample(NodeGraph *graph) override {
		return Utils::noteFrequency(u32(note) % 12, oct);
	}

	Note note;
	u32 oct;
};

#endif // TWEN_NOTE_NODE_H
