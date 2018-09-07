#ifndef TWEN_FILTER_NODE_H
#define TWEN_FILTER_NODE_H

#include "../Node.h"

class FilterNode : public Node {
	TWEN_NODE(FilterNode)
public:
	enum TFilter {
		LowPass = 0,
		HighPass
	};

	FilterNode(float sampleRate, float cut, TFilter filter)
		: Node(), filter(filter), sampleRate(sampleRate), out(0.0f), cut(cut)
	{
		addInput("In");
		addInput("CutOff");
		addOutput("Out");
	}

	void solve() {
		float cutOff = std::min(std::max(getInput("CutOff"), 20.0f), 20000.0f);
		float in = getInput("In");
		switch (filter) {
			case LowPass: {
				if (cutOff > 0.0f) {
					float dt = 1.0f / sampleRate;
					float a = dt / (dt + 1.0f / (2.0 * M_PI * cutOff));
					out = Utils::lerp(out, in, a);
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
		setOutput("Out", out);
	}

	TFilter filter;
	float sampleRate, out, prev, cut;

};

#endif // TWEN_FILTER_NODE_H
