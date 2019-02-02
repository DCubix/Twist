#ifndef TWEN_STORAGE_NODES_H
#define TWEN_STORAGE_NODES_H

#include "../NodeGraph.h"

class ReaderNode : public Node {
	TWEN_NODE(ReaderNode, "Reader")
public:
	ReaderNode(u32 slot = 0) : Node(), slot(slot) {}

	Value sample(NodeGraph *graph) override {
		return Value(graph->load(slot));
	}

	void save(JSON& json) override {
		Node::save(json);
		json["slot"] = slot;
	}

	void load(JSON json) override {
		Node::load(json);
		slot = json["slot"];
	}

	u32 slot;
};

class WriterNode : public Node {
	TWEN_NODE(WriterNode, "Writer")
public:
	WriterNode(u32 slot = 0) : Node(), slot(slot) {
		addInput("In", 0.0f);
	}

	Value sample(NodeGraph *graph) override {
		float _in = in(0).value();
		graph->store(slot, _in);
		return Value(_in);
	}

	void save(JSON& json) override {
		Node::save(json);
		json["slot"] = slot;
	}

	void load(JSON json) override {
		Node::load(json);
		slot = json["slot"];
	}

	u32 slot;
};


#endif // TWEN_STORAGE_NODES_H
