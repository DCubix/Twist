#ifndef T_MIDI_H
#define T_MIDI_H

#include <cstdint>
#include <array>

#include "RtMidi.h"

using TByte = uint8_t;
using TShort = uint16_t;
using TRawMidiMessage = std::array<TByte, 3>;

struct TMidiMessage {
	enum TMidiCommand {
		NoteOff = 0x8,
		NoteOn = 0x9,
		Aftertouch = 0xA,
		ContinuousController = 0xB,
		PatchChange = 0xC,
		ChannelPressure = 0xD,
		PitchBend = 0xE,
		System = 0xF
	};

	TMidiCommand command;
	TByte channel;
	TByte param0, param1;

	TMidiMessage() {
		command = NoteOff;
		channel = 0;
		param0 = 0;
		param1 = 0;
	}
	TMidiMessage(const TRawMidiMessage& data);
	void debugPrint();
};

#endif // T_MIDI_H