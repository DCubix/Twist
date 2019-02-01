#ifndef TWEN_REMAP_NODE_H
#define TWEN_REMAP_NODE_H

#include "../NodeGraph.h"

class RemapNode : public Node {
	TWEN_NODE(RemapNode, "Remap")
public:
	RemapNode(float omin=0, float omax=1, float nmin=0, float nmax=1)
		: Node(),
		  fromMin(omin), fromMax(omax), toMin(nmin), toMax(nmax)
	{
		addInput("In");
	}

	float sample(NodeGraph *graph) override {
		return Utils::remap(get(0), fromMin, fromMax, toMin, toMax);
	}

	void save(JSON& json) override {
		Node::save(json);
		json["from"] = { fromMin, fromMax };
		json["to"] = { toMin, toMax };
	}

	void load(JSON json) override {
		Node::load(json);
		fromMin = json["from"][0];
		fromMax = json["from"][1];
		toMin = json["to"][0];
		toMax = json["to"][1];
	}

	float fromMin, fromMax, toMin, toMax;
};

#endif // TWEN_REMAP_NODE_H
