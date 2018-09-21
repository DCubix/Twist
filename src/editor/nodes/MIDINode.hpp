#ifndef T_MIDI_NODE_H
#define T_MIDI_NODE_H

#include "../TNodeGraph.h"
#include "../TNodeEditor.h"
#include "RtMidi.h"

#include "twen/intern/Voice.h"

struct NoteVoice : Voice {
	NoteVoice() : Voice() {}
	float sample() { return noteIndex; }
	u8 noteIndex;
};

class MIDINode : public Node, public TMidiMessageSubscriber, public VoiceManager<NoteVoice, FLOAT_ARRAY_MAX> {
	TWEN_NODE(MIDINode, "MIDI")
public:
	MIDINode() : Node() {
		addOutput("Nt");
		addOutput("Gate");

		addParam("Channel", 0x0, 0xF, 0.0f, 1.0f, NodeParam::IntRange);
		addParam("Transpose", -24, 24, 0.0f, 1.0f, NodeParam::IntRange);
	}

	void messageReceived(TMidiMessage msg) {
		// msg.debugPrint();
		switch (msg.command) {
			default: break;
			case TMidiCommand::NoteOn:
				if (msg.param1 > 0) {
					noteOn(msg.param0);
				} else {
					noteOff(msg.param0);
				}
				break;
			case TMidiCommand::NoteOff:
				noteOff(msg.param0);
				break;
		}
	}

	int midiChannel() { return (int) param(0); }

	void onTrigger(NoteVoice* voice, u8 note) {
		voice->noteIndex = note;
	}

	void onRelease(NoteVoice* voice, u8 note) {
		if (voice->triggered && voice->noteIndex == note) voice->triggered = false;
	}

	void solve() {
		outs(1).set(0.0f);

		for (u32 i = 0; i < size(); i++) {
			if (get(i).triggered) {
				out(0, i) = get(i).sample() + param(1);
				out(1, i) = 1.0f;
			}
		}
	}

	int noteID = 0;
};

#endif // T_MIDI_NODE_H
