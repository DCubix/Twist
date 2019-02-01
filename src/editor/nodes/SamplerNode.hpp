#ifndef TWIST_SAMPLER_HPP
#define TWIST_SAMPLER_HPP

#define IMGUI_INCLUDE_IMGUI_USER_H
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"

#include "twen/nodes/SamplerNode.hpp"

static void Sampler_gui(Node* node) {
	SamplerNode *n = dynamic_cast<SamplerNode*>(node);

	Vec<Str> samples = n->graph()->getSampleNames();

	ImGui::PushItemWidth(90);

	static int sel = 0;
	if (ImGui::Combo(
			"Sample",
			&sel,
			[](void* vec, int idx, const char** out_text){
				Vec<Str>* vector = reinterpret_cast<Vec<Str>*>(vec);
				if (idx < 0 || idx >= vector->size()) return false;
				*out_text = vector->at(idx).c_str();
				return true;
			},
			(void*)&samples,
			samples.size())) {
		n->sampleID = n->graph()->getSampleID(samples[sel]);
		n->load();
	}

	if (!n->sampleData.valid() && !samples.empty()) {
		n->sampleID = n->graph()->getSampleID(samples[n->sampleID]);
		n->load();
	}

	ImGui::AudioView(
				"_sample",
				120,
				n->sampleData.sampleData().data(),
				n->sampleData.sampleData().size(),
				n->sampleData.frame(),
				25.0f
	);
	ImGui::PopItemWidth();
}

#endif // TWIST_SAMPLER_HPP
