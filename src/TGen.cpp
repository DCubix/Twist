#include "TGen.h"

#include <cmath>
#include "sndfile.hh"

constexpr float PI2 = M_PI * 2.0f;

namespace tgen {
	float note(int index) {
		return note(index % 12, octave(index));
	}

	float note(int index, int octave) {
		return NOTE[index] * std::pow(2, octave);
	}

	int octave(int note) {
		return note / 12;
	}
}

float TOsc::sample() {
	return sample(m_frequency) * 0.5f + 0.5f;
}

float TOsc::sample(float freq) {
	float p = m_phase;

	float v = 0.0f;
	switch (m_waveform) {
		case TOsc::Sine: v = std::sin(p); break;
		case TOsc::Pulse: v = p < M_PI*0.25f ? 1.0f : -1.0f; break;
		case TOsc::Square: v = p < M_PI ? 1.0f : -1.0f; break;
		case TOsc::Saw: v = 1.0f - (1.0f / M_PI * p); break;
		case TOsc::Triangle: {
			if (p < M_PI) {
				v = -1.0f + (2.0f / M_PI) * p;
			} else {
				v = 3.0f - (2.0f / M_PI) * p;
			}
		} break;
		case TOsc::Noise: v = (float(rand()) / RAND_MAX) * 2.0f - 1.0f; break;
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

/// ADSR
float TADSR::sample() {
	switch (m_state) {
		case Idle: break;
		case Attack: {
			m_out = m_attackBase + m_out * m_attackCoef;
			if (m_out >= 1.0f) {
				m_out = 1.0f;
				m_state = Decay;
			}
		} break;
		case Decay: {
			m_out = m_decayBase + m_out * m_decayCoef;
			if (m_out <= m_sustain) {
				m_out = m_sustain;
				m_state = Sustain;
			}
		} break;
		case Sustain: break;
		case Release: {
			m_out = m_releaseBase + m_out * m_releaseCoef;
			if (m_out <= 0.0f) {
				m_out = 0.0f;
				m_state = Idle;
			}
		} break;
	}
	m_out = std::max(std::min(m_out, 1.0f), 0.0f);
	return m_out;
}

void TADSR::gate(bool g) {
	if (g) {
		m_state = Attack;
	} else if (m_state != Idle) {
		m_state = Release;
	}
}

float TADSR::coef(float rate, float targetRatio) {
	return (rate <= 0) ? 0.0 : std::exp(-std::log((1.0 + targetRatio) / targetRatio) / rate);
}

TWaveGuide::TWaveGuide(float sr) {
	sampleRate = sr;
	clear();
}

void TWaveGuide::clear() {
	m_counter = 0;
	for (int i = 0; i < T_WAVE_GUIDE_SAMPLES; i++) m_buffer[i] = 0;
}

float TWaveGuide::sample(float in, float feedBack, float delay) {
	float delayS = float(delay) / 1000.0f;
	int delaySmp = int(delayS * sampleRate);

	// calculate delay offset
	int back = m_counter - delaySmp;

	// clip lookback buffer-bound
	if (back < 0) {
		back = T_WAVE_GUIDE_SAMPLES + back;
	}

	// compute interpolation left-floor
	int index0 = back;

	// compute interpolation right-floor
	int index_1 = index0 - 1;
	int index1 = index0 + 1;
	int index2 = index0 + 2;

	// clip interp. buffer-bound
	if (index_1 < 0) index_1 = T_WAVE_GUIDE_SAMPLES - 1;
	if (index1 >= T_WAVE_GUIDE_SAMPLES) index1 = 0;
	if (index2 >= T_WAVE_GUIDE_SAMPLES) index2 = 0;

	// get neighbour samples
	float y_1 = m_buffer[index_1];
	float y0 = m_buffer[index0];
	float y1 = m_buffer[index1];
	float y2 = m_buffer[index2];

	// compute interpolation x
	const float x = float(back) - float(index0);

	// calculate
	float c0 = y0;
	float c1 = 0.5f * (y1 - y_1);
	float c2 = y_1 - 2.5f * y0 + 2.0f * y1 - 0.5f * y2;
	float c3 = 0.5f * (y2 - y_1) + 1.5f * (y0 - y1);

	float output = ((c3 * x + c2) * x + c1) * x + c0;

	// add to delay buffer
	m_buffer[m_counter++] = in + output * feedBack;

	// clip delay counter
	if (m_counter >= T_WAVE_GUIDE_SAMPLES) m_counter = 0;

	// return output
	return output;
}

TSoundSample::TSoundSample(const std::string& fileName) {
	SndfileHandle snd = SndfileHandle(fileName);
	int maxSecs = 10;
	if (snd.samplerate() > 44100) {
		maxSecs = 5;
	}
	if (snd.frames() < snd.samplerate() * maxSecs) {
		m_sampleRate = snd.samplerate();
		m_duration = float(double(snd.frames()) / m_sampleRate);

		std::vector<float> samplesRaw;
		samplesRaw.resize(snd.frames() * snd.channels());

		snd.readf(samplesRaw.data(), samplesRaw.size());

		m_sampleData.resize(snd.frames());
		for (int i = 0; i < snd.frames(); i++) {
			m_sampleData[i] = samplesRaw[i];
		}

		m_currTime = 0;
	}
}

void TSoundSample::gate(bool g) {
	if (g) {
		m_state = TADSR::Attack;
	} else if (m_state != TADSR::Idle) {
		m_state = TADSR::Decay;
	}
}

float TSoundSample::sample() {
	float out = 0.0f;
	switch (m_state) {
		case TADSR::Idle: break;
		case TADSR::Attack: {
			if (m_currTime < m_duration) {
				int sp = int(m_currTime * m_sampleRate);
				out = m_sampleData[sp];
			}

			m_currTime += 1.0f / m_sampleRate;
			if (m_currTime >= m_duration) {
				m_state = TADSR::Decay;
			}
		} break;
		case TADSR::Decay: {
			m_currTime = 0;
			m_state = TADSR::Idle;
		} break;
		default: break;
	}
	return out;
}