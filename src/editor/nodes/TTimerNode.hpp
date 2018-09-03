#ifndef T_TIMER_NODE_H
#define T_TIMER_NODE_H

#include "TNode.h"

class TTimerNode : public TNode {
public:
	TTimerNode(float sampleRate, float bpm, float swing)
		: TNode("Timer", 120, 120),
		bpm(bpm), sampleRate(sampleRate),
		swing(swing), index(0), currTime(0)
	{
		addOutput("Index");
		addOutput("Gate");
		addOutput("Freq");
		addOutput("Time");
	}

	void gui() {
		ImGui::PushItemWidth(75);
		if (ImGui::Button("Reset", ImVec2(75, 24))) {
			currTime = 0.0f;
			index = 0;
		}
		ImGui::DragFloat("BPM##bpm", &bpm, 0.5f, 40.0f, 256.0f);
		ImGui::DragFloat("Swing##sw", &swing, 0.1f, 0.0f, 1.0f);
		ImGui::PopItemWidth();
	}

	void solve() {
		const float step = (1.0f / sampleRate) * 4;
		const float delay = (60000.0f / bpm) / 1000.0f;

		currTime += step;

		float sw = swing * delay;
		float delaySw = index % 2 == 0 ? delay + sw * 0.5f : delay - sw * 0.5f;
		updateIndex(index, currTime, gate, sw, delay);

		setOutput(0, index);
		setOutput(1, gate ? 1 : 0);
		setOutput(2, bpm / 60.0f);
		setOutput(3, tmath::remap(currTime, 0.0f, delaySw, 0.0f, 1.0f));
	}

	void save(JSON& json) {
		TNode::save(json);
		json["type"] = type();
		json["sampleRate"] = sampleRate;
		json["bpm"] = bpm;
		json["swing"] = swing;
	}

	float bpm, sampleRate, swing;

	bool gate = false;
	float currTime;
	int index;

	static std::string type() { return "Timer"; }

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

#endif // T_TIMER_NODE_H
