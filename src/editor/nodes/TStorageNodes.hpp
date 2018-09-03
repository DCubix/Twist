#ifndef T_STORAGE_NODES_H
#define T_STORAGE_NODES_H

#include "TNode.h"
#include "../TNodeGraph.h"

class TReaderNode : public TNode {
public:
	TReaderNode(int idx) : TNode("Reader", 0, 0), idx(idx) {
		addOutput("Out");
	}

	void gui() {
		ImGui::PushItemWidth(75);
		ImGui::DragInt("Loc", &idx, 0.1f, 0, GLOBAL_STORAGE_SIZE);
		ImGui::PopItemWidth();
	}

	void solve() {
		setOutput(0, parent()->load(idx));
	}

	void save(JSON& json) {
		TNode::save(json);
		json["type"] = type();
		json["idx"] = idx;
	}

	static std::string type() { return "Reader"; }

	int idx = 0;
};

class TWriterNode : public TNode {
public:
	TWriterNode(int idx) : TNode("Writer", 0, 0), idx(idx) {
		addInput("In");
	}

	void gui() {
		ImGui::PushItemWidth(75);
		ImGui::DragInt("Loc", &idx, 0.1f, 0, GLOBAL_STORAGE_SIZE);
		ImGui::PopItemWidth();
	}

	void solve() {
		parent()->store(idx, getInputOr(0));
	}

	void save(JSON& json) {
		TNode::save(json);
		json["type"] = type();
		json["idx"] = idx;
	}

	static std::string type() { return "Writer"; }

	int idx = 0;
};


#endif // T_STORAGE_NODES_H