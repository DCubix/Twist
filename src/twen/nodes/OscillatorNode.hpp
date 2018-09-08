#ifndef TWEN_OSCILLATOR_NODE_H
#define TWEN_OSCILLATOR_NODE_H

#include "../Node.h"
#include "../intern/Oscillator.h"

class OscillatorNode : public Node {
	TWEN_NODE(OscillatorNode, "Oscillator")
public:
	OscillatorNode(float sampleRate=44100.0f, u32 wf=0, float freq=220, float amp=1)
		: Node(), m_sampleRate(sampleRate)
	{
		addInput("Freq");
		addInput("Amp");
		addOutput("Out");

		addParam("WaveForm", { "Sine", "Pulse", "Square", "Saw", "Triangle", "Noise" }, wf);
		addParam("Freq", 20.0f, 20000.0f, freq, 1.0f, NodeParam::DragRange);
		addParam("Amp", 0.0f, 1.0f, amp, 0.05f, NodeParam::DragRange);

		for (int i = 0; i < FLOAT_ARRAY_MAX; i++) {
			m_osc[i] = Oscillator(m_sampleRate);
		}
	}

	void solve() {
		FloatArray freqs = ins("Freq", "Freq");
		FloatArray amps = ins("Amp", "Amp");

		int wf = (int) paramOption("WaveForm");

		int count = 0;
		float value = 0.0f;
		for (int i = 0; i < FLOAT_ARRAY_MAX; i++) {
			if (std::abs(freqs[i]) > 0.0f) {
				m_osc[i].waveForm((Oscillator::WaveForm) wf);
				m_osc[i].amplitude(amps[i]);
				value += m_osc[i].sample(freqs[i]);
				count++;
			}
		}
		value /= (count == 0 ? 1 : count);

		out("Out") = value;
	}

private:
	float m_sampleRate;
	Oscillator m_osc[FLOAT_ARRAY_MAX];
};

#endif // TWEN_OSCILLATOR_NODE_H
