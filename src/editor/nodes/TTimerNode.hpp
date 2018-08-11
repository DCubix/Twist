#ifndef T_TIMER_NODE_H
#define T_TIMER_NODE_H

#include "TNode.h"

class TTimerNode : public TNode {
public:
	TTimerNode(float sampleRate, float bpm, float swing)
		: TNode("Timer", 120, 120),
		bpm(bpm), sampleRate(sampleRate),
		swing(swing),
		index0(0), index1(0), index2(0),
		time0(0), time1(0), time2(0)
	{
		addOutput("I0");
		addOutput("I1");
		addOutput("I2");
		addOutput("4");
		addOutput("2");
		addOutput("1");
	}

	void gui() {
		if (ImGui::Button("Reset", ImVec2(120, 24))) {
			index0 = index1 = index2 = 0;
			gate0 = gate1 = gate2 = 0;
			time0 = time1 = time2 = 0;
		}
		ImGui::DragFloat("Swing##sw", &swing, 0.1f, 0.0f, 1.0f);
		ImGui::DragFloat("BPM##bpm", &bpm, 0.5f, 40.0f, 256.0f);
	}

	void solve() {
		const float step = (1.0f / sampleRate) * 4;
		const float delay = (60000.0f / bpm) / 1000.0f;

		time0 += step;
		time1 += step / 2;
		time2 += step / 4;

		float sw = swing * delay;
		updateIndex(index0, time0, gate0, sw, delay);
		updateIndex(index1, time1, gate1, sw, delay);
		updateIndex(index2, time2, gate2, sw, delay);

		setOutput(0, index0);
		setOutput(1, index1);
		setOutput(2, index2);
		setOutput(3, gate0 ? 1 : 0);
		setOutput(4, gate1 ? 1 : 0);
		setOutput(5, gate2 ? 1 : 0);
	}

	virtual void save(JSON& json) {
		TNode::save(json);
		json["type"] = type();
		json["sampleRate"] = sampleRate;
		json["bpm"] = bpm;
		json["swing"] = swing;
	}

	float bpm, sampleRate, swing;

	bool gate0 = false, gate1 = false, gate2 = false;
	float time0, time1, time2;
	int index0, index1, index2;

	static std::string type() { return "Timer"; }

private:
	void updateIndex(int& index, float& stime, bool& gate, float sw, float delay) {
		float delaySw = index % 2 == 0 ? delay - sw * 0.5f : delay + sw * 0.5f;
		if (stime >= delaySw) {
			index++;
			stime = 0;
			gate = false;
		} else {
			gate = true;
		}
	}
};

#endif // T_TIMER_NODE_H