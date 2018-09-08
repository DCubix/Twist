#ifndef TWEN_CHORUS_NODE_H
#define TWEN_CHORUS_NODE_H

#include "../Node.h"
#include "../intern/Oscillator.h"
#include "../intern/WaveGuide.h"

class ChorusNode : public Node {
	TWEN_NODE(ChorusNode, "Chorus")
public:
	ChorusNode(float sampleRate=44100.0f, float rate=0, float depth=0, float delay=0)
		: Node(), sampleRate(sampleRate)
	{
		m_lfo = Oscillator(sampleRate);
		m_lfo.amplitude(1.0f);
		m_lfo.waveForm(Oscillator::Sine);

		m_wv = WaveGuide(sampleRate);
		m_wv.clear();

		addInput("In");
		addOutput("Out");

		addParam("Rate", 0.0f, 6.0f, rate, 0.05f);
		addParam("Depth", 0.0f, 1.0f, depth, 0.05f);
		addParam("Delay", 0.0f, 1.0f, delay, 0.05f);
	}

	void solve() {
		float sgn = m_lfo.sample(param("Rate")) * param("Depth");
		float sgnDT = sgn * param("Delay");
		dt = sgnDT + param("Delay");
		float _out = m_wv.sample(in("In"), 0.0f, dt);
		out("Out") = ((_out + in("In")) * 0.5f);
	}

private:
	float sampleRate;
	float lpOut;
	float dt;

	Oscillator m_lfo;
	WaveGuide m_wv;
};

#endif // TWEN_CHORUS_NODE_H
