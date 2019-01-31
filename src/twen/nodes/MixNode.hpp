#ifndef TWEN_MIX_NODE_H
#define TWEN_MIX_NODE_H

#include "../NodeGraph.h"

class MixNode : public Node {
	TWEN_NODE(MixNode, "Mix")
public:
	MixNode(float fac=0.5f) : Node(), factor(fac) {
		addInput("A"); // A
		addInput("B"); // B
		addInput("Fac"); // Fac
	}

	float sample(NodeGraph *graph) override {
		float fac = connected(2) ? get(2) : factor;
		float a = get(0);
		float b = get(1);
		return Utils::lerp(a, b, fac);
	}

	float factor;
};

#endif // TWEN_MIX_NODE_H
