#ifndef TWEN_REMAP_NODE_H
#define TWEN_REMAP_NODE_H

#include "../Node.h"

class RemapNode : public Node {
	TWEN_NODE(RemapNode, "Remap")
public:
	RemapNode(float omin=0, float omax=1, float nmin=0, float nmax=1)
		: Node()
	{
		addInput("In");
		addOutput("Out");

		addParam("Old Min", omin, 0.05f, false, 90);
		addParam("Old Max", omax, 0.05f, false, 90);
		addParam("New Min", nmin, 0.05f, false, 90);
		addParam("New Max", nmax, 0.05f, false, 90);
	}

	void solve() {
		float v = in(0);
		float norm = (v - param(0)) / (param(1) - param(0));
		out(0) = norm * (param(3) - param(2)) + param(2);
	}

};

#endif // TWEN_REMAP_NODE_H
