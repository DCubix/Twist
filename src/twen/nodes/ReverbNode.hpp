#ifndef TWEN_REVERB_NODE_H
#define TWEN_REVERB_NODE_H

#include "../Node.h"

extern "C" {
	#include "reverb.h"
}

class ReverbNode : public Node {
	TWEN_NODE(ReverbNode, "Reverb")
public:
	ReverbNode(float sampleRate=44100.0f, int preset=SF_REVERB_PRESET_DEFAULT) : Node(), m_sampleRate(sampleRate) {
		addInput("In");
		addOutput("Out");

		addParam("Preset", {
			"Default",
			"Small Hall 1",
			"Small Hall 2",
			"Medium Hall 1",
			"Medium Hall 2",
			"Large Hall 1",
			"Large Hall 2",
			"Small Room 1",
			"Small Room 2",
			"Medium Room 1",
			"Medium Room 2",
			"Large Room 1",
			"Large Room 2",
			"Medium ER 1",
			"Medium ER 2",
			"Plate High",
			"Plate Low",
			"Long Reverb 1",
			"Long Reverb 2"
		});

		sf_presetreverb(&m_rev, (int)sampleRate, (sf_reverb_preset)preset);
	}

	void solve() {
		float _in = in(0);
		sf_sample_st ins, outs;
		ins.L = outs.L = _in;
		ins.R = outs.R = _in;

		u32 pset = paramOption(0);
		if (pset != m_preset) {
			sf_presetreverb(&m_rev, (int)m_sampleRate, (sf_reverb_preset)pset);
			m_preset = pset;
		}

		sf_reverb_process(&m_rev, 1, &ins, &outs);
		out(0) = (outs.L + outs.R) * 0.5f;
	}

private:
	sf_reverb_state_st m_rev;
	int m_preset;
	float m_sampleRate;
};

#endif //TWEN_REVERB_NODE_H
