#ifndef TWEN_MIX_NODE_H
#define TWEN_MIX_NODE_H

#include "../Node.h"

class MixNode : public Node {
	TWEN_NODE(MixNode)
public:
	MixNode() : Node() {
		addInput("A");
		addInput("B");
		addInput("Fac");
		addOutput("Out");
	}

	void solve() {
		float a = getInput("A");
		float b = getInput("B");
		float fac = getInput("Fac");
		setOutput(0, Utils::lerp(a, b, fac));
	}

};

#endif // TWEN_MIX_NODE_H
