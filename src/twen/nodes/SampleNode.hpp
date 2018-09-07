#ifndef TWEN_SAMPLE_NODE_H
#define TWEN_SAMPLE_NODE_H

#include "../Node.h"
#include "../NodeGraph.h"
#include "../intern/Sample.h"

class SampleNode : public Node {
	TWEN_NODE(SampleNode)
public:
	SampleNode(int sampleID, int sel, float amp)
		: Node(), sampleID(sampleID), selectedID(sel), volume(amp)
	{
		addInput("Gate");
		addInput("Amp");
		addOutput("Out");
	}

	void setup() {
		RawSample* sle = parent()->getSample(sampleID);
		if (sle != nullptr) {
			sample = Sample(sle->data, sle->sampleRate, sle->duration);
		}
	}

	void solve() {
		bool gate = getInput("Gate") > 0.5f ? true : false;
		if (sample.valid()) {
			sample.gate(gate);
			setOutput("Out", sample.sample() * getInput("Amp"));
		}
	}

	int sampleID, selectedID;
	Sample sample;
};

#endif // TWEN_SAMPLE_NODE_H