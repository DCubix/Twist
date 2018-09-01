#ifndef T_MIDI_NODE_H
#define T_MIDI_NODE_H

#include "TNode.h"
#include "../TNodeGraph.h"
#include "../TNodeEditor.h"
#include "RtMidi.h"

class TMIDINode : public TNode, public TMidiMessageSubscriber {
public:
	TMIDINode() : TNode("MIDI In", 0, 0) {
		addOutput("Nt");
		addOutput("Gate");
		gates.fill(false);
	}

	void messageReceived(TMidiMessage msg) {
		// msg.debugPrint();
		switch (msg.command) {
			default: break;
			case TMidiCommand::NoteOn:
				gates[msg.param0] = (msg.param1 > 0);
				break;
			case TMidiCommand::NoteOff:
				gates[msg.param0] = false;
				break;
		}
	}

	int midiChannel() { return channel; }

	void setup() {
		
	}

	void gui() {
		ImGui::InputInt("Chan.", &channel, 1, 2);
		channel = ImClamp(channel, 0x0, 0xF);
	}

	void solve() {
		for (int i = 0; i < TNODE_MAX_SIMULTANEOUS_VALUES_PER_SLOT; i++) {
			setMultiOutput(1, i, 0.0f);
		}
		for (int i = 0, s = 0; i < 128; i++) {
			if (gates[i]) {
				s = s % TNODE_MAX_SIMULTANEOUS_VALUES_PER_SLOT;
				setMultiOutput(0, s, i);
				setMultiOutput(1, s, 1.0f);
				s++;
			}
		}
	}

	void save(JSON& json) {
		TNode::save(json);
		json["type"] = type();
	}

	static std::string type() { return "MIDI"; }

	int channel = 0xF, noteID = 0;
	std::array<bool, 128> gates;
};

#endif // T_MIDI_NODE_H