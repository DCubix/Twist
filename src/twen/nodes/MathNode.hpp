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

	inline MathNode(MathOp op, float a=0, float b=0)
		: Node(), op(op), a(a), b(b)
	{
		addInput("A"); // A
		addInput("B"); // B
	}

	inline Value sample(NodeGraph *graph) override {
		float _a = connected(0) ? in(0).value() : a;
		float _b = connected(1) ? in(1).value() : b;
		float _out = 0.0f;

		switch (op) {
			case Add: _out = _a + _b; break;
			case Sub: _out = _a - _b; break;
			case Mul: _out = _a * _b; break;
			case Neg: _out = -_a; break;
			case Average: _out = (_a + _b) * 0.5f; break;
			default: break;
		}
		return Value(_out);
	}

	inline void save(JSON& json) override {
		Node::save(json);
		json["op"] = int(op);
		json["values"] = { a, b };
	}

	inline void load(JSON json) override {
		Node::load(json);
		op = MathOp(json["op"].get<int>());
		a = json["values"][0];
		b = json["values"][1];
	}

	MathOp op;
	float a, b;

};

#endif // TWEN_MATH_NODE_H
