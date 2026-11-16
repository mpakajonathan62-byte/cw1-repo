#ifndef SORTING_H
#define SORTING_H

#include <cstddef>
#include <vector>

// Stable insertion sort. Compare(a, b) must return true when a belongs before b.
template <typename T, typename Compare>
void insertionSort(std::vector<T>& values, Compare compare) {
    for (std::size_t i = 1; i < values.size(); ++i) {
        T key = values[i];
        std::size_t j = i;

        // Shift only elements strictly after key. Equal elements are not crossed,
        // which preserves their original relative order (stability).
        while (j > 0 && compare(key, values[j - 1])) {
            values[j] = values[j - 1];
            --j;
        }
        values[j] = key;
    }
}

namespace sorting_detail {
template <typename T, typename Compare>
void mergeRange(std::vector<T>& values, std::vector<T>& buffer,
                std::size_t first, std::size_t middle,
                std::size_t last, Compare compare) {
    std::size_t left = first;
    std::size_t right = middle;
    std::size_t out = first;

    while (left < middle && right < last) {
        // Choose from the left half when keys compare equal. This is what
        // makes merge sort stable.
        if (compare(values[right], values[left])) {
            buffer[out++] = values[right++];
        } else {
            buffer[out++] = values[left++];
        }
    }
    while (left < middle) buffer[out++] = values[left++];
    while (right < last) buffer[out++] = values[right++];

    for (std::size_t i = first; i < last; ++i) values[i] = buffer[i];
}

template <typename T, typename Compare>
void mergeSortRange(std::vector<T>& values, std::vector<T>& buffer,
                    std::size_t first, std::size_t last, Compare compare) {
    if (last - first <= 1) return;
    const std::size_t middle = first + (last - first) / 2;
    mergeSortRange(values, buffer, first, middle, compare);
    mergeSortRange(values, buffer, middle, last, compare);
    mergeRange(values, buffer, first, middle, last, compare);
}
} // namespace sorting_detail

// Stable merge sort: O(n log n) time in best, average and worst cases;
// O(n) additional storage.
template <typename T, typename Compare>
void mergeSort(std::vector<T>& values, Compare compare) {
    if (values.size() < 2) return;
    std::vector<T> buffer(values);
    sorting_detail::mergeSortRange(values, buffer, 0, values.size(), compare);
}

// Iterative binary search. The vector must already be sorted using the same
// ordering represented by compare. Returns the matching index, or -1.
template <typename T, typename Key, typename KeyOf, typename Compare>
std::ptrdiff_t binarySearch(const std::vector<T>& values, const Key& key,
                            KeyOf keyOf, Compare compare) {
    std::size_t low = 0;
    std::size_t high = values.size(); // half-open interval [low, high)

    while (low < high) {
        const std::size_t middle = low + (high - low) / 2;
        const auto& middleKey = keyOf(values[middle]);

        if (compare(middleKey, key)) {
            low = middle + 1;
        } else if (compare(key, middleKey)) {
            high = middle;
        } else {
            return static_cast<std::ptrdiff_t>(middle);
        }
    }
    return -1;
}

#endif
