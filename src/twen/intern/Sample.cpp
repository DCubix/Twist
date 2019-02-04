#include "Sample.h"

#include "Log.h"
#include "TAudio.h"

Sample::Sample(const std::string& fileName) {
	TAudioFile snd(fileName);
	int maxSecs = 15;
	if (snd.sampleRate() > 44100) {
		maxSecs = 10;
	}
	if (snd.frames() < snd.sampleRate() * maxSecs) {
		m_sampleRate = snd.sampleRate();

		std::vector<float> samplesRaw;
		samplesRaw.resize(snd.frames() * snd.channels());
		u32 interleavedFrames = snd.readf(samplesRaw.data(), samplesRaw.size());

		m_sampleData.resize(interleavedFrames / snd.channels());
		std::fill(m_sampleData.begin(), m_sampleData.end(), 0.0f);

		u32 s = 0;
		for (int i = 0; i < interleavedFrames; i+=snd.channels()) {
			for (int c = 0; c < snd.channels(); c++) {
				m_sampleData[s] = samplesRaw[c + i * snd.channels()];
			}
			m_sampleData[s] /= snd.channels();
			s++;
		}
		m_frame = 0;
	}
}

float Sample::sampleDirect(float sampleRate) {
	float out = m_sampleData[int(m_frame)];
	float frameStep = m_sampleRate / sampleRate;

	if (m_frame >= m_sampleData.size()) {
		m_frame = 0.0f;
	} else {
		m_frame += frameStep;
	}

	return out;
}

void Sample::gate(bool g) {
	if (g) {
		m_state = Attack;
	} else if (m_state != Idle) {
		m_state = Decay;
	}
}

float Sample::sample(float sampleRate, bool repeat) {
	const float frameStep = m_sampleRate / sampleRate;

	float out = 0.0f;
	switch (m_state) {
		case Idle: break;
		case Attack: {
			if (m_frame < m_sampleData.size()) {
				out = m_sampleData[u32(m_frame)];
			}

			m_frame += frameStep;
			if (m_frame >= m_sampleData.size()) {
				if (!repeat) m_state = Decay;
				else m_frame = 0;
			}
		} break;
		case Decay: {
			m_frame = 0;
			m_state = Idle;
		} break;
		default: break;
	}
	return out;
}
