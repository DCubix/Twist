#ifndef TWEN_OUT_NODE_H
#define TWEN_OUT_NODE_H

#include "../Node.h"

class OutNode : public Node {
	TWEN_NODE(OutNode, "Output")
public:
	OutNode() : Node() {
		addInput("In");
		addInput("Vol.");
		ins("Vol.").set(1.0f);
	}

	void solve() {
		in("In") = in("In") * in("Vol.");
	}

};

#endif // TWEN_OUT_NODE_H