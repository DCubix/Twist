#ifndef TWEN_UTILS_H
#define TWEN_UTILS_H

#define _USE_MATH_DEFINES
#include <cmath>

constexpr float PI = M_PI;
constexpr float PI2 = M_PI * 2.0f;

#include <array>
#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <type_traits>
#include <memory>
#include <cstdint>

#include "json.hpp"
using JSON = nlohmann::json;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using TypeIndex = std::type_index;

template <typename T, u8 N> using Arr = std::array<T, N>;
template <typename T> using Vec = std::vector<T>;
template <typename K, typename V> using Map = std::map<K, V>;
template <typename K, typename V> using UMap = std::unordered_map<K, V>;
template <typename T> using Ptr = std::unique_ptr<T>;
using Str = std::string;

namespace Utils {
	float noteFrequency(int index);
	float noteFrequency(int index, int octave);
	int octave(int note);

	float lerp(float a, float b, float t);
	float remap(float value, float from1, float to1, float from2, float to2);
	float cyclef(float f);
	
	template <typename T>
	static TypeIndex getTypeIndex() {
		return std::type_index(typeid(T));
	}
}

class UID {
public:
	static u64 getNew() { return _id++; }
	static u64 get() { return _id; }
private:
	static u64 _id;
};

enum Note {
	C = 0,
	Cs,
	D,
	Ds,
	E,
	F,
	Fs,
	G,
	Gs,
	A,
	As,
	B,
	Count
};

#endif // TWEN_UTILS_H