#ifndef TWEN_WAVE_GUIDE_H
#define TWEN_WAVE_GUIDE_H

#include "Utils.h"

#define WAVE_GUIDE_SAMPLES 22050
class WaveGuide {
public:
	WaveGuide() : m_sampleRate(44100), m_counter(0) { m_buffer.fill(0); }
	WaveGuide(float sampleRate);

	void clear();
	float sample(float in, float feedBack, float delay);

	void sampleRate(float sr) { m_sampleRate = sr; }

private:
	float m_sampleRate;
	Arr<float, WAVE_GUIDE_SAMPLES> m_buffer;
	int m_counter;
};

#endif // TWEN_WAVE_GUIDE_H