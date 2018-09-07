#ifndef TWEN_CHORUS_NODE_H
#define TWEN_CHORUS_NODE_H

#include "../Node.h"
#include "../intern/Oscillator.h"
#include "../intern/WaveGuide.h"

class ChorusNode : public Node {
	TWEN_NODE(ChorusNode)
public:
	ChorusNode(float sampleRate, float dt=1, float cr=1.0f, float cd=1.0f)
		: Node(), sampleRate(sampleRate), delayTime(dt), chorusRate(cr), chorusDepth(cd)
	{
		m_lfo = Oscillator(sampleRate);
		m_lfo.amplitude(1.0f);
		m_lfo.waveForm(Oscillator::Sine);

		m_wv = WaveGuide(sampleRate);
		m_wv.clear();

		addInput("In");
		addOutput("Out");
	}

	void solve() {
		float sgn = m_lfo.sample(chorusRate) * chorusDepth;
		float sgnDT = sgn * delayTime;
		dt = sgnDT + delayTime;
		float out = m_wv.sample(getInput("In"), 0.0f, dt);
		setOutput("Out", (out + getInput("In")) * 0.5f);
	}

	float sampleRate;
	float lpOut;
	float dt;

	float chorusRate, chorusDepth, delayTime;

private:
	Oscillator m_lfo;
	WaveGuide m_wv;
};

#endif // TWEN_CHORUS_NODE_H
