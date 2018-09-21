#ifndef TWEN_OSCILLATOR_H
#define TWEN_OSCILLATOR_H

#include "Utils.h"

class Oscillator {
public:
	enum WaveForm {
		Sine = 0,
		Pulse,
		Square,
		Saw,
		Triangle,
		Noise
	};

	Oscillator()
	 : m_sampleRate(44100),
		m_amplitude(0.2f), m_phase(0.0f),
		m_frequency(220.0f), m_waveform(Sine)
	{}

	Oscillator(float sampleRate)
	 : m_sampleRate(sampleRate),
		m_amplitude(0.2f), m_phase(0.0f),
		m_frequency(220.0f), m_waveform(Sine)
	{}

	float sample(float freq);
	float sample();

	WaveForm waveForm() const { return m_waveform; }
	void waveForm(WaveForm wave) { m_waveform = wave; }

	float amplitude() const { return m_amplitude; }
	void amplitude(float amp) { m_amplitude = amp; }

	float frequency() const { return m_frequency; }
	void frequency(float freq) { m_frequency = freq; }

	float sampleRate() const { return m_sampleRate; }
	void sampleRate(float sr) { m_sampleRate = sr; }

	void reset() { m_phase = 0; }

private:
	float m_sampleRate, m_phase, m_amplitude, m_frequency, m_noise;
	WaveForm m_waveform;
};

#endif // TWEN_OSCILLATOR_H