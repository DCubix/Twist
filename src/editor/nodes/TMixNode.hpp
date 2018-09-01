#ifndef T_MIX_NODE_H
#define T_MIX_NODE_H

#include "TNode.h"

class TMixNode : public TNode {
public:
	TMixNode(float fac = 0.5f) : TNode("Mix", 140, 90), factor(fac) {
		addInput("A");
		addInput("B");
		addInput("Fac");
		addOutput("Out");
	}

	void gui() {
		ImGui::Knob("Factor", &factor, 0.0f, 1.0f);
	}

	void solve() {
		float a = getInput(0);
		float b = getInput(1);
		float fac = getInputOr(2, factor);
		setOutput(0, tmath::lerp(a, b, fac));
	}

	void save(JSON& json) {
		TNode::save(json);
		json["type"] = type();
		json["factor"] = factor;
	}

	float factor;

	static std::string type() { return "Mix"; }
};

#endif // T_MIX_NODE_H
