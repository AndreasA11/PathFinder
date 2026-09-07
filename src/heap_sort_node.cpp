#ifndef MIN_HEAP_HPP
#define MIN_HEAP_HPP

#include <iostream>
#include <vector>
// *CHANGES*: Included <stdexcept> for std::runtime_error, std::underflow_error, std::invalid_argument, and std::out_of_range.
#include <stdexcept>
// *CHANGES*: Included <utility> for std::swap.
#include <utility>
// *CHANGES*: Included <cstddef> for size_t.
#include <cstddef>
#include <initializer_list>

// *CHANGES*: Renamed class from BinaryHeap to MinHeap to match prompt specification (promts/Task_ Binary Min-Heap Primitive.md).
// Provided a template alias 'BinaryHeap' below for backward compatibility.
template <typename T>
class MinHeap {
    private:
        std::vector<T> heap;
        // *CHANGES*: Initialized currentSize to 0. Maintained for compatibility, although heap.size() is primary.
        int currentSize{0};

        // Helper functions
        // *CHANGES*: Changed index parameters from int to size_t to prevent signed/unsigned comparison warnings
        // and ensure memory safety with large vectors.

        // Returns parent index for a given node index
        static constexpr size_t parent(size_t index) noexcept {
            return (index - 1) / 2;
        }

        // Returns left child index (2*i + 1)
        static constexpr size_t leftChild(size_t index) noexcept {
            return (index * 2) + 1;
        }

        // Returns right child index (2*i + 2)
        static constexpr size_t rightChild(size_t index) noexcept {
            return (index * 2) + 2;
        }

        /*
        1. While the key at index i is less than its parent and i is not the root:
        2. Swap the key with its parent.
        3. Update i to the parent index.
        4. Continue this process until the heap property is restored.
        */
        void heapifyUp(size_t index) {
            // *CHANGES*: Condition updated to 'index > 0' instead of 'index' to be explicit and safe with unsigned size_t.
            while (index > 0 && heap[index] < heap[parent(index)]) {
                std::swap(heap[index], heap[parent(index)]);
                index = parent(index);
            }
        }

        /*
        1. Initialize the smallest node as the current node, i.
        2. Calculate the left child index as 2i+1.
        3. Calculate the right child index as 2i+2.
        4. If the left child exists (i.e., it is within the array bounds) and is less than the current smallest node, update the smallest node to the left child.
        5. If the right child exists (i.e., it is within the array bounds) and is less than the current smallest node, update the smallest node to the right child.
        6. If the smallest node is not the current node, i, swap the current node with the smallest node using the std::swap function.
        7. Recursively apply the heapify operation to the subtree rooted at the smallest node.
        */
        void heapifyDown(size_t index) {
            // *CHANGES*: Added bounds check to safely handle empty heap or out-of-bounds index calls.
            if (index >= heap.size()) {
                return;
            }

            size_t left = leftChild(index);
            size_t right = rightChild(index);
            size_t smallest = index;

            // *CHANGES*: Use size_t comparisons to eliminate compiler warnings.
            if (left < heap.size() && heap[left] < heap[smallest]) {
                smallest = left;
            } 
            if (right < heap.size() && heap[right] < heap[smallest]) {
                smallest = right;
            }
            if (smallest != index) {
                std::swap(heap[index], heap[smallest]);
                heapifyDown(smallest);
            }
        }

    public:
        // ==========================================
        // Constructors
        // ==========================================

        // *CHANGES*: Explicit default constructor initializing an empty heap.
        MinHeap() : heap(), currentSize(0) {}

        // *CHANGES*: Added constructor to build heap from an arbitrary vector in O(n) time using bottom-up heapify.
        // Required by Task_ Binary Min-Heap Primitive.md.
        explicit MinHeap(const std::vector<T>& values) : heap(values), currentSize(static_cast<int>(values.size())) {
            heapify();
        }

        // *CHANGES*: Added initializer list constructor for convenient instantiation (e.g. MinHeap<int> h = {3, 1, 2}).
        MinHeap(std::initializer_list<T> init) : heap(init), currentSize(static_cast<int>(init.size())) {
            heapify();
        }

        // ==========================================
        // Public interface functions
        // ==========================================

        // *CHANGES*: Added size() method. This was previously missing, causing a compiler error in deleteKey().
        size_t size() const noexcept {
            return heap.size();
        }

        // Checks if the heap is empty
        // *CHANGES*: Marked const noexcept.
        bool isEmpty() const noexcept {
            return heap.empty();
        }

        // *CHANGES*: Added STL-idiomatic empty() method.
        bool empty() const noexcept {
            return heap.empty();
        }

        // *CHANGES*: Added data() and toVector() methods to inspect and export the heap as an array/vector,
        // directly satisfying the requirement: "inspecting/exporting the heap contents as an array".
        const std::vector<T>& data() const noexcept {
            return heap;
        }

        std::vector<T> toVector() const {
            return heap;
        }

        // Transforms arbitrary contents in `heap` into a valid binary min-heap using Floyd's bottom-up algorithm in O(n).
        // *CHANGES*: Implemented heapify() method. It sifts down non-leaf nodes starting from the last parent down to index 0.
        void heapify() {
            if (heap.size() <= 1) {
                return;
            }
            // The last parent node is at parent(heap.size() - 1) = (heap.size() - 2) / 2.
            // Using size_t loop with > 0 guard to avoid underflow.
            for (size_t i = (heap.size() - 2) / 2 + 1; i > 0; --i) {
                heapifyDown(i - 1);
            }
        }

        // Inserts a new element into the heap
        /*
        1. Append the new key to the end of the array.
        2. Initialize the index i as the index of the newly inserted key (last element of the array).
        3. Call heapifyUp
        */
        // *CHANGES*: Passed key by const reference (const T&) to avoid unnecessary object copies.
        void insert(const T& key) {
            heap.push_back(key);
            currentSize = static_cast<int>(heap.size());
            size_t index = heap.size() - 1;
            heapifyUp(index);
        }

        // Removes and returns the minimum element (min-heap root)
        /*
        1. Check if the heap is empty. If true, throw an underflow_error.
        2. If the heap has only one element, remove and return that element.
        3. Store the minimum value (root of the heap) in a temporary variable.
        4. Replace the root of the heap with the last element in the array.
        5. Remove the last element from the array.
        6. Apply the heapify operation on the root to restore the heap property.
        7. Return the stored minimum value.
        */
        // *CHANGES*:
        // 1. Threw std::underflow_error (with std:: prefix) instead of unqualified runtime_error.
        // 2. Added early return for single-element heap (step 2 in comments) to avoid unnecessary/invalid sift on empty container.
        // 3. Properly updated currentSize.
        T extractMin() {
            if (isEmpty()) {
                throw std::underflow_error("Heap is empty");
            }

            T root = heap.front();

            // Single element case: pop and return directly without sifting down
            if (heap.size() == 1) {
                heap.pop_back();
                currentSize = 0;
                return root;
            }

            // Move last element to root and remove the last position
            heap[0] = heap.back();
            heap.pop_back();
            currentSize = static_cast<int>(heap.size());

            // Restore min-heap invariant from the root
            heapifyDown(0);
            return root;
        }

        // Returns the minimum element without removing it
        // *CHANGES*:
        // 1. Threw std::underflow_error (with std:: prefix) instead of unqualified runtime_error.
        // 2. Returned const T& instead of copy, and marked method const.
        const T& getMin() const {
            if (isEmpty()) {
                throw std::underflow_error("Heap is empty");
            }
            return heap[0];
        }

        // *CHANGES*: Added peek() alias for getMin().
        const T& peek() const {
            return getMin();
        }

        /*
        1. Check if the index i is valid (within the array bounds). If not, throw an out_of_range exception.
        2. Replace the key at index i with the last element in the array.
        3. Remove the last element from the array.
        4. Apply the heapify operation on the subtree rooted at index i to restore the heap property.
        */
        // *CHANGES*:
        // 1. Fixed undefined size() call by using heap.size().
        // 2. Qualified std::runtime_error.
        // 3. Handled edge case where key is the last element (avoid out-of-bounds indexing in heapifyUp/Down).
        // 4. Properly updated currentSize.
        void deleteKey(const T& key) {
            int index = -1;
            // Find the index of the node to delete
            for (size_t i = 0; i < heap.size(); ++i) {
                if (heap[i] == key) {
                    index = static_cast<int>(i);
                    break;
                }
            }

            if (index == -1) {
                throw std::runtime_error("Key not found in the heap");
            }

            // If the element to delete is already the last element, simply pop it
            if (static_cast<size_t>(index) == heap.size() - 1) {
                heap.pop_back();
                currentSize = static_cast<int>(heap.size());
                return;
            }

            // Replace node with the last element
            heap[index] = heap.back();
            heap.pop_back();
            currentSize = static_cast<int>(heap.size());

            // Restore heap property: key could need to move up or down
            heapifyUp(static_cast<size_t>(index));
            heapifyDown(static_cast<size_t>(index));
        }

        /*
        1. Check if the index i is valid and the new value is less than the current value. If not, throw an invalid_argument exception.
        2. Update the value at index i to the new value.
        3. Call heapifyUp
        */
        // *CHANGES*:
        // 1. Changed index type to size_t to prevent signedness warnings.
        // 2. Threw std::out_of_range for invalid index.
        // 3. Validated that newValue <= heap[i] and threw std::invalid_argument if violated (as described in step 1 of comment).
        void decreaseKey(size_t i, const T& newValue) {
            if (i >= heap.size()) {
                throw std::out_of_range("Index out of bounds");
            }
            if (heap[i] < newValue) {
                throw std::invalid_argument("New value must be less than or equal to current value in decreaseKey");
            }
            heap[i] = newValue;
            heapifyUp(i);
        }

        // Prints the heap elements
        // *CHANGES*:
        // 1. Fixed syntax error 'for(const &T elt: heap)' to 'for(const T& elt: heap)'.
        // 2. Marked method const.
        void printHeap() const {
            for (const T& elt : heap) {
                std::cout << elt << " \n";  
            }
        }

        // ==========================================
        // Heap-Sort Support
        // ==========================================

        // *CHANGES*: Implemented heapSort static method as required by Task_ Binary Min-Heap Primitive.md.
        // Transforms the input into a min-heap and repeatedly extracts the minimum to sort in ascending order.
        // Built strictly on heap operations without standard library sorting algorithms.
        static std::vector<T> heapSort(const std::vector<T>& input) {
            MinHeap<T> h(input);
            std::vector<T> sorted;
            sorted.reserve(input.size());
            while (!h.isEmpty()) {
                sorted.push_back(h.extractMin());
            }
            return sorted;
        }

        // *CHANGES*: Added instance sort() method producing an ascending sorted copy of the heap elements.
        std::vector<T> sort() const {
            MinHeap<T> copy = *this;
            std::vector<T> sorted;
            sorted.reserve(copy.size());
            while (!copy.isEmpty()) {
                sorted.push_back(copy.extractMin());
            }
            return sorted;
        }
};

// *CHANGES*: Provided template alias 'BinaryHeap' for backward compatibility.
template <typename T>
using BinaryHeap = MinHeap<T>;

#endif // MIN_HEAP_HPP
