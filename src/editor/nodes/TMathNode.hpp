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
		addInput("A");
		addInput("B");
	}

	void gui() {
		static const char* OPS[] = {
			"Add\0",
			"Subtract\0",
			"Multiply\0",
			"Negate",
			"Average"
		};
		ImGui::PushItemWidth(70);
		ImGui::Combo("Op", (int*)&op, OPS, OpCount, -1);
		ImGui::DragFloat("A", &aValue, 0.01f);
		ImGui::DragFloat("B", &bValue, 0.01f);
		ImGui::PopItemWidth();
	}

	void solve() {
		// FloatArray a = getMultiInputValues(0, aValue);
		// FloatArray b = getMultiInputValues(1, bValue);
		// FloatArray out;
		// switch (op) {
		// 	case Add: out = SIMD::add(a, b); break;
		// 	case Sub: out = SIMD::sub(a, b); break;
		// 	case Mul: out = SIMD::mul(a, b); break;
		// 	case Neg: out = SIMD::neg(a); break;
		// 	case Average: out = SIMD::avg(a, b); break;
		// }
		// setMultiOutputValues(0, out);
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
