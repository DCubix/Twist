#ifndef T_FILTER_NODE_H
#define T_FILTER_NODE_H

#include "TNode.h"
#include "../../TGen.h"

class TFilterNode : public TNode {
public:
	enum TFilter {
		LowPass = 0,
		HighPass
	};

	TFilterNode(float sampleRate, float cut, TFilter filter)
		: TNode("Filter", 140, 100),
		filter(filter), sampleRate(sampleRate), out(0.0f), cut(cut)
	{
		addInput("In");
		addInput("CutOff");
		addOutput("Out");
	}

	void gui() {
		static const char* OPS[] = { "Low-Pass\0", "High-Pass\0" };
		ImGui::Combo("Filter", (int*)&filter, OPS, 2, -1);
		ImGui::DragFloat("Cut-Off", &cut, 0.5f, 20.0f, 20000.0f);
	}

	void solve() {
		float cutOff = std::min(std::max(getInputOr(1, cut), 20.0f), 20000.0f);
		float in = getInput(0);
		switch (filter) {
			case LowPass: {
				if (cutOff > 0.0f) {
					float dt = 1.0f / sampleRate;
					float a = dt / (dt + 1.0f / (2.0 * M_PI * cutOff));
					out = tmath::lerp(out, in, a);
				} else {
					out = 0.0f;
				}
			} break;
			case HighPass: {
				float rc = 1.0f / (2.0f * M_PI * cutOff);
				float a = rc / (rc + 1.0f / sampleRate);

				float result = a * (prev + in);
				prev = result - in;

				out = result;
			} break;
		}
		setOutput(0, out);
	}

	void save(JSON& json) {
		TNode::save(json);
		json["type"] = type();
		json["sampleRate"] = sampleRate;
		json["cutOff"] = cut;
		json["filter"] = (int) filter;
	}

	TFilter filter;
	float sampleRate, out, prev, cut;

	static std::string type() { return "Filter"; }
};

#endif // T_FILTER_NODE_H
