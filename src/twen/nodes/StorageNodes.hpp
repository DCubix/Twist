#ifndef TWEN_STORAGE_NODES_H
#define TWEN_STORAGE_NODES_H

#include "../Node.h"
#include "../NodeGraph.h"

class ReaderNode : public Node {
	TWEN_NODE(ReaderNode)
public:
	ReaderNode(int idx) : Node(), idx(idx) {
		addOutput("Out");
	}

	void solve() {
		setOutput("Out", parent()->load(idx));
	}

	int idx = 0;
};

class WriterNode : public Node {
	TWEN_NODE(WriterNode)
public:
	WriterNode(int idx) : Node(), idx(idx) {
		addInput("In");
	}

	void solve() {
		parent()->store(idx, getInput("Out"));
	}

	int idx = 0;
};


#endif // TWEN_STORAGE_NODES_H