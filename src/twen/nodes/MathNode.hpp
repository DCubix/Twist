#ifndef TWEN_MATH_NODE_H
#define TWEN_MATH_NODE_H

#include "../Node.h"

class MathNode : public Node {
	TWEN_NODE(MathNode)
public:
	enum TMathNodeOp {
		Add = 0,
		Sub,
		Mul,
		Neg,
		Average,
		OpCount
	};

	MathNode(TMathNodeOp op, float a, float b)
		: Node(), op(op), aValue(a), bValue(b)
	{
		addOutput("Out");
		addInput("A");
		addInput("B");
	}

	void solve() {
		FloatArray a = inputs("A");
		FloatArray b = inputs("B");
		FloatArray out;
		switch (op) {
			case Add: out = a + b; break;
			case Sub: out = a - b; break;
			case Mul: out = a * b; break;
			case Neg: out = -a; break;
			case Average: out = (a + b) * 0.5f; break;
		}
		setMultiOutputValues("Out", out);
	}

	TMathNodeOp op;
	float aValue = 0.0f, bValue = 0.0f;

};

#endif // TWEN_MATH_NODE_H
