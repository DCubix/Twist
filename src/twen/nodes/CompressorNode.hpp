#ifndef TWEN_COMPRESSOR_H
#define TWEN_COMPRESSOR_H

#include "../Node.h"
extern "C" {
	#include "compressor.h"
}

class CompressorNode : public Node {
	TWEN_NODE(CompressorNode, "Compressor")
public:
	CompressorNode(
		float sampleRate=44100.0f,
		float preGain=0.0f,
		float threshold=-24,
		float knee=30,
		float ratio=12,
		float attack=0.003f,
		float release=0.25f
	) : m_sampleRate(sampleRate)
	{
		addInput("In");
		addOutput("Out");

		addParam("Pre-Gain", 0.0f, 100.0f, preGain, 0.1f, NodeParam::DragRange, false, 80);
		addParam("Threshold", -100.0f, 0.0f, threshold, 0.1f, NodeParam::DragRange, false, 80);
		addParam("Knee", 0.0f, 40.0f, knee, 0.1f, NodeParam::DragRange, false, 80);
		addParam("Ratio", 1.0f, 20.0f, ratio, 0.1f, NodeParam::DragRange, false, 80);
		addParam("Attack", 0.0f, 1.0f, attack, 0.01f, NodeParam::DragRange, false, 80);
		addParam("Release", 0.0f, 1.0f, release, 0.01f, NodeParam::DragRange, false, 80);

		m_pregain = preGain;
		m_threshold = threshold;
		m_knee = knee;
		m_ratio = ratio;
		m_attack = attack;
		m_release = release;

		sf_simplecomp(&m_comp, int(sampleRate), preGain, threshold, knee, ratio, attack, release);
	}

	void solve() {
		float _in = in(0);
		sf_sample_st ins, outs;
		ins.L = outs.L = _in;
		ins.R = outs.R = _in;

		float pregain = param(0);
		float threshold = param(1);
		float knee = param(2);
		float ratio = param(3);
		float attack = param(4);
		float release = param(5);

		bool change = false;
		change |= m_pregain != pregain;
		change |= m_threshold != threshold;
		change |= m_knee != knee;
		change |= m_ratio != ratio;
		change |= m_attack != attack;
		change |= m_release != release;

		if (change) {
			sf_simplecomp(&m_comp, int(m_sampleRate), pregain, threshold, knee, ratio, attack, release);
			m_pregain = pregain;
			m_threshold = threshold;
			m_knee = knee;
			m_ratio = ratio;
			m_attack = attack;
			m_release = release;
		}

		sf_compressor_process(&m_comp, 1, &ins, &outs);

		out(0) = (outs.L + outs.R) * 0.5f;
	}

private:
	float m_sampleRate;
	float m_pregain;
	float m_threshold;
	float m_knee;
	float m_ratio;
	float m_attack;
	float m_release;
	sf_compressor_state_st m_comp;
};

#endif // TWEN_COMPRESSOR_H