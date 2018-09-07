#ifndef TWEN_ADSR_NODE_H
#define TWEN_ADSR_NODE_H

#include "../Node.h"
#include "../intern/ADSR.h"

class ADSRNode : public Node {
	TWEN_NODE(ADSRNode)
public:

	ADSRNode(float sampleRate, float a, float d, float s, float r)
		: Node(), sr(sampleRate), a(a), d(d), s(s), r(r)
	{
		addOutput("Out");
		addInput("Gate");

		for (int i = 0; i < FLOAT_ARRAY_MAX; i++) {
			m_adsr[i] = ADSR(a, d, s, r);
		}

		trigger.fill(false);
	}

	void solve() {
		for (int i = 0; i < FLOAT_ARRAY_MAX; i++) {
			m_adsr[i].attack(a * sr);
			m_adsr[i].decay(d * sr);
			m_adsr[i].sustain(s * sr);
			m_adsr[i].release(r * sr);

			if (getMultiInput("Gate", i) >= 0.9f) {
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

			setMultiOutput("Out", i, m_adsr[i].sample());
		}
	}

	float value, sr, a, d, s, r;
	std::array<bool, FLOAT_ARRAY_MAX> trigger;

private:
	ADSR m_adsr[FLOAT_ARRAY_MAX];
};

#endif // TWEN_ADSR_NODE_H
