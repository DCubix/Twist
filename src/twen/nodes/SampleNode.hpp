#ifndef TWEN_SAMPLE_NODE_H
#define TWEN_SAMPLE_NODE_H

#include "../Node.h"
#include "../NodeGraph.h"
#include "../intern/Sample.h"

class SampleNode : public Node {
	TWEN_NODE(SampleNode, "Sample")
public:
	SampleNode(int sampleID=0) : Node() {
		addInput("Gate");
		addInput("Amp");
		addOutput("Out");
		setup();

		addParam("Sample", {}, sampleID);
		addParam("Amp", 0.0f, 1.0f, 1.0f, 0.05f, NodeParam::KnobRange);
	}

	void setup() {
		u32 id = paramOption("Sample");
		RawSample* sle = parent()->getSample(id);
		if (sle != nullptr) {
			sample = Sample(sle->data, sle->sampleRate, sle->duration);
		}
	}

	void solve() {
		params()["Sample"].options = parent()->getSampleNames();

		if (sample.valid()) {
			bool gate = in("Gate") > 0.5f ? true : false;
			sample.gate(gate);
			out("Out") = sample.sample() * in("Amp", "Amp");
		} else {
			setup();
		}
	}

private:
	Sample sample;
};

#endif // TWEN_SAMPLE_NODE_H