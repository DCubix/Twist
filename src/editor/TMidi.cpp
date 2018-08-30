#include "TMidi.h"

#include <stdio.h>

TMidiMessage::TMidiMessage(const TRawMidiMessage& data) {
	command = (TMidiCommand)((data[0] & 0xF0) >> 4);
	channel = data[0] & 0xF;
	param0 = data[1] & 0x7F;
	param1 = data[2] & 0x7F;
}

void TMidiMessage::debugPrint() {
	printf("[MIDI]: ");
	switch (command) {
		case System: printf("System"); break;
		case NoteOn: printf("NoteOn"); break;
		case NoteOff: printf("NoteOff"); break;
		case PitchBend: printf("PitchBend"); break;
		case Aftertouch: printf("Aftertouch"); break;
		case PatchChange: printf("PatchChange"); break;
		case ChannelPressure: printf("ChannelPressure"); break;
		case ContinuousController: printf("ContinuousController"); break;
	}
	printf("(CH %d, %d, %d)\n", channel, param0, param1);
}