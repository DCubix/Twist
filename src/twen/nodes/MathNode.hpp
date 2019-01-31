#ifndef TWEN_MATH_NODE_H
#define TWEN_MATH_NODE_H

#include "../NodeGraph.h"

class MathNode : public Node {
	TWEN_NODE(MathNode, "Math")
public:
	enum MathOp {
		Add = 0,
		Sub,
		Mul,
		Neg,
		Average,
		OpCount
	};

	MathNode(MathOp op, float a=0, float b=0)
		: Node(), op(op), a(a), b(b)
	{
		addInput("A"); // A
		addInput("B"); // B
	}

	float sample(NodeGraph *graph) override {
		float _a = connected(0) ? get(0) : a;
		float _b = connected(1) ? get(1) : b;
		float _out;

		switch (op) {
			case Add: _out = a + b; break;
			case Sub: _out = a - b; break;
			case Mul: _out = a * b; break;
			case Neg: _out = -a; break;
			case Average: _out = (a + b) * 0.5f; break;
		}
		return _out;
	}

	MathOp op;
	float a, b;

};

#endif // TWEN_MATH_NODE_H
