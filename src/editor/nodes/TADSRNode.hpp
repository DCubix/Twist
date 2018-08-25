#ifndef T_ADSR_NODE_H
#define T_ADSR_NODE_H

#include "TNode.h"
#include "../../TGen.h"

#include "imgui.h"

class TADSRNode : public TNode {
public:
	TADSRNode(float sampleRate, float a, float d, float s, float r)
		: TNode("ADSR", 140, 155),
		 m_adsr(new TADSR(0, 0, 1, 0)),
		 trigger(false), sr(sampleRate),
		 a(a), d(d), s(s), r(r)
	{
		addOutput("Out");
		addInput("Gate");
	}

	~TADSRNode() {
		delete m_adsr;
		m_adsr = nullptr;
	}

	void gui() {
		// ImGui::DragFloat("Attack", &a, 0.01f, 0.0f, 1.0f);
		// ImGui::DragFloat("Decay", &d, 0.01f, 0.0f, 1.0f);
		// ImGui::DragFloat("Sustain", &s, 0.01f, 0.0f, 1.0f);
		// ImGui::DragFloat("Release", &r, 0.01f, 0.0f, 1.0f);
		ImGui::Knob("Att.", &a, 0.0f, 1.0f);
		ImGui::SameLine();
		ImGui::Knob("Dec.", &d, 0.0f, 1.0f);
		ImGui::SameLine();
		ImGui::Knob("Sus.", &s, 0.0f, 1.0f);
		ImGui::SameLine();
		ImGui::Knob("Rel.", &r, 0.0f, 10.0f);
	}

	void solve() {
		if (getInput(0) >= 1.0f) {
			if (!trigger) {
				m_adsr->gate(true);
				trigger = true;
			}
		} else {
			if (trigger) {
				m_adsr->gate(false);
				trigger = false;
			}
		}

		m_adsr->attack(a * sr);
		m_adsr->decay(d * sr);
		m_adsr->sustain(s * sr);
		m_adsr->release(r * sr);
		value = m_adsr->sample();
		setOutput(0, value);
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
	bool trigger;

	static std::string type() { return "ADSR"; }

private:
	TADSR* m_adsr;
};

#endif // T_ADSR_NODE_H
