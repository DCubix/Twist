#ifndef T_MIDI_NODE_H
#define T_MIDI_NODE_H

#include "../TNodeGraph.h"
#include "../TNodeEditor.h"
#include "RtMidi.h"

class MIDINode : public Node, public TMidiMessageSubscriber {
	TWEN_NODE(MIDINode, "MIDI")
public:
	MIDINode() : Node() {
		addOutput("Nt");
		addOutput("Gate");
		gates.fill(false);

		addParam("Channel", 0x0, 0xF, 0.0f, 1.0f, NodeParam::Range);
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

	int midiChannel() { return (int) param(0); }

	void solve() {
//		outs(0).set(0.0f);
		outs(1).set(0.0f);

		for (int i = 0, s = 0; i < 128; i++) {
			if (gates[i]) {
				s = s % FLOAT_ARRAY_MAX;
				out(0, s) = i;
				out(1, s) = 1.0f;
				s++;
			}
		}
	}

	int noteID = 0;
	std::array<bool, 128> gates;
};

#endif // T_MIDI_NODE_H
