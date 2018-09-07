#include "Utils.h"

static const float NOTE[] = {
	32.70320f,
	34.64783f,
	36.70810f,
	38.89087f,
	41.20344f,
	43.65353f,
	46.24930f,
	48.99943f,
	51.91309f,
	55.00000f,
	58.27047f,
	61.73541f
};

namespace Utils {
	float noteFrequency(int index) {
		return noteFrequency(index % 12, octave(index));
	}

	float noteFrequency(int index, int octave) {
		return NOTE[index] * std::pow(2, octave);
	}

	int octave(int note) {
		return note / 12;
	}

	float lerp(float a, float b, float t) {
		return (1.0f - t) * a + b * t;
	}

	float remap(float value, float from1, float to1, float from2, float to2) {
		return (value - from1) / (to1 - from1) * (to2 - from2) + from2;
	}

	float cyclef(float f) {
		auto m2 = std::fmod(f, 2.0f);
		return m2 < 1.0 ? m2 : 2 - m2;
	}
}

u64 UID::_id = 0;