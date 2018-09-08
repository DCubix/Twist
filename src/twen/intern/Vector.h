#ifndef TWEN_VECTOR_H
#define TWEN_VECTOR_H

#include <array>
#include <cstdint>
#include <assert.h>

#ifdef __GNUC__
	#include <x86intrin.h>
#else
	#include <immintrin.h>         // MS version of immintrin.h covers AVX, AVX2 and FMA3
#endif // __GNUC__

#if defined(USING_SSE2)
#include <emmintrin.h>
#elif defined(USING_SSE3)
#include <pmmintrin.h>
#elif defined(USING_SSE2_MSVC)
#include <intrin.h>
#endif

#if defined(USING_SSE2) || defined(USING_SSE3) || defined(USING_SSE2_MSVC)
#define USING_SIMD
#define FLOAT_SIZE sizeof(float)
#define VEC_SIZE(x) (x - x % FLOAT_SIZE)
#endif

template <uint32_t N>
class Vector {
public:
	Vector() { m_data.fill(0.0f); }
	Vector(float v) { m_data.fill(0.0f); }
	Vector(const std::array<float, N>& data) {
#ifdef USING_SIMD
	size_t sz = VEC_SIZE(m_data.size());
	size_t inc = FLOAT_SIZE;
	for (size_t i = 0; i < sz; i += inc) {
		__m128 a = _mm_load_ps(&data[i]);
		_mm_store_ps(&m_data[i], a);
	}
	for (size_t i = sz; i < m_data.size(); i++)
		m_data[i] = data[i];
#else
	for (size_t i = 0; i < m_data.size(); i++)
		m_data[i] = data[i];
#endif
	}

	Vector<N> operator+ (const Vector<N>& o) {
		Vector<N> ret;
#ifdef USING_SIMD
		size_t sz = VEC_SIZE(m_data.size());
		size_t inc = FLOAT_SIZE;
		for (size_t i = 0; i < sz; i += inc) {
			__m128 a = _mm_load_ps(&m_data[i]);
			__m128 b = _mm_load_ps(&o.m_data[i]);
			__m128 res = _mm_add_ps(a, b);
			_mm_store_ps(&ret[i], res);
		}
		for (size_t i = sz; i < m_data.size(); i++)
			ret[i] = m_data[i] + o[i];
#else
		for (size_t i = 0; i < m_data.size(); i++)
			ret[i] = m_data[i] + o[i];
#endif
		return ret;
	}

	Vector<N> operator- (const Vector<N>& o) {
		Vector<N> ret;
#ifdef USING_SIMD
		size_t sz = VEC_SIZE(m_data.size());
		size_t inc = FLOAT_SIZE;
		for (size_t i = 0; i < sz; i += inc) {
			__m128 a = _mm_load_ps(&m_data[i]);
			__m128 b = _mm_load_ps(&o.m_data[i]);
			__m128 res = _mm_sub_ps(a, b);
			_mm_store_ps(&ret[i], res);
		}
		for (size_t i = sz; i < m_data.size(); i++)
			ret[i] = m_data[i] - o[i];
#else
		for (size_t i = 0; i < m_data.size(); i++)
			ret[i] = m_data[i] - o[i];
#endif
		return ret;
	}

	Vector<N> operator* (const Vector<N>& o) {
		Vector<N> ret;
#ifdef USING_SIMD
		size_t sz = VEC_SIZE(m_data.size());
		size_t inc = FLOAT_SIZE;
		for (size_t i = 0; i < sz; i += inc) {
			__m128 a = _mm_load_ps(&m_data[i]);
			__m128 b = _mm_load_ps(&o.m_data[i]);
			__m128 res = _mm_mul_ps(a, b);
			_mm_store_ps(&ret[i], res);
		}
		for (size_t i = sz; i < m_data.size(); i++)
			ret[i] = m_data[i] * o[i];
#else
		for (size_t i = 0; i < m_data.size(); i++)
			ret[i] = m_data[i] * o[i];
#endif
		return ret;
	}

	Vector<N> operator/ (const Vector<N>& o) {
		Vector<N> ret;
#ifdef USING_SIMD
		size_t sz = VEC_SIZE(m_data.size());
		size_t inc = FLOAT_SIZE;
		for (size_t i = 0; i < sz; i += inc) {
			__m128 a = _mm_load_ps(&m_data[i]);
			__m128 b = _mm_load_ps(&o.m_data[i]);
			__m128 res = _mm_div_ps(a, b);
			_mm_store_ps(&ret[i], res);
		}
		for (size_t i = sz; i < m_data.size(); i++)
			ret[i] = m_data[i] / o[i];
#else
		for (size_t i = 0; i < m_data.size(); i++)
			ret[i] = m_data[i] / o[i];
#endif
		return ret;
	}

	Vector<N> operator- () {
		Vector<N> ret;
#ifdef USING_SIMD
		size_t sz = VEC_SIZE(m_data.size());
		size_t inc = FLOAT_SIZE;
		for (size_t i = 0; i < sz; i += inc) {
			__m128 a = _mm_load_ps(&m_data[i]);
			__m128 res = _mm_sub_ps(_mm_set1_ps(0.0f), a);
			_mm_store_ps(&ret[i], res);
		}
		for (size_t i = sz; i < m_data.size(); i++)
			ret[i] = -m_data[i];
#else
		for (size_t i = 0; i < m_data.size(); i++)
			ret[i] = -m_data[i];
#endif
		return ret;
	}

	Vector<N> operator* (float o) {
		Vector<N> ret;
#ifdef USING_SIMD
		size_t sz = VEC_SIZE(m_data.size());
		size_t inc = FLOAT_SIZE;
		for (size_t i = 0; i < sz; i += inc) {
			__m128 a = _mm_load_ps(&m_data[i]);
			__m128 b = _mm_set1_ps(o);
			__m128 res = _mm_mul_ps(a, b);
			_mm_store_ps(&ret[i], res);
		}
		for (size_t i = sz; i < m_data.size(); i++)
			ret[i] = m_data[i] * o;
#else
		for (size_t i = 0; i < m_data.size(); i++)
			ret[i] = m_data[i] * o;
#endif
		return ret;
	}

	Vector<N> operator/ (float o) {
		Vector<N> ret;
#ifdef USING_SIMD
		size_t sz = VEC_SIZE(m_data.size());
		size_t inc = FLOAT_SIZE;
		for (size_t i = 0; i < sz; i += inc) {
			__m128 a = _mm_load_ps(&m_data[i]);
			__m128 b = _mm_set1_ps(o);
			__m128 res = _mm_div_ps(a, b);
			_mm_store_ps(&ret[i], res);
		}
		for (size_t i = sz; i < m_data.size(); i++)
			ret[i] = m_data[i] / o;
#else
		for (size_t i = 0; i < m_data.size(); i++)
			ret[i] = m_data[i] / o;
#endif
		return ret;
	}

	void set(float v) {
		m_data.fill(v);
	}

	void set(const Vector<N>& v) {
#ifdef USING_SIMD
		size_t sz = VEC_SIZE(m_data.size());
		size_t inc = FLOAT_SIZE;
		for (size_t i = 0; i < sz; i += inc) {
			__m128 a = _mm_load_ps(&v.m_data[i]);
			_mm_store_ps(&m_data[i], a);
		}
		for (size_t i = sz; i < m_data.size(); i++)
			m_data[i] = v[i];
#else
		for (int i = 0; i < N; i++)
			m_data[i] = v.m_data[i];
#endif
	}

	float operator[] (uint32_t i) const {
		assert(i < N);
		return m_data[i];
	}
	float& operator[] (uint32_t i) {
		assert(i < N);
		return m_data[i];
	}

	float* data() { return m_data.data(); }

	static const uint32_t SIZE = N;
	static const uint32_t BYTE_SIZE = N * sizeof(float);
private:
	std::array<float, N> m_data;
};

#endif // TWEN_VECTOR_H