#include "Oscillator.h"

float Oscillator::sample() {
	return sample(m_frequency) * 0.5f + 0.5f;
}

float Oscillator::sample(float freq) {
	float p = m_phase;

	float v = 0.0f;
	switch (m_waveform) {
		case Sine: v = std::sin(p); break;
		case Pulse: v = p < PI * 0.25f ? 1.0f : -1.0f; break;
		case Square: v = p < PI ? 1.0f : -1.0f; break;
		case Saw: v = 1.0f - (1.0f / PI * p); break;
		case Triangle: {
			if (p < PI) {
				v = -1.0f + (2.0f / PI) * p;
			} else {
				v = 3.0f - (2.0f / PI) * p;
			}
		} break;
		case Noise: v = (float(rand()) / RAND_MAX) * 2.0f - 1.0f; break;
	}

	// Modulate
	float Mfreq = freq;
	float Mamp = m_amplitude;

	m_phase += (PI2 * Mfreq) / m_sampleRate;
	if (m_phase > PI2) {
		m_phase -= PI2;
	}

	return v * Mamp;
}