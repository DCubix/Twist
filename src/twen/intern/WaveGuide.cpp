#include "WaveGuide.h"

WaveGuide::WaveGuide(float sampleRate) {
	m_sampleRate = sampleRate;
	clear();
}

void WaveGuide::clear() {
	m_counter = 0;
	m_buffer.fill(0.0f);
}

float WaveGuide::sample(float in, float feedBack, float delay) {
	float delayS = float(delay) / 1000.0f;
	int delaySmp = int(delayS * m_sampleRate);

	// calculate delay offset
	int back = m_counter - delaySmp;

	// clip lookback buffer-bound
	if (back < 0) {
		back = WAVE_GUIDE_SAMPLES + back;
	}

	// compute interpolation left-floor
	int index0 = back;

	// compute interpolation right-floor
	int index_1 = index0 - 1;
	int index1 = index0 + 1;
	int index2 = index0 + 2;

	// clip interp. buffer-bound
	if (index_1 < 0) index_1 = WAVE_GUIDE_SAMPLES - 1;
	if (index1 >= WAVE_GUIDE_SAMPLES) index1 = 0;
	if (index2 >= WAVE_GUIDE_SAMPLES) index2 = 0;

	// get neighbour samples
	float y_1 = m_buffer[index_1];
	float y0 = m_buffer[index0];
	float y1 = m_buffer[index1];
	float y2 = m_buffer[index2];

	// compute interpolation x
	const float x = float(back) - float(index0);

	// calculate
	float c0 = y0;
	float c1 = 0.5f * (y1 - y_1);
	float c2 = y_1 - 2.5f * y0 + 2.0f * y1 - 0.5f * y2;
	float c3 = 0.5f * (y2 - y_1) + 1.5f * (y0 - y1);

	float output = ((c3 * x + c2) * x + c1) * x + c0;

	// add to delay buffer
	m_buffer[m_counter++] = in + output * feedBack;

	// clip delay counter
	if (m_counter >= WAVE_GUIDE_SAMPLES) m_counter = 0;

	// return output
	return output;
}