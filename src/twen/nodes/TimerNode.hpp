#ifndef TWEN_TIMER_NODE_H
#define TWEN_TIMER_NODE_H

#include "../Node.h"

class TimerNode : public Node {
	TWEN_NODE(TimerNode, "Timer")
public:
	TimerNode(float sampleRate=44100.0f, float bpm=120.0f, float swing=0.0f)
		: Node(), m_sampleRate(sampleRate), index(0), currTime(0)
	{
		addOutput("Index");
		addOutput("Gate");
		addOutput("Freq");
		addOutput("Time");

		addParam("Swing", 0.0f, 0.9f, swing, 0.05f, NodeParam::DragRange);
		addParam("BPM", 0.0f, 255.0f, bpm, 1.0f, NodeParam::DragRange);
		
	}

	void solve() {
		const float step = (1.0f / m_sampleRate) * 4;
		const float delay = (60000.0f / param("BPM")) / 1000.0f;

		currTime += step;

		float sw = param("Swing") * delay;
		float delaySw = index % 2 == 0 ? delay + sw * 0.5f : delay - sw * 0.5f;
		updateIndex(index, currTime, gate, sw, delay);

		out("Index") = index;
		out("Gate") = gate ? 1 : 0;
		out("Freq") = param("BPM") / 60.0f;
		out("Time") = Utils::remap(currTime, 0.0f, delaySw, 0.0f, 1.0f);
	}

private:
	float m_sampleRate;

	bool gate = false;
	float currTime;
	int index;

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
