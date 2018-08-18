#ifndef T_NOTE_NODE_H
#define T_NOTE_NODE_H

#include "TNode.h"
#include "../../TGen.h"

class TNoteNode : public TNode {
public:
	TNoteNode(Notes note=Notes::C, int oct=0)
		: TNode("Note", 160, 125), note(note), oct(oct)
	{
		addInput("Mod");
		addOutput("Out");
		addOutput("Nt");
	}

	void gui() {
		static const char* NOTES[] = {
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
		ImGui::Combo("Note", (int*)&note, NOTES, 12, -1);
		ImGui::DragInt("Oct", &oct, 1, -7, 7);
	}

	void solve() {
		setOutput(0, getInput(0) + tgen::note(note, oct));
		setOutput(1, (int)note + (oct * 12));
	}

	void save(JSON& json) {
		TNode::save(json);
		json["type"] = type();
		json["note"] = (int) note;
		json["oct"] = oct;
	}

	Notes note;
	int oct;

	static std::string type() { return "Note"; }
};

#endif // T_NOTE_NODE_H
