#ifndef TWEN_FILTER_NODE_H
#define TWEN_FILTER_NODE_H

#include "../Node.h"

class FilterNode : public Node {
	TWEN_NODE(FilterNode, "Filter")
public:
	enum Filter {
		LowPass = 0,
		HighPass
	};

	FilterNode(float sampleRate=44100.0f, float co=20, u32 filter=0)
		: Node(), sampleRate(sampleRate)
	{
		addInput("In");
		addInput("CutOff");
		addOutput("Out");

		addParam("CutOff", 20.0f, 20000.0f, co);
		addParam("Filter", { "Low-Pass", "High-Pass" }, filter);
	}

	void solve() {
		float cutOff = std::min(std::max(in("CutOff", "CutOff"), 20.0f), 20000.0f);
		float _in = in("In");
		Filter filter = (Filter) paramOption("Filter");
		switch (filter) {
			case LowPass: {
				if (cutOff > 0.0f) {
					float dt = 1.0f / sampleRate;
					float a = dt / (dt + 1.0f / (2.0 * M_PI * cutOff));
					_out = Utils::lerp(_out, _in, a);
				} else {
					_out = 0.0f;
				}
			} break;
			case HighPass: {
				float rc = 1.0f / (2.0f * M_PI * cutOff);
				float a = rc / (rc + 1.0f / sampleRate);

				float result = a * (prev + _in);
				prev = result - _in;

				_out = result;
			} break;
		}
		out("Out") = _out;
	}

private:
	float sampleRate, _out, prev;
};

#endif // TWEN_FILTER_NODE_H
