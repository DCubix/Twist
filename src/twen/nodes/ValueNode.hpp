#ifndef TWEN_VALUE_NODE_H
#define TWEN_VALUE_NODE_H

#include "../Node.h"

class ValueNode : public Node {
	TWEN_NODE(ValueNode, "Value")
public:
	ValueNode(float v) : Node() {
		addOutput("Out");

		addParam("Val.", v, 0.05f, false, 90);
	}

	void solve() {
		outs("Out").set(param("Val."));
	}

};

#endif // TWEN_VALUE_NODE_H
