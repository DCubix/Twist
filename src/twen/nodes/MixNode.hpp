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

	Value sample(NodeGraph *graph) override {
		float fac = connected(2) ? in(2).value() : factor;
		float a = in(0).value();
		float b = in(1).value();
		return Value(Utils::lerp(a, b, fac));
	}

	void save(JSON& json) override {
		Node::save(json);
		json["factor"] = factor;
	}

	void load(JSON json) override {
		Node::load(json);
		factor = json["factor"];
	}

	float factor;
};

#endif // TWEN_MIX_NODE_H
