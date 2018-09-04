#ifndef T_GEN_H
#define T_GEN_H

#include <vector>
#include <memory>

#define _USE_MATH_DEFINES
#include <cmath>

static const float NOTE[] = {
	32.70320f,
	34.64783f,
	36.70810f,
	38.89087f,
	41.20344f,
	43.65353f,
	46.24930f,
	48.99943f,
	51.91309f,
	55.00000f,
	58.27047f,
	61.73541f
};

namespace tgen {
	float note(int index);
	float note(int index, int octave);
	int octave(int note);
}

enum Notes {
	C = 0,
	Cs,
	D,
	Ds,
	E,
	F,
	Fs,
	G,
	Gs,
	A,
	As,
	B,
	Count
};

class TSampler {
public:
	virtual float sample() = 0;
};

class TOsc : public TSampler {
public:
	enum TWave {
		Sine = 0,
		Pulse,
		Square,
		Saw,
		Triangle,
		Noise
	};

	TOsc() {}
	TOsc(float sampleRate)
	 : m_sampleRate(sampleRate),
		m_amplitude(0.2f),
		m_phase(0.0f),
		m_frequency(220.0f),
		m_waveform(Sine)
	{}
	float sample(float freq);
	float sample();

	TWave waveForm() const { return m_waveform; }
	void waveForm(TWave wave) { m_waveform = wave; }

	float amplitude() const { return m_amplitude; }
	void amplitude(float amp) { m_amplitude = amp; }

	float frequency() const { return m_frequency; }
	void frequency(float freq) { m_frequency = freq; }

	float sampleRate() const { return m_sampleRate; }

	void reset() { m_phase = 0; }

private:
	float m_sampleRate, m_phase, m_amplitude, m_frequency, m_noise;
	TWave m_waveform;
};

class TADSR : public TSampler{
public:
	enum TState {
		Idle = 0,
		Attack,
		Decay,
		Sustain,
		Release
	};

	TADSR()
		: m_attack(0.0f), m_decay(0.0f), m_sustain(1.0f), m_release(0.0f),
		m_targetRatioA(0.3f), m_targetRatioDR(0.0001f), m_out(0.0f)
	{}

	TADSR(float a, float d, float s, float r)
		: m_targetRatioA(0.3f), m_targetRatioDR(0.0001f), m_out(0.0f)
	{
		attack(a);
		decay(d);
		sustain(s);
		release(r);
	}

	float attack() const { return m_attack; }
	void attack(float v) {
		m_attack = v;
		m_attackCoef = coef(v, m_targetRatioA);
		m_attackBase = (1.0f + m_targetRatioA) * (1.0f - m_attackCoef);
	}

	float decay() const { return m_decay; }
	void decay(float v) {
		m_decay = v;
		m_decayCoef = coef(v, m_targetRatioDR);
		m_decayBase = (m_sustain - m_targetRatioDR) * (1.0f - m_decayCoef);
	}

	float release() const { return m_release; }
	void release(float v) {
		m_release = v;
		m_releaseCoef = coef(v, m_targetRatioDR);
		m_releaseBase = -m_targetRatioDR * (1.0f - m_releaseCoef);
	}

	float sustain() const { return m_sustain; }
	void sustain(float v) {
		m_sustain = v;
		m_decayBase = (m_sustain - m_targetRatioDR) * (1.0f - m_decayCoef);
	}

	float targetRatioA() const { return m_targetRatioA; }
	void targetRatioA(float v) {
		if (v < 0.000000001f)
			v = 0.000000001f;  // -180 dB
		m_targetRatioA = v;
		m_attackCoef = coef(m_attack, m_targetRatioA);
		m_attackBase = (1.0f + m_targetRatioA) * (1.0f - m_attackCoef);
	}

	float targetRatioDR() const { return m_targetRatioDR; }
	void targetRatioDR(float v) {
		if (v < 0.000000001f)
			v = 0.000000001f;  // -180 dB
		m_targetRatioDR = v;
		m_decayCoef = coef(m_decay, m_targetRatioDR);
		m_releaseCoef = coef(m_release, m_targetRatioDR);
		m_decayBase = (m_sustain - m_targetRatioDR) * (1.0f - m_decayCoef);
		m_releaseBase = -m_targetRatioDR * (1.0 - m_releaseCoef);
	}

	void gate(bool g);
	float sample();
private:
	TState m_state;
	float m_attack,
		m_decay,
		m_release,
		m_sustain,
		m_attackBase,
		m_decayBase,
		m_releaseBase,
		m_attackCoef,
		m_decayCoef,
		m_releaseCoef,
		m_targetRatioA,
		m_targetRatioDR,
		m_out;


	float coef(float rate, float targetRatio);

};

class TSoundSample : public TSampler {
public:
	TSoundSample() { }
	TSoundSample(const std::vector<float> data, float sr, float dur)
		: m_sampleData(data), m_sampleRate(sr), m_duration(dur), m_currTime(0), m_state(0)
	{ }
	TSoundSample(const std::string& fileName);

	void gate(bool g);
	bool valid() const { return !m_sampleData.empty() && m_duration > 0.0f; }
	void invalidate() { m_sampleData.clear(); m_duration = 0; }

	std::vector<float> sampleData() const { return m_sampleData; }
	float sampleRate() const { return m_sampleRate; }
	float duration() const { return m_duration; }
	float time() const { return m_currTime; }

	float sample();
protected:
	std::vector<float> m_sampleData;
	float m_duration = 0, m_currTime, m_sampleRate;
	int m_state;
};

#define T_WAVE_GUIDE_SAMPLES 44100
class TWaveGuide {
public:
	TWaveGuide() {}
	TWaveGuide(float sr);

	void clear();
	float sample(float in, float feedBack, float delay);
private:
	float sampleRate;
	float m_buffer[T_WAVE_GUIDE_SAMPLES];
	int m_counter;
};

#endif // T_GEN_H