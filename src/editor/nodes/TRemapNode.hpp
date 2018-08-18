#ifndef T_REMAP_NODE_H
#define T_REMAP_NODE_H

#include "TNode.h"

class TRemapNode : public TNode {
public:
	TRemapNode(float om, float oma, float nm, float nma)
		: TNode("Remap", 140, 155),
			oldMin(om), oldMax(oma),
			newMin(nm), newMax(nma)
	{
		addInput("In");
		addOutput("Out");
	}

	void gui() {
		ImGui::InputFloat("Old Min", &oldMin, 0.01f, 0.1f, 3);
		ImGui::InputFloat("Old Max", &oldMax, 0.01f, 0.1f, 3);
		ImGui::InputFloat("New Min", &newMin, 0.01f, 0.1f, 3);
		ImGui::InputFloat("New Max", &newMax, 0.01f, 0.1f, 3);
	}

	void solve() {
		float v = getInput(0);
		float norm = (v - oldMin) / (oldMax - oldMin);
		setOutput(0, norm * (newMax - newMin) + newMin);
	}

	void save(JSON& json) {
		TNode::save(json);
		json["type"] = type();
		json["omin"] = oldMin;
		json["omax"] = oldMax;
		json["nmin"] = newMin;
		json["nmax"] = newMax;
	}

	float oldMin, oldMax, newMin, newMax;

	static std::string type() { return "Remap"; }
};

#endif // T_REMAP_NODE_H
