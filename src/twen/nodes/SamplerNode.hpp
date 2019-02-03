#ifndef TWEN_SAMPLER_NODE_H
#define TWEN_SAMPLER_NODE_H

#include "../Node.h"
#include "../NodeGraph.h"
#include "../intern/Sample.h"

class SamplerNode : public Node {
	TWEN_NODE(SamplerNode, "Sampler")
public:
	SamplerNode(const Str& sampleName="")
		: Node(), sampleName(sampleName), sampleID(0)
	{
		addInput("Gate");
		sampleData.invalidate();
	}

	void load() {
		RawSample* sle = graph()->getSample(sampleName);
		if (sle != nullptr) {
			LogI("Loaded sample: ", sle->name);
			sampleData.invalidate();
			sampleData = Sample(sle->data, sle->sampleRate);
		}
	}

	Value sample(NodeGraph *graph) override {
		bool gate = connected(0) ? in(0).gate() : true;
		bool repeat = gate && !connected(0);
		if (sampleData.valid()) {
			sampleData.gate(gate);
		}
		return Value(sampleData.valid() ? sampleData.sample(graph->sampleRate(), repeat) : 0.0f);
	}

	void save(JSON& json) override {
		Node::save(json);
		json["sample"] = sampleName;
	}

	void load(JSON json) override {
		Node::load(json);
		sampleName = json["sample"];
		auto samples = graph()->getSampleNames();
		auto pos = std::find(samples.begin(), samples.end(), sampleName);
		if (pos != samples.end()) {
			sampleID = std::distance(samples.begin(), pos);
		}
		load();
	}

	Sample sampleData;
	Str sampleName;
	u32 sampleID;
};

#endif // TWEN_SAMPLER_NODE_H
