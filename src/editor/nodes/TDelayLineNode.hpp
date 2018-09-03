#ifndef T_DELAY_LINE_NODE_H
#define T_DELAY_LINE_NODE_H

#include "TNode.h"
#include "../../TGen.h"

class TDelayLineNode : public TNode {
public:
	TDelayLineNode(float sampleRate, float fd, int dl)
		: TNode("Delay Line", 0, 0),
		feedback(fd), delay(dl), m_wv(TWaveGuide(sampleRate))
	{
		addInput("In");
		addOutput("Out");
	}

	void gui() {
		ImGui::PushItemWidth(75);
		ImGui::DragFloat("Feedback", &feedback, 0.1f, 0.0f, 0.99f);
		ImGui::InputInt("Delay (ms)", &delay);
		ImGui::PopItemWidth();
	}

	void solve() {
		float in = getInputOr(0);
		setOutput(0, m_wv.sample(in, feedback, delay));
	}

	void save(JSON& json) {
		TNode::save(json);
		json["type"] = type();
		json["feedback"] = feedback;
		json["delay"] = delay;
	}

	float feedback;
	int delay;

	static std::string type() { return "Delay Line"; };

private:
	TWaveGuide m_wv;

};

#endif // T_DELAY_LINE_H