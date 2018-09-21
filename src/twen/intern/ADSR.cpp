#include "ADSR.h"

float ADSR::sample() {
	switch (m_state) {
		case Idle: break;
		case Attack: {
			m_out = m_attackBase + m_out * m_attackCoef;
			if (m_out >= 1.0f) {
				m_out = 1.0f;
				m_state = Decay;
			}
		} break;
		case Decay: {
			m_out = m_decayBase + m_out * m_decayCoef;
			if (m_out <= m_sustain) {
				m_out = m_sustain;
				m_state = Sustain;
			}
		} break;
		case Sustain: break;
		case Release: {
			m_out = m_releaseBase + m_out * m_releaseCoef;
			if (m_out <= 0.0f) {
				m_out = 0.0f;
				m_state = Idle;
			}
		} break;
	}
	m_out = std::max(std::min(m_out, 1.0f), 0.0f);
	return m_out;
}

void ADSR::gate(bool g) {
	if (g) {
		m_state = Attack;
	} else if (m_state != Idle) {
		m_state = Release;
	}
}

float ADSR::coef(float rate, float targetRatio) {
	return (rate <= 0) ? 0.0f : std::exp(-std::log((1.0f + targetRatio) / targetRatio) / rate);
}

void ADSR::reset() {
	m_state = Release;
	m_out = 0.0f;
}