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

	void save(JSON& json) override {
		Node::save(json);
		json["note"] = int(note);
		json["oct"] = oct;
	}

	void load(JSON json) override {
		Node::load(json);
		note = Note(json["note"].get<int>());
		oct = json["oct"];
	}

	Note note;
	u32 oct;
};

#endif // TWEN_NOTE_NODE_H
