#ifndef TWEN_VALUE_NODE_H
#define TWEN_VALUE_NODE_H

#include "../NodeGraph.h"

class ValueNode : public Node {
	TWEN_NODE(ValueNode, "Value")
public:
	ValueNode(float v) : Node(), value(v) {}

	float sample(NodeGraph *graph) override {
		return value;
	}

	float value;
};

#endif // TWEN_VALUE_NODE_H
