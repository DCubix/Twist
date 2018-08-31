#ifndef T_MIDI_H
#define T_MIDI_H

#include <cstdint>
#include <array>
#include <vector>
#include <map>

#include "RtMidi.h"

using TByte = uint8_t;
using TShort = uint16_t;
using TRawMidiMessage = std::array<TByte, 3>;

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

struct TMidiMessage {
	TMidiCommand command;
	TByte channel;
	TByte param0, param1;

	TShort param() { return (param0 & 0x7F) | (param1 & 0x7F) << 7; }
	TRawMidiMessage raw();

	TMidiMessage() {
		command = NoteOff;
		channel = 0;
		param0 = 0;
		param1 = 0;
	}
	TMidiMessage(const TRawMidiMessage& data);
	void debugPrint();
};

class TMidiMessageSubscriber {
public:
	virtual void messageReceived(TMidiMessage msg) = 0;
	virtual int midiChannel() = 0;
};

using TMidiMessageQueue = std::vector<TMidiMessage>;
using TMidiMessageSubscriberList = std::vector<TMidiMessageSubscriber*>;

#define MIDI_CHANNEL_ALL -1
class TMessageBus {
public:
	static void broadcast(int channel, TMidiCommand command, TByte param0, TByte param1);
	static void broadcast(int channel, TMidiCommand command, TShort param);
	static void subscribe(TMidiMessageSubscriber* sub);
	static void process();
private:
	static TMidiMessageQueue messageQueue;
	static TMidiMessageSubscriberList subscribers;
};

#endif // T_MIDI_H