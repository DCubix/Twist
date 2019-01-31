#ifndef TWEN_CHORUS_NODE_H
#define TWEN_CHORUS_NODE_H

#include "../NodeGraph.h"
#include "../intern/Oscillator.h"
#include "../intern/WaveGuide.h"

class ChorusNode : public Node {
	TWEN_NODE(ChorusNode, "Chorus")
public:
	ChorusNode(float rate=0, float depth=0, float delay=0)
		: Node(), rate(rate), depth(depth), delay(delay)
	{
		m_lfo = Oscillator(44100.0f);
		m_lfo.amplitude(1.0f);
		m_lfo.waveForm(Oscillator::Sine);

		m_wv = WaveGuide(44100.0f);
		m_wv.clear();

		addInput("In"); // Input
	}

	float sample(NodeGraph *graph) override {
		m_lfo.sampleRate(graph->sampleRate());
		m_wv.sampleRate(graph->sampleRate());

		float sgn = m_lfo.sample(rate) * depth;
		float sgnDT = sgn * delay;
		dt = sgnDT + delay;
		float _out = m_wv.sample(get(0), 0.0f, dt);
		return ((_out + get(0)) * 0.5f);
	}

	float rate, depth, delay;

private:
	float lpOut;
	float dt;

	Oscillator m_lfo;
	WaveGuide m_wv;
};

#endif // TWEN_CHORUS_NODE_H
