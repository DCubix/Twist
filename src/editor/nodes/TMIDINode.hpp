#ifndef T_MIDI_NODE_H
#define T_MIDI_NODE_H

#include "TNode.h"
#include "../TNodeGraph.h"
#include "../TNodeEditor.h"
#include "RtMidi.h"

class TMIDINode : public TNode {
public:
	TMIDINode() : TNode("MIDI In", 0, 0) {

	}

	void setup() {
		RtMidiIn* midi = parent()->editor()->midiIn();
		
	}

	void gui() {

	}

	void solve() {
		
	}

	void save(JSON& json) {
		TNode::save(json);
		json["type"] = type();
	}

	static std::string type() { return "MIDI"; }

	int channel;
};

#endif // T_MIDI_NODE_H