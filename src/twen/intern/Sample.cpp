#include "Sample.h"

#include "Log.h"
#include "sndfile.hh"

Sample::Sample(const std::string& fileName) {
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

void Sample::gate(bool g) {
	if (g) {
		m_state = Attack;
	} else if (m_state != Idle) {
		m_state = Decay;
	}
}

float Sample::sample() {
	float out = 0.0f;
	switch (m_state) {
		case Idle: break;
		case Attack: {
			if (m_currTime < m_duration) {
				int sp = int(m_currTime * m_sampleRate);
				out = m_sampleData[sp];
			}

			m_currTime += 1.0f / m_sampleRate;
			if (m_currTime >= m_duration) {
				m_state = Decay;
			}
		} break;
		case Decay: {
			m_currTime = 0;
			m_state = Idle;
		} break;
		default: break;
	}
	return out;
}