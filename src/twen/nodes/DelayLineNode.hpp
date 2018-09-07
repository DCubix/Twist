#ifndef TWEN_DELAY_LINE_NODE_H
#define TWEN_DELAY_LINE_NODE_H

#include "../Node.h"
#include "../intern/WaveGuide.h"

class DelayLineNode : public Node {
	TWEN_NODE(DelayLineNode)
public:
	DelayLineNode(float sampleRate, float fd, int dl)
		: Node(), feedback(fd), delay(dl), m_wv(WaveGuide(sampleRate))
	{
		addInput("In");
		addOutput("Out");
	}

	void solve() {
		float in = getInput("In");
		setOutput(0, m_wv.sample(in, feedback, delay));
	}

	float feedback;
	int delay;

private:
	WaveGuide m_wv;

};

#endif // TWEN_DELAY_LINE_NODE_H