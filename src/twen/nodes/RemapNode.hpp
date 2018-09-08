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

		addParam("Old Min", omin, 0.05f, false);
		addParam("Old Max", omax, 0.05f, false);
		addParam("New Min", nmin, 0.05f, false);
		addParam("New Max", nmax, 0.05f, false);
	}

	void solve() {
		float v = in("In");
		float norm = (v - param("Old Min")) / (param("Old Max") - param("Old Min"));
		out("Out") = norm * (param("New Max") - param("New Min")) + param("New Min");
	}

};

#endif // TWEN_REMAP_NODE_H
