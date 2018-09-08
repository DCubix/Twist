#ifndef TWEN_MATH_NODE_H
#define TWEN_MATH_NODE_H

#include "../Node.h"

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

	MathNode(MathOp op, float a=0, float b=0) : Node() {
		addOutput("Out");
		addInput("A");
		addInput("B");

		addParam("Op", { "Add", "Sub", "Mul", "Div", "Neg", "Avg" }, op);
		addParam("A", a, 0.05f, false);
		addParam("B", b, 0.05f, false);
	}

	void solve() {
		FloatArray a = in("A", "A");
		FloatArray b = in("B", "B");
		FloatArray _out;

		MathOp op = (MathOp) paramOption("Op");
		switch (op) {
			case Add: _out = a + b; break;
			case Sub: _out = a - b; break;
			case Mul: _out = a * b; break;
			case Neg: _out = -a; break;
			case Average: _out = (a + b) * 0.5f; break;
		}
		outs("Out") = _out;
	}

};

#endif // TWEN_MATH_NODE_H
