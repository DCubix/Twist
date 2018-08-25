#ifndef T_OSCILLATOR_NODE_H
#define T_OSCILLATOR_NODE_H

#include "TNode.h"
#include "../../TGen.h"

class TOscillatorNode : public TNode {
public:
	TOscillatorNode(float sampleRate, TOsc::TWave wf=TOsc::Sine, float freq=440.0f, float amp=1.0f)
		: TNode("Oscillator", 140, 130),
			m_osc(new TOsc(sampleRate)),
			value(0.0f), wf(wf),
			frequency(freq), amplitude(amp)
	{
		addInput("Freq");
		addInput("Amp");
		addOutput("Out");
	}

	~TOscillatorNode() {
		delete m_osc;
		m_osc = nullptr;
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
		float freq = getInputOr(0, frequency);
		float amp = getInputOr(1, amplitude);

		m_osc->amplitude(amp);
		m_osc->frequency(freq);
		m_osc->waveForm((TOsc::TWave) wf);

		value = m_osc->sample(freq);

		setOutput(0, value);
	}

	void save(JSON& json) {
		TNode::save(json);
		json["type"] = type();
		json["sampleRate"] = m_osc->sampleRate();
		json["waveForm"] = wf;
		json["freq"] = frequency;
		json["amp"] = amplitude;
	}

	float value, frequency, amplitude;
	int wf;

	static std::string type() { return "Oscillator"; }

private:
	TOsc* m_osc;
};

#endif // T_OSCILLATOR_NODE_H
