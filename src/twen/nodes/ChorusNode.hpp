#ifndef TWEN_CHORUS_NODE_H
#define TWEN_CHORUS_NODE_H

#include "../NodeGraph.h"
#include "../intern/Oscillator.h"
#include "../intern/WaveGuide.h"

class ChorusNode : public Node {
	TWEN_NODE(ChorusNode, "Chorus")
public:
	inline ChorusNode(float rate=0, float depth=0, float delay=0)
		: Node(), rate(rate), depth(depth), delay(delay)
	{
		m_lfo = Oscillator(44100.0f);
		m_lfo.amplitude(1.0f);
		m_lfo.waveForm(Oscillator::Sine);

		m_wv = WaveGuide(44100.0f);
		m_wv.clear();

		addInput("In"); // Input
	}

	inline Value sample(NodeGraph *graph) override {
		m_lfo.sampleRate(graph->sampleRate());
		m_wv.sampleRate(graph->sampleRate());

		float sgn = m_lfo.sample(rate) * depth;
		float sgnDT = sgn * delay;
		dt = sgnDT + delay;
		float _out = m_wv.sample(in(0).value(), 0.0f, dt);
		return Value(((_out + in(0).value()) * 0.5f));
	}

	inline void save(JSON& json) override {
		Node::save(json);
		json["rate"] = rate;
		json["depth"] = depth;
		json["delay"] = delay;
	}

	inline void load(JSON json) override {
		Node::load(json);
		rate = json["rate"];
		depth = json["depth"];
		delay = json["delay"];
	}

	float rate, depth, delay;

private:
	float lpOut;
	float dt;

	Oscillator m_lfo;
	WaveGuide m_wv;
};

#endif // TWEN_CHORUS_NODE_H
