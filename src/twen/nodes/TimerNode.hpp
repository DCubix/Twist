#ifndef TWEN_TIMER_NODE_H
#define TWEN_TIMER_NODE_H

#include "../Node.h"

class TimerNode : public Node {
	TWEN_NODE(TimerNode)
public:
	TimerNode(float sampleRate, float bpm, float swing)
		: Node(),
		bpm(bpm), sampleRate(sampleRate),
		swing(swing), index(0), currTime(0)
	{
		addOutput("Index");
		addOutput("Gate");
		addOutput("Freq");
		addOutput("Time");
	}

	void solve() {
		const float step = (1.0f / sampleRate) * 4;
		const float delay = (60000.0f / bpm) / 1000.0f;

		currTime += step;

		float sw = swing * delay;
		float delaySw = index % 2 == 0 ? delay + sw * 0.5f : delay - sw * 0.5f;
		updateIndex(index, currTime, gate, sw, delay);

		setOutput("Index", index);
		setOutput("Gate", gate ? 1 : 0);
		setOutput("Freq", bpm / 60.0f);
		setOutput("Time", Utils::remap(currTime, 0.0f, delaySw, 0.0f, 1.0f));
	}

	float bpm, sampleRate, swing;

	bool gate = false;
	float currTime;
	int index;

private:
	void updateIndex(int& index, float& stime, bool& gate, float sw, float delay) {
		float delaySw = index % 2 == 0 ? delay + sw * 0.5f : delay - sw * 0.5f;
		if (stime >= delaySw) {
			index++;
			stime = 0;
			gate = false;
		} else {
			gate = true;
		}
	}
};

#endif // TWEN_TIMER_NODE_H
