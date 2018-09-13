#ifndef TWEN_STORAGE_NODES_H
#define TWEN_STORAGE_NODES_H

#include "../Node.h"
#include "../NodeGraph.h"

class ReaderNode : public Node {
	TWEN_NODE(ReaderNode, "Reader")
public:
	ReaderNode(int idx=0) : Node() {
		addOutput("Out");
		addParam("Slot", 0, GLOBAL_STORAGE_SIZE, idx, 1.0f, NodeParam::IntRange);
	}

	void solve() {
		out(0) = parent()->load((int) param(0));
	}
};

class WriterNode : public Node {
	TWEN_NODE(WriterNode, "Writer")
public:
	WriterNode(int idx=0) : Node() {
		addInput("In");
		addParam("Slot", 0, GLOBAL_STORAGE_SIZE, idx, 1.0f, NodeParam::IntRange);
	}

	void solve() {
		parent()->store((int) param(0), in(0));
	}
};


#endif // TWEN_STORAGE_NODES_H
