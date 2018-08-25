#ifndef T_SAMPLE_NODE_H
#define T_SAMPLE_NODE_H

#include <iostream>
#include <string>

#include "TNode.h"
#include "../TNodeGraph.h"
#include "../../TGen.h"

#include "../tinyfiledialogs.h"

class TSampleNode : public TNode {
public:
	TSampleNode(int sampleID, int sel, float amp)
		: TNode("Sample", 0, 0), sampleID(sampleID), selectedID(sel), volume(amp)
	{
		addInput("Gate");
		addInput("Amp");
		addOutput("Out");

	}

	void setup() {
		TSampleLibEntry* sle = parent()->getSample(sampleID);
		if (sle != nullptr) {
			sample = TSoundSample(sle->sampleData, sle->sampleRate, sle->duration);
		}
	}

	void gui() {
		auto slns = parent()->getSampleNames();
		if (ImGui::Combo("Sample", &selectedID, slns.data(), slns.size())) {
			sampleID = parent()->getSampleID(std::string(slns[selectedID]));
		}
		if (sample.valid()) {
			ImGui::Knob("Vol.", &volume, 0.0f, 1.0f);
			ImGui::SameLine();
			ImGui::AudioView(
				"##audio_view0",
				120,
				sample.sampleData().data(),
				sample.sampleData().size(),
				int(sample.time() * sample.sampleRate())
			);
		} else {
			TSampleLibEntry* sle = parent()->getSample(sampleID);
			if (sle != nullptr) {
				sample = TSoundSample(sle->sampleData, sle->sampleRate, sle->duration);
			}
		}
	}

	void solve() {
		bool gate = getInputOr(0, 0.0f) > 0.0f ? true : false;
		if (sample.valid()) {
			sample.gate(gate);
			setOutput(0, sample.sample() * getInputOr(1, volume));
		}
	}

	void save(JSON& json) {
		TNode::save(json);
		json["type"] = type();
		json["sample"] = sampleID;
		json["selectedID"] = selectedID;
		json["volume"] = volume;
	}

	static std::string type() { return "Sample"; }

	int sampleID, selectedID;
	float volume=1;
	TSoundSample sample;
};

#endif // T_SAMPLE_NODE_H