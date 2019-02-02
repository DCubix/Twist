#ifndef TWEN_VALUE_NODE_H
#define TWEN_VALUE_NODE_H

#include "../NodeGraph.h"

class ValueNode : public Node {
	TWEN_NODE(ValueNode, "Value")
public:
	ValueNode(float v) : Node(), value(v) {}

	Value sample(NodeGraph *graph) override {
		return Value(value);
	}

	void save(JSON& json) override {
		Node::save(json);
		json["value"] = value;
	}

	void load(JSON json) override {
		Node::load(json);
		value = json["value"];
	}

	float value;
};

#endif // TWEN_VALUE_NODE_H
