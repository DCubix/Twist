#ifndef TWEN_REVERB_NODE_H
#define TWEN_REVERB_NODE_H

#include "../Node.h"

extern "C" {
	#include "reverb.h"
}

class ReverbNode : public Node {
	TWEN_NODE(ReverbNode)
public:
	ReverbNode(float sampleRate)
		: Node(), sampleRate(sampleRate)
	{
		loaded = true;
		addInput("In");
		addOutput("Out");

		sf_presetreverb(&m_rev, (int)sampleRate, SF_REVERB_PRESET_DEFAULT);
	}

	void solve() {
		float in = getInput("In");
		sf_sample_st ins, outs;
		ins.L = in;
		ins.R = in;

		sf_reverb_process(&m_rev, 1, &ins, &outs);
		setOutput("Out", (outs.L + outs.R) * 0.5f);
	}

	sf_reverb_preset pset;
	sf_reverb_state_st m_rev;
	int m_preset = 0;
	float sampleRate;
	bool loaded = false;

};

#endif //TWEN_REVERB_NODE_H