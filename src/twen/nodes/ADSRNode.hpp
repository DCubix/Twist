#ifndef TWEN_ADSR_NODE_H
#define TWEN_ADSR_NODE_H

#include "../Node.h"
#include "../intern/ADSR.h"

class ADSRNode : public Node {
	TWEN_NODE(ADSRNode, "ADSR")
public:
	ADSRNode(float sampleRate=44100.0f, float a=0, float d=0, float s=0, float r=0)
		: Node(), m_sampleRate(sampleRate)
	{
		addOutput("Out");
		addInput("Gate");
		
		addParam("A", 0.0f, 10.0f, a, 0.05f, NodeParam::KnobRange, true);
		addParam("D", 0.0f, 10.0f, d, 0.05f, NodeParam::KnobRange, true);
		addParam("S", 0.0f,  1.0f, s, 0.05f, NodeParam::KnobRange, true);
		addParam("R", 0.0f, 10.0f, r, 0.05f, NodeParam::KnobRange);

		for (int i = 0; i < FLOAT_ARRAY_MAX; i++) {
			m_adsr[i] = ADSR(0.0f, 0.0f, 0.0f, 0.0f);
		}

		m_trigger.fill(false);
	}

	void solve() {
		for (int i = 0; i < FLOAT_ARRAY_MAX; i++) {
			m_adsr[i].attack(param("A") * m_sampleRate);
			m_adsr[i].decay(param("D") * m_sampleRate);
			m_adsr[i].sustain(param("S") * m_sampleRate);
			m_adsr[i].release(param("R") * m_sampleRate);

			if (ins("Gate")[i] >= 0.9f) {
				if (!m_trigger[i]) {
					m_adsr[i].gate(true);
					m_trigger[i] = true;
				}
			} else {
				if (m_trigger[i]) {
					m_adsr[i].gate(false);
					m_trigger[i] = false;
				}
			}

			outs("Out")[i] = m_adsr[i].sample();
		}
	}

private:
	ADSR m_adsr[FLOAT_ARRAY_MAX];
	float m_sampleRate;
	Arr<bool, FLOAT_ARRAY_MAX> m_trigger;
};

#endif // TWEN_ADSR_NODE_H
