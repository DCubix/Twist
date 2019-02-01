#ifndef TWEN_OSCILLATOR_NODE_H
#define TWEN_OSCILLATOR_NODE_H

#include "../NodeGraph.h"
#include <cmath>

class Phase {
public:
	Phase(float period = 0.0f) {
		m_phase = 0.0f;
		m_period = period;
	}

	float advance(float freq, float sampleRate) {
		m_phase += freq * (PI * 2.0f / sampleRate);
		m_phase = std::fmod(m_phase, m_period);
		return m_phase;
	}

private:
	float m_phase, m_period;
};

class OscillatorNode : public Node {
	TWEN_NODE(OscillatorNode, "Oscillator")
public:
	enum WaveForm {
		Sine = 0,
		Square,
		Saw,
		Triangle,
		Noise
	};

	OscillatorNode(float freq = 220, WaveForm wf = WaveForm::Sine)
		: Node(), frequency(freq), waveForm(wf), m_lastNoise(0.0f), m_phase(Phase(PI2))
	{
		addInput("Mod"); // Frequency Modulator
		addInput("Freq"); // Frequency
	}

	float sample(NodeGraph *graph) override {
		float freqMod = connected(0) ? get(0) : 0.0f;
		float freqVal = connected(1) ? get(1) : frequency;
		float freq = m_phase.advance(freqVal, graph->sampleRate()) + freqMod;
		float nfreq = freq / PI2;

		switch (waveForm) {
			case Sine: return std::sin(freq);
			case Square: return (std::sin(freq) > 0.0f ? 1.0f : -1.0f);
			case Saw: return (std::fmod(nfreq, 1.0f) * 2.0f - 1.0f);
			case Triangle: return (std::asin(std::cos(freq)) / 1.5708f);
			case Noise:
				float dt = std::fmod(freq, frequency);
				if (dt <= PI) {
					m_lastNoise = (float(rand() % RAND_MAX) / float(RAND_MAX)) * 2.0f - 1.0f;
				}
				return m_lastNoise * 0.5f;
		}
		return 0.0f;
	}

	void save(JSON& json) override {
		Node::save(json);
		json["frequency"] = frequency;
		json["waveForm"] = int(waveForm);
	}

	void load(JSON json) override {
		Node::load(json);
		frequency = json["frequency"];
		waveForm = WaveForm(json["waveForm"].get<int>());
	}

	float frequency;
	WaveForm waveForm;

private:
	float m_lastNoise;
	Phase m_phase;
};

#endif // TWEN_OSCILLATOR_NODE_H
