#ifndef TWEN_OSCILLATOR_NODE_H
#define TWEN_OSCILLATOR_NODE_H

#include "../Node.h"
#include "../intern/Oscillator.h"

class OscillatorNode : public Node {
	TWEN_NODE(OscillatorNode)
public:
	OscillatorNode(
		float sampleRate,
		Oscillator::WaveForm wf=Oscillator::Sine,
		float freq=220.0f,
		float amp=1.0f
	) : Node(), wf(wf), frequency(freq), amplitude(amp), sampleRate(sampleRate)
	{
		addInput("Freq");
		addInput("Amp");
		addOutput("Out");

		for (int i = 0; i < FLOAT_ARRAY_MAX; i++) {
			m_osc[i] = Oscillator(sampleRate);
		}
	}

	void solve() {
		FloatArray freqs = inputs("Freq");
		FloatArray amps = inputs("Amp");

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

		setOutput("Out", value);
	}

	float frequency, amplitude, sampleRate;
	int wf;

private:
	Oscillator m_osc[FLOAT_ARRAY_MAX];
};

#endif // TWEN_OSCILLATOR_NODE_H
