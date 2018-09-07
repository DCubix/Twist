#ifndef TWEN_OUT_NODE_H
#define TWEN_OUT_NODE_H

#include "../Node.h"

class OutNode : public Node {
	TWEN_NODE(OutNode)
public:
	OutNode() : Node() {
		addInput("In");
		addInput("Vol.");
	}

	void solve() {
		setInput("In", getInput("In") * getInput("Vol."));
	}

};

#endif // TWEN_OUT_NODE_H