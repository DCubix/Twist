#ifndef T_REVERB_NODE_H
#define T_REVERB_NODE_H

#include "TNode.h"

extern "C" {
	#include "reverb.h"
}

class TReverbNode : public TNode {
public:
	TReverbNode(float sampleRate)
		: TNode("Reverb", 120, 120), sampleRate(sampleRate)
	{
		loaded = true;
		addInput("In");
		addOutput("Out");

		sf_presetreverb(&m_rev, (int)sampleRate, SF_REVERB_PRESET_DEFAULT);
	}
	void gui() {
		static const char* PRESETS[] = {
			"Default\0",
			"Small Hall 1\0",
			"Small Hall 2\0",
			"Medium Hall 1\0",
			"Medium Hall 2\0",
			"Large Hall 1\0",
			"Large Hall 2\0",
			"Small Room 1\0",
			"Small Room 2\0",
			"Medium Room 1\0",
			"Medium Room 2\0",
			"Large Room 1\0",
			"Large Room 2\0",
			"Medium ER 1\0",
			"Medium ER 2\0",
			"Plate High\0",
			"Plate Low\0",
			"Long Reverb 1\0",
			"Long Reverb 2\0",
			NULL
		};
		if (ImGui::Combo("Preset##revpres", &m_preset, PRESETS, 19)) {
			sf_presetreverb(&m_rev, (int)sampleRate, (sf_reverb_preset) m_preset);
			pset = SF_REVERB_PRESETS[m_preset];
		}

		bool change = false;
		change |= ImGui::DragInt("Oversample##osf", &pset.osf, 1, 1, 4);
		change |= ImGui::DragFloat("Early Reflection Amount##era", &pset.p1, 0.1f, 0.0f, 1.0f);
		change |= ImGui::DragFloat("Early Reflection Factor##erf", &pset.p4, 0.01f, 0.5f, 2.5f);
		change |= ImGui::DragFloat("Early Reflection Width##erw", &pset.p5, 0.1f, -1.0f, 1.0f);
		change |= ImGui::DragFloat("Final Wet##fwet", &pset.p2, 0.1f, -70.0f, 10.0f);
		change |= ImGui::DragFloat("Final Dry##fwet", &pset.p3, 0.1f, -70.0f, 10.0f);
		change |= ImGui::DragFloat("Mix Width##mwd", &pset.p6, 0.1f, 0.0f, 1.0f);
		change |= ImGui::DragFloat("Wet##wet", &pset.p7, 0.1f, -70.0f, 10.0f);
		change |= ImGui::DragFloat("Wander##wndr", &pset.p8, 0.01f, 0.1f, 0.6f);
		change |= ImGui::DragFloat("Bass Boost##bssbst", &pset.p9, 0.01f, 0.0f, 0.5f);
		change |= ImGui::DragFloat("Spin##spin", &pset.p10, 0.1f, 0.0f, 10.0f);
		change |= ImGui::DragFloat("In Lowpass Cutoff##ilpc", &pset.p11, 0.1f, 200.0f, 18000.0f);
		change |= ImGui::DragFloat("Bass Lowpass Cutoff##blpc", &pset.p12, 0.1f, 50.0f, 1050.0f);
		change |= ImGui::DragFloat("Damp Lowpass Cutoff##dlpc", &pset.p13, 0.1f, 200.0f, 18000.0f);
		change |= ImGui::DragFloat("Out Lowpass Cutoff##olpc", &pset.p14, 0.1f, 200.0f, 18000.0f);
		change |= ImGui::DragFloat("Decay##dec", &pset.p15, 0.1f, 0.1f, 30.0f);
		change |= ImGui::DragFloat("Delay##dec", &pset.p16, 0.01f, -0.5f, 0.5f);

		if (change || loaded) {
			sf_advancereverb(&m_rev, (int)sampleRate,
				pset.osf,
				pset.p1,
				pset.p2,
				pset.p3,
				pset.p4,
				pset.p5,
				pset.p6,
				pset.p7,
				pset.p8,
				pset.p9,
				pset.p10,
				pset.p11,
				pset.p12,
				pset.p13,
				pset.p14,
				pset.p15,
				pset.p16
			);
			loaded = false;
		}
	}

	void solve() {
		float in = getInput(0);
		sf_sample_st ins, outs;
		ins.L = in;
		ins.R = in;

		sf_reverb_process(&m_rev, 1, &ins, &outs);
		setOutput(0, (outs.L + outs.R) * 0.5f);
	}

	void save(JSON& json) {
		TNode::save(json);
		json["type"] = type();
		json["sampleRate"] = sampleRate;
		json["preset"] = {
			pset.osf,
			pset.p1,
			pset.p2,
			pset.p3,
			pset.p4,
			pset.p5,
			pset.p6,
			pset.p7,
			pset.p8,
			pset.p9,
			pset.p10,
			pset.p11,
			pset.p12,
			pset.p13,
			pset.p14,
			pset.p15,
			pset.p16
		};
	}

	static std::string type() { return "Reverb"; }

	sf_reverb_preset_t pset;
	sf_reverb_state_st m_rev;
	int m_preset = 0;
	float sampleRate;
	bool loaded = false;

};

#endif //T_REVERB_NODE_H