#ifndef TWEN_VOICE_H
#define TWEN_VOICE_H

#include "Utils.h"
#include "Log.h"

struct Voice {
	void trigger() { triggered = true; }
	virtual void reset() { triggered = false; }

	virtual float sample() { return 0.0f; }

	bool triggered = false;
};

template <class V, u32 S>
class VoiceManager {
public:
	VoiceManager() {}

	void noteOn(u8 note) {
		V* voice = findAvailableVoice();
		if (!voice)
			return;
		voice->reset();
		voice->trigger();
		onTrigger(voice, note);
	}

	void noteOff(u8 note) {
		for (int i = 0; i < S; i++) {
			V* voice = &m_voices[i];
			onRelease(voice, note);
		}
	}

	virtual void onTrigger(V* voice, u8 note) = 0;
	virtual void onRelease(V* voice, u8 note) = 0;

	V& get(u32 index) { return m_voices[index]; }

	constexpr u32 size() const { return S; }

private:
	Arr<V, S> m_voices;

	V* findAvailableVoice() {
		V* freeVoice = nullptr;
		for (int i = 0; i < S; i++) {
			if (!m_voices[i].triggered) {
				freeVoice = &m_voices[i];
				freeVoice->reset();
				break;
			}
		}
		return freeVoice;
	}
};

#endif // TWEN_VOICE_H
