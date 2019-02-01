#ifndef TWEN_SAMPLER_NODE_H
#define TWEN_SAMPLER_NODE_H

#include "../Node.h"
#include "../NodeGraph.h"
#include "../intern/Sample.h"

class SamplerNode : public Node {
	TWEN_NODE(SamplerNode, "Sampler")
public:
	SamplerNode(int sampleID=0) : Node(), sampleID(sampleID) { sampleData.invalidate(); }

	void load() {
		RawSample* sle = graph()->getSample(sampleID);
		if (sle != nullptr) {
			LogI("Loaded sample: ", sle->name);
			sampleData.invalidate();
			sampleData = Sample(sle->data, sle->sampleRate);
		}
	}

	float sample(NodeGraph *graph) override {
		return sampleData.valid() ? sampleData.sampleDirect(graph->sampleRate()) : 0.0f;
	}

	void save(JSON& json) override {
		Node::save(json);

	}

	void load(JSON json) override {
		Node::load(json);

	}

	Sample sampleData;
	int sampleID;
};

#endif // TWEN_SAMPLER_NODE_H
