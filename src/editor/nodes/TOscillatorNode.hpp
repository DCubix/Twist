#ifndef T_OSCILLATOR_NODE_H
#define T_OSCILLATOR_NODE_H

#include "TNode.h"
#include "../../TGen.h"

class TOscillatorNode : public TNode {
public:
	TOscillatorNode(float sampleRate, TOsc::TWave wf=TOsc::Sine, float freq=440.0f, float amp=1.0f)
		: TNode("Oscillator", 140, 130), wf(wf), frequency(freq), amplitude(amp), sampleRate(sampleRate)
	{
		addInput("Freq");
		addInput("Amp");
		addOutput("Out");

		for (int i = 0; i < TNODE_MAX_SIMULTANEOUS_VALUES_PER_SLOT; i++) {
			m_osc[i] = TOsc(sampleRate);
		}

	}	

	void gui() {
		static const char* WAVES[] = {
			"Sine\0",
			"Pulse\0",
			"Square\0",
			"Saw\0",
			"Triangle\0",
			"Noise\0"
		};
		ImGui::Combo("WaveForm", (int*)&wf, WAVES, 6, -1);

		ImGui::DragFloat("Freq", &frequency, 0.1f, 0.0f, 20000.0f);
		ImGui::DragFloat("Amp", &amplitude, 0.1f, 0.0f, 9999.0f);
	}

	void solve() {
		TValueList freqs = getMultiInputValues(0, frequency);
		TValueList amps = getMultiInputValues(1, amplitude);

		int count = 0;
		float value = 0.0f;
		for (int i = 0; i < TNODE_MAX_SIMULTANEOUS_VALUES_PER_SLOT; i++) {
			if (std::abs(freqs[i]) > 0.0f) {
				m_osc[i].waveForm((TOsc::TWave) wf);
				m_osc[i].amplitude(amps[i]);
				value += m_osc[i].sample(freqs[i]);
				count++;
			}
		}
		value /= (count == 0 ? 1 : count);

		setOutput(0, value);
	}

	void save(JSON& json) {
		TNode::save(json);
		json["type"] = type();
		json["sampleRate"] = sampleRate;
		json["waveForm"] = wf;
		json["freq"] = frequency;
		json["amp"] = amplitude;
	}

	float frequency, amplitude, sampleRate;
	int wf;

	static std::string type() { return "Oscillator"; }

private:
	TOsc m_osc[TNODE_MAX_SIMULTANEOUS_VALUES_PER_SLOT];
};

#endif // T_OSCILLATOR_NODE_H
