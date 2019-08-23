#ifndef TWIST_MIDI_NODE_HPP
#define TWIST_MIDI_NODE_HPP

#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"

#include "twen/NodeGraph.h"
#include "../TMidi.h"

class MIDINode : public Node, public TMidiMessageSubscriber  {
	TWEN_NODE(MIDINode, "MIDI In")
public:
	inline MIDINode()
		: Node()
	{}

	inline MIDINode(JSON param)
		: Node()
	{
		load(param);
	}

	inline void messageReceived(TMidiMessage msg) override {
		switch (msg.command) {
			default: break;
			case TMidiCommand::NoteOn:
				if (msg.param0 >= from && msg.param0 <= to) {
					out.gate = (msg.param1 > 0);
					out.velocity = float(msg.param1) / 128.0f;
					out.value = msg.param0 - 21;
				}
				break;
			case TMidiCommand::NoteOff:
				out.gate = false;
				break;
		}
	}

	inline int midiChannel() override { return channel; }

	inline Value sample(NodeGraph *graph) override {
		return out;
	}

	inline void save(JSON& json) override {
		Node::save(json);
		json["channel"] = channel;
		json["from"] = from;
		json["to"] = to;
	}

	inline void load(JSON json) override {
		Node::load(json);
		if (json["channel"].is_null()) return;
		channel = json["channel"].get<u32>();
		from = json["from"].get<u32>();
		to = json["to"].get<u32>();
	}

	u32 channel{ 15 };
	u32 from{ 21 }, to{ 108 };

	int noteID{ 0 };

	Value out;
};

static void MIDI_gui(Node* node) {
	MIDINode *n = dynamic_cast<MIDINode*>(node);

	ImGui::PushItemWidth(50);
	ImGui::DragInt("Ch.", (int*)&n->channel, 1.0f, 0, 15);

	static const char* NOTES[] = {
		"A0", "A#0", "B0",
		"C1", "C#1", "D1", "D#1", "E1", "F1", "F#1", "G1", "G#1", "A1", "A#1", "B1",
		"C2", "C#2", "D2", "D#2", "E2", "F2", "F#2", "G2", "G#2", "A2", "A#2", "B2",
		"C3", "C#3", "D3", "D#3", "E3", "F3", "F#3", "G3", "G#3", "A3", "A#3", "B3",
		"C4", "C#4", "D4", "D#4", "E4", "F4", "F#4", "G4", "G#4", "A4", "A#4", "B4",
		"C5", "C#5", "D5", "D#5", "E5", "F5", "F#5", "G5", "G#5", "A5", "A#5", "B5",
		"C6", "C#6", "D6", "D#6", "E6", "F6", "F#6", "G6", "G#6", "A6", "A#6", "B6",
		"C7", "C#7", "D7", "D#7", "E7", "F7", "F#7", "G7", "G#7", "A7", "A#7", "B7",
		"C8"
	};

	ImGui::Text("Keyboard Range");

	int v[] = { n->from - 21, n->to - 21 };
	if (ImGui::Combo("From", &v[0], NOTES, 88)) {
		n->from = v[0] + 21;
	}
	ImGui::SameLine();
	if (ImGui::Combo("To", &v[1], NOTES, 88)) {
		n->to = v[1] + 21;
	}

	if (n->from > n->to) {
		std::swap(n->from, n->to);
	}

	ImGui::PopItemWidth();
}

#endif // TWIST_MIDI_NODE_HPP
