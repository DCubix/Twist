#ifndef T_MATH_NODE_H
#define T_MATH_NODE_H

#include "TNode.h"

class TMathNode : public TNode {
public:
	enum TMathNodeOp {
		Add = 0,
		Sub,
		Mul,
		Neg,
		Average,
		OpCount
	};

	TMathNode(TMathNodeOp op, float a, float b)
		: TNode("Math", 120, 90), op(op), aValue(a), bValue(b)
	{
		addOutput("Out");
		addInput("In 0");
		addInput("In 1");
	}

	void gui() {
		static const char* OPS[] = {
			"Add\0",
			"Subtract\0",
			"Multiply\0",
			"Negate",
			"Average"
		};
		ImGui::PushItemWidth(80);
		ImGui::Combo("Op", (int*)&op, OPS, OpCount, -1);
		ImGui::DragFloat("A", &aValue, 0.01f);
		ImGui::DragFloat("B", &bValue, 0.01f);
		ImGui::PopItemWidth();
	}

	void solve() {
		float a = getInputOr(0, aValue);
		float b = getInputOr(1, bValue);
		float out = 0.0f;
		switch (op) {
			case Add: out = a + b; break;
			case Sub: out = a - b; break;
			case Mul: out = a * b; break;
			case Neg: out = -a; break;
			case Average: out = (a + b) * 0.5f; break;
		}
		setOutput(0, out);
	}

	void save(JSON& json) {
		TNode::save(json);
		json["type"] = type();
		json["op"] = (int)op;
		json["a"] = aValue;
		json["b"] = bValue;
	}

	TMathNodeOp op;
	float aValue = 0.0f, bValue = 0.0f;

	static std::string type() { return "Math"; }
};

#endif // T_MATH_NODE_H
