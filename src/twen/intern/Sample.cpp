#include "Sample.h"

#include "Log.h"
#include "sndfile.hh"

Sample::Sample(const std::string& fileName) {
	SndfileHandle snd = SndfileHandle(fileName);
	int maxSecs = 15;
	if (snd.samplerate() > 44100) {
		maxSecs = 10;
	}
	if (snd.frames() < snd.samplerate() * maxSecs) {
		m_sampleRate = snd.samplerate();

		std::vector<float> samplesRaw;
		samplesRaw.resize(snd.frames() * snd.channels());
		snd.readf(samplesRaw.data(), samplesRaw.size());

		m_sampleData.resize(snd.frames());
		for (int i = 0; i < snd.frames(); i++) {
			m_sampleData[i] = samplesRaw[0 + i * 2] * 0.5f;
			m_sampleData[i] += samplesRaw[1 + i * 2] * 0.5f;
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
