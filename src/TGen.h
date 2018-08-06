#ifndef T_GEN_H
#define T_GEN_H

#include <vector>
#include <memory>
#include <cmath>

#define NOTE_RATIO 1.059463094f
#define NOTE(x) (261.63f * std::pow(NOTE_RATIO, x))

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
	B
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

	TOsc() : m_modulator(nullptr) {}
	TOsc(float sampleRate)
	 : m_modulator(nullptr), 
		m_sampleRate(sampleRate),
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

	void modulator(TSampler* s) {
		m_modulator = s;
	}

private:
	float m_sampleRate, m_phase, m_amplitude, m_frequency, m_noise;
	TWave m_waveform;

	TSampler* m_modulator;

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

#define T_WAVE_GUIDE_SAMPLES 22050
class TWaveGuide {
public:
	TWaveGuide();

	void clear();
	float sample(float in, float feedBack, double delay);
private:
	float m_buffer[T_WAVE_GUIDE_SAMPLES];
	int m_counter;
};

#endif // T_GEN_H