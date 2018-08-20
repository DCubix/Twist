#ifndef T_OUT_NODE_H
#define T_OUT_NODE_H

#include <iostream>

#include "TNode.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include "imgui_user.h"

class TOutNode : public TNode {
public:
	TOutNode() : TNode("Output", 140, 140), volume(1.0f) {
		addInput("In");
	}

	void gui() {
		float height = ImGui::VUMeter("##wf", lvl);
		ImGui::SameLine();
		ImGui::VSliderFloat("##volume", ImVec2(16, height), &volume, 0.0f, 1.0f);
	}

	void solve() {
		lvl = tmath::lerp(lvl, -1.0f, 1.0f / 44010);
		buffer[bufPointer++ % 1024] = getInput(0);

		for (int i = 0; i < 1024; i++) {
			lvl = std::max(lvl, buffer[i]);
		}

		setInput(0, getInput(0) * volume);
	}

	void save(JSON& json) {
		TNode::save(json);
		json["type"] = type();
		json["volume"] = volume;
	}

	float volume = 1.0f;
	float lvl = 0.0f;
	float buffer[1024];
	int bufPointer = 0;

	static std::string type() { return "Out"; }
};

#endif // T_OUT_NODE_H