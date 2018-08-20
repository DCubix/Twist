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
		if (inputs()[0].connected) {
			lvl = std::max(lvl, getInput(0) * 0.5f + 0.5f);
		}

		setInput(0, getInput(0) * volume);

		lvl -= 1.0f / 22050;
		lvl = std::min(std::max(lvl, 0.0f), 1.0f);
	}

	void save(JSON& json) {
		TNode::save(json);
		json["type"] = type();
		json["volume"] = volume;
	}

	float volume = 1.0f;
	float lvl = 0.0f;

	static std::string type() { return "Out"; }
};

#endif // T_OUT_NODE_H