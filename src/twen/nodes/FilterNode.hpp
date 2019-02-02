#ifndef TWEN_FILTER_NODE_H
#define TWEN_FILTER_NODE_H

#include "../NodeGraph.h"

class FilterNode : public Node {
	TWEN_NODE(FilterNode, "Filter")
public:
	enum Filter {
		LowPass = 0,
		HighPass
	};

	FilterNode(float co=20, Filter filter=Filter::LowPass)
		: Node(), cutOff(co), filter(filter)
	{
		addInput("In"); // Input
		addInput("CutOff"); // Cutoff
	}

	Value sample(NodeGraph *graph) override {
		float co = connected(1) ? in(1).value() : cutOff;
		float _co = std::min(std::max(co, 20.0f), 20000.0f);
		float _in = in(0).value();

		switch (filter) {
			case LowPass: {
				if (_co > 0.0f) {
					float dt = 1.0f / graph->sampleRate();
					float a = dt / (dt + 1.0f / (2.0 * M_PI * _co));
					_out = Utils::lerp(_out, _in, a);
				} else {
					_out = 0.0f;
				}
			} break;
			case HighPass: {
				float rc = 1.0f / (2.0f * M_PI * _co);
				float a = rc / (rc + 1.0f / graph->sampleRate());

				float result = a * (prev + _in);
				prev = result - _in;

				_out = result;
			} break;
		}
		return Value(_out);
	}

	void save(JSON& json) override {
		Node::save(json);
		json["cutOff"] = cutOff;
		json["filter"] = int(filter);
	}

	void load(JSON json) override {
		Node::load(json);
		cutOff = json["cutOff"];
		filter = Filter(json["filter"].get<int>());
	}

	float cutOff;
	Filter filter;

private:
	float _out, prev;
};

#endif // TWEN_FILTER_NODE_H
