#ifndef TWIST_SAMPLER_HPP
#define TWIST_SAMPLER_HPP

#define IMGUI_INCLUDE_IMGUI_USER_H
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"

#include "twen/nodes/SamplerNode.hpp"

static void Sampler_gui(Node* node) {
	SamplerNode *n = dynamic_cast<SamplerNode*>(node);
	ImGui::PushID(n);

	Vec<Str> samples = n->graph()->getSampleNames();

	ImGui::PushItemWidth(90);

	if (ImGui::Combo(
			"Sample",
			(int*) &n->sampleID,
			[](void* vec, int idx, const char** out_text){
				Vec<Str>* vector = reinterpret_cast<Vec<Str>*>(vec);
				if (idx < 0 || idx >= vector->size()) return false;
				*out_text = vector->at(idx).c_str();
				return true;
			},
			(void*)&samples,
			samples.size()))
	{
		n->sampleName = samples[n->sampleID];
		n->load();
	}

	ImGui::AudioView(
				"_sample",
				130,
				n->sampleData.sampleData().data(),
				n->sampleData.sampleData().size(),
				n->sampleData.frame(),
				50.0f
	);
	ImGui::PopItemWidth();

	ImGui::PopID();
}

#endif // TWIST_SAMPLER_HPP
