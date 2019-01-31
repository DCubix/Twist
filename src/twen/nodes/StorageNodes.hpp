#ifndef TWEN_STORAGE_NODES_H
#define TWEN_STORAGE_NODES_H

#include "../NodeGraph.h"

class ReaderNode : public Node {
	TWEN_NODE(ReaderNode, "Reader")
public:
	ReaderNode(u32 slot = 0) : Node(), slot(slot) {}

	float sample(NodeGraph *graph) override {
		return graph->load(slot);
	}

	u32 slot;
};

class WriterNode : public Node {
	TWEN_NODE(WriterNode, "Writer")
public:
	WriterNode(u32 slot = 0) : Node(), slot(slot) {
		addInput("In", 0.0f);
	}

	float sample(NodeGraph *graph) override {
		float in = get(0);
		graph->store(slot, in);
		return in;
	}

	u32 slot;
};


#endif // TWEN_STORAGE_NODES_H
