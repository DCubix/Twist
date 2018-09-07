#ifndef TWEN_REMAP_NODE_H
#define TWEN_REMAP_NODE_H

#include "../Node.h"

class RemapNode : public Node {
	TWEN_NODE(RemapNode)
public:
	RemapNode(float om, float oma, float nm, float nma)
		: Node(),
		oldMin(om), oldMax(oma),
		newMin(nm), newMax(nma)
	{
		addInput("In");
		addOutput("Out");
	}

	void solve() {
		float v = getInput("In");
		float norm = (v - oldMin) / (oldMax - oldMin);
		setOutput("Out", norm * (newMax - newMin) + newMin);
	}

	float oldMin, oldMax, newMin, newMax;

};

#endif // TWEN_REMAP_NODE_H
