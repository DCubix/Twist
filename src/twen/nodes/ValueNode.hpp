#ifndef TWEN_VALUE_NODE_H
#define TWEN_VALUE_NODE_H

#include "../Node.h"

class ValueNode : public Node {
	TWEN_NODE(ValueNode)
public:
	ValueNode(float v) : Node(), value(v) {
		addOutput("Out");
	}

	void solve() {
		outputs("Out").set(value);
	}

	float value;

};

#endif // TWEN_VALUE_NODE_H
