
/**
 * @file small_map.h
 * @brief Provides a simple sorted map implementation for small key-value sets.
 */
#ifndef ISTD_UTIL_SMALL_MAP_H
#define ISTD_UTIL_SMALL_MAP_H

#include <algorithm>
#include <stdexcept>
#include <vector>

namespace istd {

/**
 * @brief A simple sorted map for small key-value sets.
 *
 * Stores entries in a sorted vector and provides basic map operations.
 *
 * @tparam T_Key Type of the key.
 * @tparam T_Value Type of the value.
 */
template<typename T_Key, typename T_Value>
requires std::totally_ordered<T_Key>
class SmallMap {
	/**
	 * @brief Internal entry structure for key-value pairs.
	 */
	struct Entry {
		T_Key key;     ///< Key of the entry
		T_Value value; ///< Value of the entry

		/**
		 * @brief Comparison operator for sorting entries by key.
		 * @param other_key Key to compare with.
		 * @return True if this entry's key is less than other_key.
		 */
		bool operator<(const T_Key &other_key) const {
			return key < other_key;
		}
	};

	std::vector<Entry> entries_; ///< Container for all entries

public:
	/**
	 * @brief Default constructor.
	 */
	SmallMap() = default;

	/**
	 * @brief Inserts a new key-value pair.
	 * @param key Key to insert.
	 * @param value Value to associate with the key.
	 * @throws std::invalid_argument if the key already exists.
	 */
	void insert(const T_Key &key, const T_Value &value) {
		// Binary search for existing key
		auto it = std::lower_bound(entries_.begin(), entries_.end(), key);

		if (it != entries_.end() && it->key == key) {
			// Key exists, but it shouldn't
			throw std::invalid_argument("Key already exists in SmallMap");
		}

		// Insert new entry in sorted order
		entries_.insert(it, {key, value});
	}

	/**
	 * @brief Accesses the value associated with a key.
	 * @tparam Self Type of the SmallMap instance.
	 * @param key Key to look up.
	 * @return Reference to the value associated with the key.
	 * @throws std::out_of_range if the key does not exist.
	 */
	template<typename Self>
	auto &&operator[](this Self &&self, const T_Key &key) {
		auto it = std::lower_bound(
			self.entries_.begin(), self.entries_.end(), key
		);

		if (it != self.entries_.end() && it->key == key) {
			return it->value; // Return existing value
		}
		throw std::out_of_range("Key not found in SmallMap");
	}

	/**
	 * @brief Removes all entries from the map.
	 */
	void clear() noexcept {
		entries_.clear();
	}

	/**
	 * @brief Returns the number of entries in the map.
	 * @return Number of key-value pairs.
	 */
	size_t size() const noexcept {
		return entries_.size();
	}

	/**
	 * @brief Checks if the map is empty.
	 * @return True if the map contains no entries.
	 */
	bool empty() const noexcept {
		return entries_.empty();
	}

	/**
	 * @brief Removes the entry with the specified key.
	 * @param key Key to remove.
	 * @throws std::out_of_range if the key does not exist.
	 */
	void erase(const T_Key &key) {
		auto it = std::lower_bound(entries_.begin(), entries_.end(), key);
		if (it != entries_.end() && it->key == key) {
			entries_.erase(it); // Remove the entry
		} else {
			throw std::out_of_range("Key not found in SmallMap");
		}
	}

	/**
	 * @brief Returns an iterator to the beginning of the entries.
	 * @return Iterator to the first entry.
	 */
	auto begin() noexcept {
		return entries_.begin();
	}

	/**
	 * @brief Returns an iterator to the end of the entries.
	 * @return Iterator to one past the last entry.
	 */
	auto end() noexcept {
		return entries_.end();
	}

	/**
	 * @brief Returns a const iterator to the beginning of the entries.
	 * @return Const iterator to the first entry.
	 */
	auto cbegin() const noexcept {
		return entries_.cbegin();
	}

	/**
	 * @brief Returns a const iterator to the end of the entries.
	 * @return Const iterator to one past the last entry.
	 */
	auto cend() const noexcept {
		return entries_.cend();
	}
};

} // namespace istd

#endif