#include "TMidi.h"

#include <stdio.h>
#include <cassert>

TMidiMessageQueue TMessageBus::messageQueue;
TMidiMessageSubscriberList TMessageBus::subscribers;

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

TRawMidiMessage TMidiMessage::raw() {
	TRawMidiMessage ret;
	ret[0] = (int(command) & 0xF) << 4 | (channel & 0xF);
	ret[1] = param0 & 0x7F;
	ret[2] = param1 & 0x7F;
	return ret;
}

void TMessageBus::subscribe(TMidiMessageSubscriber* sub) {
	assert(sub != nullptr && "Subscriber shouldn't be null!");
	subscribers.push_back(sub);
}

void TMessageBus::broadcast(int channel, TMidiCommand command, TByte param0, TByte param1) {
	TMidiMessage msg;
	msg.channel = channel;
	msg.command = command;
	msg.param0 = param0;
	msg.param1 = param1;
	messageQueue.push_back(msg);
}

void TMessageBus::broadcast(int channel, TMidiCommand command, TShort param) {
	TByte lsb = param & 0x007F;
	TByte msb = (param >> 8) & 0x7F;
	broadcast(channel, command, lsb, msb);
}

void TMessageBus::process() {
	while (!messageQueue.empty()) {
		TMidiMessage msg = messageQueue.front();
		for (TMidiMessageSubscriber* sub : subscribers) {
			if (sub->midiChannel() == msg.channel || sub->midiChannel() == MIDI_CHANNEL_ALL) {
				sub->messageReceived(msg);
			}
		}
		messageQueue.erase(messageQueue.begin());
	}
}