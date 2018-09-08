#ifndef TWEN_DICT_H
#define TWEN_DICT_H

#include "Utils.h"
#include "Log.h"

template <typename V>
class Dict {
public:
	Dict() {}

	size_t add(const Str& key, V value) {
		auto pos = std::find(m_keys.begin(), m_keys.end(), key);
		LogAssert(pos == m_keys.end(), "Duplicate key not allowed: ", key);

		m_indices[key] = m_keys.size();
		m_keys.push_back(key);
		m_values.push_back(value);
		return m_indices[key];
	}
	
	void remove(const Str& key) {
		size_t i = indexOf(key);
		LogAssert(i < m_keys.size(), "Key does not exist: ", key);

		m_keys.erase(m_keys.begin() + i);
		m_values.erase(m_values.begin() + i);
		m_indices.erase(key);
	}

	size_t indexOf(const Str& key) {
		if (m_indices.find(key) == m_indices.end()) {
			return m_keys.size();
		}
		return m_indices[key];
	}

	bool has(const Str& key) {
		return indexOf(key) != m_keys.size();
	}

	V& operator[] (const Str& key) {
		size_t i = indexOf(key);
		LogAssert(i < m_keys.size(), "Key does not exist: ", key);

		return m_values[i];
	}

	const V operator[] (const Str& key) const {
		size_t i = indexOf(key);
		LogAssert(i < m_keys.size(), "Key does not exist. ", key);

		return m_values[i];
	}

	Vec<Str> keys() const { return m_keys; }
	Vec<V> values() const { return m_values; }
	size_t size() const { return m_keys.size(); }

private:
	Map<Str, size_t> m_indices;
	Vec<Str> m_keys;
	Vec<V> m_values;
};

#endif // TWEN_DICT_H