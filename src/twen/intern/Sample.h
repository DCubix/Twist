#ifndef TWEN_SAMPLE_H
#define TWEN_SAMPLE_H

#include "Utils.h"

class Sample {
public:

	Sample() : m_frame(0), m_sampleRate(0.0f) { }
	Sample(const Vec<float> data, float sr)
		: m_sampleData(data), m_sampleRate(sr), m_frame(0)
	{ }
	Sample(const Str& fileName);

	bool valid() const { return !m_sampleData.empty() && m_sampleRate > 0.0f; }
	void invalidate() { m_sampleData.clear(); }

	Vec<float> sampleData() const { return m_sampleData; }
	float sampleRate() const { return m_sampleRate; }
	float frame() const { return m_frame; }

	float sampleDirect(float sampleRate);

protected:
	Vec<float> m_sampleData;
	float m_frame;
	float m_sampleRate;
};

#endif // TWEN_SAMPLE_H
