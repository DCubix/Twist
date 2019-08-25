#ifndef TWEN_NOTE_NODE_H
#define TWEN_NOTE_NODE_H

#include "../NodeGraph.h"

class NoteNode : public Node {
	TWEN_NODE(NoteNode, "Note")
public:
	inline NoteNode(Note note, u32 oct=0)
		: Node(), note(note), oct(oct)
	{
		addInput("Base");
	}

	inline Value sample(NodeGraph *graph) override {
		int base = connected(0) ? int(in(0).value()) : 0;
		return Value(u32(note) + (12 * oct) + base);
	}

	inline void save(JSON& json) override {
		Node::save(json);
		json["note"] = int(note);
		json["oct"] = oct;
	}

	inline void load(JSON json) override {
		Node::load(json);
		note = Note(json["note"].get<int>());
		oct = json["oct"];
	}

	Note note;
	u32 oct;
};

class HertzNode : public Node {
	TWEN_NODE(HertzNode, "Hertz")
public:
	inline HertzNode()
		: Node()
	{
		addInput("Note");
	}

	inline Value sample(NodeGraph *graph) override {
		return Value(Utils::noteFrequency(in(0).value()));
	}

	inline void save(JSON& json) override {
		Node::save(json);
	}

	inline void load(JSON json) override {
		Node::load(json);
	}
};

#endif // TWEN_NOTE_NODE_H
