#ifndef T_NOTE_NODE_H
#define T_NOTE_NODE_H

#include "TNode.h"
#include "../../TGen.h"

class TNoteNode : public TNode {
public:
	TNoteNode(Notes note=Notes::C, int oct=0)
		: TNode("Note", 160, 125), note(note), oct(oct)
	{
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
		ImGui::PushItemWidth(45);
		ImGui::Combo("Note", (int*)&note, NOTES, 12, -1);
		ImGui::DragInt("Oct", &oct, 0.1f, 0, 5);
		ImGui::PopItemWidth();
	}

	void solve() {
		setOutput(0, (int)note + (oct * 12));
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

class TFreqNode : public TNode {
public:
	TFreqNode() : TNode("Freq", 0, 0) {
		addInput("Nt");
		addOutput("Freq");
	}

	void gui() {
		
	}

	void solve() {
		FloatArray in = getMultiInputValues(0);
		FloatArray out;
		for (int i = 0; i < FLT_ARR_MAX; i++)
			out[i] = tgen::note(int(in[i]));
		setMultiOutputValues(0, out);
	}

	void save(JSON& json) {
		TNode::save(json);
		json["type"] = type();
	}
	

	static std::string type() { return "Frequency"; }
};

#endif // T_NOTE_NODE_H
