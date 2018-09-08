#ifndef TWEN_SAMPLE_H
#define TWEN_SAMPLE_H

#include "Utils.h"

class Sample {
public:
	enum State {
		Idle = 0,
		Attack,
		Decay
	};

	Sample() : m_duration(0.0f), m_sampleRate(0.0f) { }
	Sample(const Vec<float> data, float sr, float dur)
		: m_sampleData(data), m_sampleRate(sr),
		m_duration(dur), m_currTime(0), m_state(Idle)
	{ }
	Sample(const Str& fileName);

	void gate(bool g);
	bool valid() const { return !m_sampleData.empty() && m_duration > 0.0f && m_sampleRate > 0.0f; }
	void invalidate() { m_sampleData.clear(); m_duration = 0; }

	Vec<float> sampleData() const { return m_sampleData; }
	float sampleRate() const { return m_sampleRate; }
	float duration() const { return m_duration; }
	float time() const { return m_currTime; }

	float sample();

protected:
	Vec<float> m_sampleData;
	float m_duration = 0, m_currTime, m_sampleRate;
	State m_state;
};

#endif // TWEN_SAMPLE_H