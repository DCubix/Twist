#ifndef T_ADSR_NODE_H
#define T_ADSR_NODE_H

#include "TNode.h"
#include "../../TGen.h"

#include "imgui.h"

class TADSRNode : public TNode {
public:
	TADSRNode(float sampleRate, float a, float d, float s, float r)
		: TNode("ADSR", 140, 155),
		 sr(sampleRate), a(a), d(d), s(s), r(r)
	{
		addOutput("Out");
		addInput("Gate");

		for (int i = 0; i < FLT_ARR_MAX; i++) {
			m_adsr[i] = TADSR(a, d, s, r);
		}

		trigger.fill(false);
	}

	void gui() {
		ImGui::Knob("Att.", &a, 0.0f, 1.0f);
		ImGui::SameLine();
		ImGui::Knob("Dec.", &d, 0.0f, 1.0f);
		ImGui::SameLine();
		ImGui::Knob("Sus.", &s, 0.0f, 1.0f);
		ImGui::SameLine();
		ImGui::Knob("Rel.", &r, 0.0f, 10.0f);
	}

	void solve() {
		for (int i = 0; i < FLT_ARR_MAX; i++) {
			m_adsr[i].attack(a * sr);
			m_adsr[i].decay(d * sr);
			m_adsr[i].sustain(s * sr);
			m_adsr[i].release(r * sr);

			if (getMultiInput(0, i) >= 0.9f) {
				if (!trigger[i]) {
					m_adsr[i].gate(true);
					trigger[i] = true;
				}
			} else {
				if (trigger[i]) {
					m_adsr[i].gate(false);
					trigger[i] = false;
				}
			}

			setMultiOutput(0, i, m_adsr[i].sample());
		}
	}

	void save(JSON& json) {
		TNode::save(json);
		json["type"] = type();
		json["a"] = a;
		json["d"] = d;
		json["s"] = s;
		json["r"] = r;
		json["sampleRate"] = sr;
	}

	float value, sr, a, d, s, r;
	std::array<bool, FLT_ARR_MAX> trigger;

	static std::string type() { return "ADSR"; }

private:
	TADSR m_adsr[FLT_ARR_MAX];
};

#endif // T_ADSR_NODE_H
