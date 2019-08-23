#ifndef TWEN_ADSR_NODE_H
#define TWEN_ADSR_NODE_H

#include "../NodeGraph.h"
#include "../intern/ADSR.h"

class ADSRNode : public Node {
	TWEN_NODE(ADSRNode, "ADSR")
public:
	inline ADSRNode(float a=0, float d=0, float s=0, float r=0)
		: Node(), a(a), d(d), s(s), r(r)
	{
		addInput("Gate"); // Gate

		m_adsr = ADSR(0.0f, 0.0f, 0.0f, 0.0f);
		m_trigger = false;
	}

	inline Value sample(NodeGraph *graph) override {
		float sr = graph->sampleRate();
		m_adsr.attack(a * sr);
		m_adsr.decay(d * sr);
		m_adsr.sustain(s);
		m_adsr.release(r * sr);

		if (in(0).gate()) {
			if (!m_trigger) {
				m_adsr.gate(true);
				m_trigger = true;
			}
		} else {
			if (m_trigger) {
				m_adsr.gate(false);
				m_trigger = false;
			}
		}

		return Value(m_adsr.sample());
	}

	inline void save(JSON& json) override {
		Node::save(json);
		json["a"] = a;
		json["d"] = d;
		json["s"] = s;
		json["r"] = r;
	}

	inline void load(JSON json) override {
		Node::load(json);
		a = json["a"];
		d = json["d"];
		s = json["s"];
		r = json["r"];
	}

	inline ADSR& adsr() { return m_adsr; }

	float a, d, s, r;

private:
	ADSR m_adsr;
	bool m_trigger;
};

#endif // TWEN_ADSR_NODE_H
