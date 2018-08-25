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
		addInput("Vol.");
	}

	void gui() {
		// ImGui::Text("LEVEL: %.3f", nextLvl);
		ImGui::VUMeter("##wf", lvl);
	}

	void solve() {
		nextLvl = std::max(nextLvl, getInput(0));
		volume = getInputOr(1, 1.0f);
		setInput(0, getInput(0) * volume);

		lvl = tmath::lerp(lvl, nextLvl, 0.0007f);
		nextLvl -= 1.0f / 22050;
		nextLvl = std::min(std::max(nextLvl, 0.0f), 1.0f);
	}

	void save(JSON& json) {
		TNode::save(json);
		json["type"] = type();
		json["volume"] = volume;
	}

	float volume = 1.0f;
	float lvl = 0.0f, nextLvl = 0.0f;

	static std::string type() { return "Out"; }
};

#endif // T_OUT_NODE_H