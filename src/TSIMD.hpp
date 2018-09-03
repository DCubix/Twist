#ifndef T_SIMD_HPP
#define T_SIMD_HPP

#include <cstddef>
#include <array>
#include "xsimd/xsimd.hpp"

#define FLT_ARR_MAX 8

using Float = xsimd::simd_type<float>;
using FloatArray = std::array<float, FLT_ARR_MAX>;

namespace SIMD {
	static FloatArray add(const FloatArray& a, const FloatArray& b) {
		FloatArray res;
		std::size_t inc = Float::size;
		std::size_t size = FLT_ARR_MAX;
		std::size_t vec_size = size - size % inc;
		
		for (std::size_t i = 0; i < vec_size; i += inc) {
			Float avec = xsimd::load_unaligned(&a[i]);
			Float bvec = xsimd::load_unaligned(&b[i]);
			Float rvec = (avec + bvec);
			xsimd::store_unaligned(&res[i], rvec);
		}

		// Remaining part that cannot be vectorized
		for(std::size_t i = vec_size; i < size; ++i) {
			res[i] = (a[i] + b[i]);
		}

		return res;
	}

	static FloatArray sub(const FloatArray& a, const FloatArray& b) {
		FloatArray res;
		std::size_t inc = Float::size;
		std::size_t size = FLT_ARR_MAX;
		std::size_t vec_size = size - size % inc;
		
		for (std::size_t i = 0; i < vec_size; i += inc) {
			Float avec = xsimd::load_unaligned(&a[i]);
			Float bvec = xsimd::load_unaligned(&b[i]);
			Float rvec = (avec - bvec);
			xsimd::store_unaligned(&res[i], rvec);
		}

		// Remaining part that cannot be vectorized
		for(std::size_t i = vec_size; i < size; ++i) {
			res[i] = (a[i] - b[i]);
		}

		return res;
	}

	static FloatArray mul(const FloatArray& a, const FloatArray& b) {
		FloatArray res;
		std::size_t inc = Float::size;
		std::size_t size = FLT_ARR_MAX;
		std::size_t vec_size = size - size % inc;
		
		for (std::size_t i = 0; i < vec_size; i += inc) {
			Float avec = xsimd::load_unaligned(&a[i]);
			Float bvec = xsimd::load_unaligned(&b[i]);
			Float rvec = (avec * bvec);
			xsimd::store_unaligned(&res[i], rvec);
		}

		// Remaining part that cannot be vectorized
		for(std::size_t i = vec_size; i < size; ++i) {
			res[i] = (a[i] * b[i]);
		}

		return res;
	}

	static FloatArray avg(const FloatArray& a, const FloatArray& b) {
		FloatArray res;
		std::size_t inc = Float::size;
		std::size_t size = FLT_ARR_MAX;
		std::size_t vec_size = size - size % inc;
		
		for (std::size_t i = 0; i < vec_size; i += inc) {
			Float avec = xsimd::load_unaligned(&a[i]);
			Float bvec = xsimd::load_unaligned(&b[i]);
			Float rvec = (avec + bvec) / 2;
			xsimd::store_unaligned(&res[i], rvec);
		}

		// Remaining part that cannot be vectorized
		for(std::size_t i = vec_size; i < size; ++i) {
			res[i] = (a[i] + b[i]) / 2;
		}

		return res;
	}

	static FloatArray neg(const FloatArray& a) {
		FloatArray res;
		std::size_t inc = Float::size;
		std::size_t size = FLT_ARR_MAX;
		std::size_t vec_size = size - size % inc;
		
		for (std::size_t i = 0; i < vec_size; i += inc) {
			Float avec = xsimd::load_unaligned(&a[i]);
			Float rvec = -avec;
			xsimd::store_unaligned(&res[i], rvec);
		}

		// Remaining part that cannot be vectorized
		for(std::size_t i = vec_size; i < size; ++i) {
			res[i] = -a[i];
		}

		return res;
	}

	static FloatArray div(const FloatArray& a, const FloatArray& b) {
		FloatArray res;
		std::size_t inc = Float::size;
		std::size_t size = FLT_ARR_MAX;
		std::size_t vec_size = size - size % inc;
		
		for (std::size_t i = 0; i < vec_size; i += inc) {
			Float avec = xsimd::load_unaligned(&a[i]);
			Float bvec = xsimd::load_unaligned(&b[i]);
			Float rvec = (avec / bvec);
			xsimd::store_unaligned(&res[i], rvec);
		}

		// Remaining part that cannot be vectorized
		for(std::size_t i = vec_size; i < size; ++i) {
			res[i] = (a[i] / b[i]);
		}

		return res;
	}

	static void set(const FloatArray& a, FloatArray& res) {
		std::size_t inc = Float::size;
		std::size_t size = FLT_ARR_MAX;
		std::size_t vec_size = size - size % inc;

		for (std::size_t i = 0; i < vec_size; i += inc) {
			Float avec = xsimd::load_unaligned(&a[i]);
			xsimd::store_unaligned(&res[i], avec);
		}

		// Remaining part that cannot be vectorized
		for(std::size_t i = vec_size; i < size; ++i) {
			res[i] = a[i];
		}
	}

};

#endif // T_SIMD_HPP