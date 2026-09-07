#include <iostream>
#include <vector>
#include <cassert>
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include "min_heap.hpp"

// Helper: Verify that a vector satisfies the binary min-heap invariant
template <typename T>
bool isMinHeap(const std::vector<T>& v) {
    size_t n = v.size();
    for (size_t i = 0; i < n; ++i) {
        size_t left = 2 * i + 1;
        size_t right = 2 * i + 2;
        if (left < n && v[left] < v[i]) return false;
        if (right < n && v[right] < v[i]) return false;
    }
    return true;
}

// Helper: Verify multiset equality (same elements with same frequencies)
template <typename T>
bool multisetEqual(std::vector<T> a, std::vector<T> b) {
    if (a.size() != b.size()) return false;
    std::sort(a.begin(), a.end());
    std::sort(b.begin(), b.end());
    return a == b;
}

void test_empty_heap() {
    std::cout << "[TEST] Empty heap..." << std::endl;
    MinHeap<double> h;
    assert(h.isEmpty());
    assert(h.empty());
    assert(h.size() == 0);
    assert(h.toVector().empty());
    assert(h.data().empty());

    bool threw = false;
    try {
        h.getMin();
    } catch (const std::underflow_error&) {
        threw = true;
    }
    assert(threw && "getMin() on empty heap must throw underflow_error");

    threw = false;
    try {
        h.extractMin();
    } catch (const std::underflow_error&) {
        threw = true;
    }
    assert(threw && "extractMin() on empty heap must throw underflow_error");

    std::vector<double> sorted = MinHeap<double>::heapSort({});
    assert(sorted.empty());
    std::cout << "  -> PASSED" << std::endl;
}

void test_singleton_heap() {
    std::cout << "[TEST] Singleton heap..." << std::endl;
    MinHeap<double> h({42.5});
    assert(!h.isEmpty());
    assert(h.size() == 1);
    assert(h.getMin() == 42.5);
    assert(isMinHeap(h.data()));

    double minVal = h.extractMin();
    assert(minVal == 42.5);
    assert(h.isEmpty());
    assert(h.size() == 0);

    std::vector<double> sorted = MinHeap<double>::heapSort({-17.2});
    assert(sorted.size() == 1 && sorted[0] == -17.2);
    std::cout << "  -> PASSED" << std::endl;
}

void test_heapify_arbitrary_array() {
    std::cout << "[TEST] Construction from arbitrary array (heapify)..." << std::endl;
    std::vector<double> input = {9.0, 3.5, 7.2, 1.1, 8.4, 2.0, 5.5, 0.5, 6.3};
    MinHeap<double> h(input);

    assert(h.size() == input.size());
    assert(isMinHeap(h.data()));
    assert(multisetEqual(h.data(), input));
    assert(h.getMin() == 0.5);
    std::cout << "  -> PASSED" << std::endl;
}

void test_insert_preserves_invariant() {
    std::cout << "[TEST] Insertion preserves min-heap invariant..." << std::endl;
    MinHeap<int> h;
    std::vector<int> values = {10, 4, 15, 20, 0, 8, -5, 3};
    for (int v : values) {
        h.insert(v);
        assert(isMinHeap(h.data()));
    }
    assert(h.size() == values.size());
    assert(h.getMin() == -5);
    std::cout << "  -> PASSED" << std::endl;
}

void test_extract_min_preserves_invariant_and_sorts() {
    std::cout << "[TEST] Extract-min preserves invariant and yields nondecreasing order..." << std::endl;
    std::vector<double> input = {5.5, -2.1, 100.0, 0.0, -10.5, 3.3, -2.1, 7.7, 0.0};
    MinHeap<double> h(input);

    std::vector<double> extracted;
    while (!h.isEmpty()) {
        extracted.push_back(h.extractMin());
        assert(isMinHeap(h.data()));
    }

    assert(extracted.size() == input.size());
    assert(multisetEqual(extracted, input));
    for (size_t i = 1; i < extracted.size(); ++i) {
        assert(extracted[i - 1] <= extracted[i]);
    }
    std::cout << "  -> PASSED" << std::endl;
}

void test_heap_sort() {
    std::cout << "[TEST] Heap sort..." << std::endl;
    std::vector<double> input = {3.0, 1.0, 2.0};
    std::vector<double> sorted = MinHeap<double>::heapSort(input);
    std::vector<double> expected = {1.0, 2.0, 3.0};
    assert(sorted == expected);

    // Negative, fractional, duplicates, all-equal
    std::vector<double> mixed = {-3.5, 2.1, -3.5, 0.0, 10.0, -100.25, 0.0};
    std::vector<double> sortedMixed = MinHeap<double>::heapSort(mixed);
    assert(multisetEqual(sortedMixed, mixed));
    for (size_t i = 1; i < sortedMixed.size(); ++i) {
        assert(sortedMixed[i - 1] <= sortedMixed[i]);
    }

    std::vector<int> allEqual = {7, 7, 7, 7, 7};
    std::vector<int> sortedEqual = MinHeap<int>::heapSort(allEqual);
    assert(sortedEqual == allEqual);
    std::cout << "  -> PASSED" << std::endl;
}

void test_duplicates_and_all_equal() {
    std::cout << "[TEST] Duplicates and all-equal values..." << std::endl;
    std::vector<int> input = {5, 5, 2, 2, 8, 8, 2};
    MinHeap<int> h(input);
    assert(isMinHeap(h.data()));
    assert(multisetEqual(h.data(), input));
    assert(h.getMin() == 2);
    std::cout << "  -> PASSED" << std::endl;
}

void test_decrease_key() {
    std::cout << "[TEST] decreaseKey..." << std::endl;
    MinHeap<int> h({10, 20, 30, 40, 50});
    assert(isMinHeap(h.data()));

    // Find index of 50 and decrease to 5
    size_t idx50 = 0;
    for (size_t i = 0; i < h.size(); ++i) {
        if (h.data()[i] == 50) { idx50 = i; break; }
    }
    h.decreaseKey(idx50, 5);
    assert(isMinHeap(h.data()));
    assert(h.getMin() == 5);

    // Out of bounds exception
    bool threw = false;
    try {
        h.decreaseKey(100, 1);
    } catch (const std::out_of_range&) {
        threw = true;
    }
    assert(threw && "decreaseKey with out of bounds index must throw out_of_range");

    // Increasing value exception
    threw = false;
    try {
        h.decreaseKey(0, 999);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw && "decreaseKey with larger value must throw invalid_argument");
    std::cout << "  -> PASSED" << std::endl;
}

void test_delete_key() {
    std::cout << "[TEST] deleteKey..." << std::endl;
    MinHeap<int> h({15, 10, 20, 8, 12, 25, 30});
    assert(isMinHeap(h.data()));

    // Delete an interior element
    h.deleteKey(12);
    assert(isMinHeap(h.data()));
    assert(h.size() == 6);

    // Delete the root
    h.deleteKey(h.getMin());
    assert(isMinHeap(h.data()));
    assert(h.size() == 5);

    // Delete the last element
    int lastVal = h.data().back();
    h.deleteKey(lastVal);
    assert(isMinHeap(h.data()));
    assert(h.size() == 4);

    // Key not found exception
    bool threw = false;
    try {
        h.deleteKey(99999);
    } catch (const std::runtime_error&) {
        threw = true;
    }
    assert(threw && "deleteKey with nonexistent key must throw runtime_error");
    std::cout << "  -> PASSED" << std::endl;
}

void test_binary_heap_alias() {
    std::cout << "[TEST] BinaryHeap backward compatibility alias..." << std::endl;
    BinaryHeap<double> bh({3.0, 1.0, 2.0});
    assert(bh.getMin() == 1.0);
    assert(bh.extractMin() == 1.0);
    assert(bh.extractMin() == 2.0);
    assert(bh.extractMin() == 3.0);
    assert(bh.isEmpty());
    std::cout << "  -> PASSED" << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Running Binary Min-Heap Acceptance Tests" << std::endl;
    std::cout << "========================================" << std::endl;

    test_empty_heap();
    test_singleton_heap();
    test_heapify_arbitrary_array();
    test_insert_preserves_invariant();
    test_extract_min_preserves_invariant_and_sorts();
    test_heap_sort();
    test_duplicates_and_all_equal();
    test_decrease_key();
    test_delete_key();
    test_binary_heap_alias();

    std::cout << "========================================" << std::endl;
    std::cout << "All Binary Min-Heap tests PASSED successfully!" << std::endl;
    std::cout << "========================================" << std::endl;
    return 0;
}
