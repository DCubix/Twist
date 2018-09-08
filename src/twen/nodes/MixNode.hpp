#ifndef TWEN_MIX_NODE_H
#define TWEN_MIX_NODE_H

#include "../Node.h"

class MixNode : public Node {
	TWEN_NODE(MixNode, "Mix")
public:
	MixNode(float fac=0.5f) : Node() {
		addInput("A");
		addInput("B");
		addInput("Fac");
		addOutput("Out");

		addParam("Fac.", 0.0f, 1.0f, fac, 0.05f, NodeParam::KnobRange);
	}

	void solve() {
		float a = in("A");
		float b = in("B");
		float fac = in("Fac", "Fac.");
		out("Out") = Utils::lerp(a, b, fac);
	}

};

#endif // TWEN_MIX_NODE_H
