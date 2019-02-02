#ifndef TWEN_DELAY_LINE_NODE_H
#define TWEN_DELAY_LINE_NODE_H

#include "../NodeGraph.h"
#include "../intern/WaveGuide.h"

class DelayLineNode : public Node {
	TWEN_NODE(DelayLineNode, "Delay Line")
public:
	DelayLineNode(float fb=0, float dl=0)
		: Node(), m_wv(WaveGuide(44100.0f)), feedBack(fb), delay(dl)
	{
		addInput("In"); // Input
	}

	Value sample(NodeGraph *graph) override {
		m_wv.sampleRate(graph->sampleRate());
		return Value(m_wv.sample(in(0).value(), feedBack, delay));
	}

	void save(JSON& json) override {
		Node::save(json);
		json["feedBack"] = feedBack;
		json["delay"] = delay;
	}

	void load(JSON json) override {
		Node::load(json);
		feedBack = json["feedBack"];
		delay = json["delay"];
	}

	float feedBack, delay;

private:
	WaveGuide m_wv;

};

#endif // TWEN_DELAY_LINE_NODE_H
