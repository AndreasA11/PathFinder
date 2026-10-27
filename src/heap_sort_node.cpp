#ifndef MIN_HEAP_HPP
#define MIN_HEAP_HPP

#include <iostream>
#include <vector>
#include <stdexcept>
#include <utility>
#include <cstddef>
#include <initializer_list>

// Provided a template alias 'BinaryHeap' below for backward compatibility.
template <typename T>
class MinHeap {
    private:
        std::vector<T> heap;
        // *CHANGES*: Initialized currentSize to 0. Maintained for compatibility, although heap.size() is primary.
        int currentSize{0};

        // ==========================================
        // Helper functions
        // ==========================================
        
        /*
                    4
            6               5
        9       12      8       13

        4 6 5 9 12 8 13
        */

        /*
        1. For an element at index i, compute its parent index using integer division: (i - 1) / 2.
        2. Return the calculated parent index.
        */
        // *CHANGES*: Changed index parameters from int to size_t to prevent signed/unsigned comparison warnings
        // and ensure memory safety with large vectors.
        static constexpr size_t parent(size_t index) noexcept {
            // Associated with Step 1 & 2: Calculate and return parent index
            return (index - 1) / 2;
        }

        /*
        1. For an element at index i, compute its left child index as: 2 * i + 1.
        2. Return the calculated left child index.
        */
        // *CHANGES*: Changed index parameter from int to size_t.
        static constexpr size_t leftChild(size_t index) noexcept {
            // Associated with Step 1 & 2: Calculate and return left child index
            return (index * 2) + 1;
        }

        /*
        1. For an element at index i, compute its right child index as: 2 * i + 2.
        2. Return the calculated right child index.
        */
        // *CHANGES*: Changed index parameter from int to size_t.
        static constexpr size_t rightChild(size_t index) noexcept {
            // Associated with Step 1 & 2: Calculate and return right child index
            return (index * 2) + 2;
        }

        /*
        1. While the key at index i is less than its parent and i is not the root:
        2. Swap the key with its parent.
        3. Update i to the parent index.
        4. Continue this process until the heap property is restored.
        */
        void heapifyUp(size_t index) {
            // Associated with Step 1: Check loop condition: index is not root (index > 0)
            // and key at index i is less than its parent's key.
            // *CHANGES*: Condition updated to 'index > 0' instead of 'index' to be explicit and safe with unsigned size_t.
            while (index > 0 && heap[index] < heap[parent(index)]) {
                // Associated with Step 2: Swap the key with its parent
                std::swap(heap[index], heap[parent(index)]);
                // Associated with Step 3: Update i to the parent index
                index = parent(index);
                // Associated with Step 4: Continue loop until condition in Step 1 becomes false
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

            // Associated with Step 2: Calculate the left child index as 2i+1
            size_t left = leftChild(index);
            // Associated with Step 3: Calculate the right child index as 2i+2
            size_t right = rightChild(index);
            // Associated with Step 1: Initialize the smallest node as the current node, i
            size_t smallest = index;

            // Associated with Step 4: If left child exists and is less than current smallest, update smallest
            // *CHANGES*: Use size_t comparisons to eliminate compiler warnings.
            if (left < heap.size() && heap[left] < heap[smallest]) {
                smallest = left;
            } 
            // Associated with Step 5: If right child exists and is less than current smallest, update smallest
            if (right < heap.size() && heap[right] < heap[smallest]) {
                smallest = right;
            }
            // Associated with Step 6 & 7: If smallest node is not current node i, swap and recursively heapify
            if (smallest != index) {
                // Associated with Step 6: Swap current node with smallest node using std::swap
                std::swap(heap[index], heap[smallest]);
                // Associated with Step 7: Recursively apply heapify operation to subtree rooted at smallest node
                heapifyDown(smallest);
            }
        }

    public:
        // ==========================================
        // Constructors
        // ==========================================

        /*
        1. Initialize backing storage vector to empty.
        2. Initialize currentSize counter to 0.
        */
        // *CHANGES*: Explicit default constructor initializing an empty heap.
        MinHeap() : heap(), currentSize(0) {
        }

        /*
        1. Copy the input vector into the internal backing container.
        2. Initialize currentSize to the number of elements in the vector.
        3. Call bottom-up heapify() to transform the container into a valid min-heap in O(n) time.
        */
        // *CHANGES*: Added constructor to build heap from an arbitrary vector in O(n) time using bottom-up heapify.
        // Required by Task_ Binary Min-Heap Primitive.md.
        explicit MinHeap(const std::vector<T>& values) : heap(values), currentSize(static_cast<int>(values.size())) {
            heapify();
        }

        /*
        1. Initialize internal container with the initializer list elements.
        2. Initialize currentSize to the size of the initializer list.
        3. Call bottom-up heapify() to transform into a valid min-heap in O(n) time.
        */
        // *CHANGES*: Added initializer list constructor for convenient instantiation (e.g. MinHeap<int> h = {3, 1, 2}).
        MinHeap(std::initializer_list<T> init) : heap(init), currentSize(static_cast<int>(init.size())) {
            // Associated with Step 1 & 2: Copy initializer list into heap and set currentSize
            // Associated with Step 3: Transform into a valid binary min-heap
            heapify();
        }

        // ==========================================
        // Public interface functions
        // ==========================================

        /*
        1. Query the size of the underlying container.
        2. Return the number of elements as size_t.
        */
        // *CHANGES*: Added size() method. This was previously missing, causing a compiler error in deleteKey().
        size_t size() const noexcept {
            return heap.size();
        }

        /*
        1. Check whether the underlying container contains 0 elements.
        2. Return true if empty, false otherwise.
        */
        // Checks if the heap is empty
        // *CHANGES*: Marked const noexcept.
        bool isEmpty() const noexcept {
            return heap.empty();
        }

        /*
        1. STL-standard method to check whether the heap is empty.
        2. Return true if empty, false otherwise.
        */
        // *CHANGES*: Added STL-idiomatic empty() method.
        bool empty() const noexcept {
            return heap.empty();
        }

        /*
        1. Provide direct read-only access to the underlying vector container.
        2. Return a const reference to the internal heap vector.
        */
        // *CHANGES*: Added data() method to inspect heap contents as an array
        // as required by Task_ Binary Min-Heap Primitive.md.
        const std::vector<T>& data() const noexcept {
            return heap;
        }

        /*
        1. Create a copy of the underlying heap array.
        2. Return the vector copy for external inspection or serialization.
        */
        // *CHANGES*: Added toVector() method to export heap contents as an array.
        std::vector<T> toVector() const {
            return heap;
        }

        /*
        1. Check if the heap has 1 or fewer elements; if so, it already satisfies the heap invariant, so return immediately.
        2. Identify the index of the last non-leaf parent node: (heap.size() - 2) / 2.
        3. Iterate backwards from the last parent node down to the root (index 0).
        4. On each iteration, call heapifyDown on the current node to restore the subtree min-heap invariant.
        */
        // Transforms arbitrary contents in `heap` into a valid binary min-heap using Floyd's bottom-up algorithm in O(n).
        // *CHANGES*: Implemented heapify() method. It sifts down non-leaf nodes starting from the last parent down to index 0.
        void heapify() {
            if (heap.size() <= 1) {
                return;
            }
            for (size_t i = (heap.size() - 2) / 2 + 1; i > 0; --i) {
                heapifyDown(i - 1);
            }
        }

        /*
        1. Append the new key to the end of the array.
        2. Initialize the index i as the index of the newly inserted key (last element of the array).
        3. Call heapifyUp on index i to restore the min-heap invariant.
        */
        // Inserts a new element into the heap
        // *CHANGES*: Passed key by const reference (const T&) to avoid unnecessary object copies.
        void insert(const T& key) {
            // Associated with Step 1: Append new key to the end of the vector
            heap.push_back(key);
            // *CHANGES*: Keep currentSize synchronized with container size
            currentSize = static_cast<int>(heap.size());
            // Associated with Step 2: Initialize index as the newly inserted key's position
            size_t index = heap.size() - 1;
            // Associated with Step 3: Call heapifyUp on the new element
            heapifyUp(index);
        }

        /*
        1. Check if the heap is empty. If true, throw an underflow_error.
        2. If the heap has only one element, remove and return that element.
        3. Store the minimum value (root of the heap) in a temporary variable.
        4. Replace the root of the heap with the last element in the array.
        5. Remove the last element from the array.
        6. Apply the heapify operation on the root to restore the heap property.
        7. Return the stored minimum value.
        */
        // Removes and returns the minimum element (min-heap root)
        // *CHANGES*:
        // 1. Threw std::underflow_error (with std:: prefix) instead of unqualified runtime_error.
        // 2. Added early return for single-element heap (step 2 in comments) to avoid unnecessary/invalid sift on empty container.
        // 3. Properly updated currentSize.
        T extractMin() {
            if (isEmpty()) {
                throw std::underflow_error("Heap is empty");
            }

            T root = heap.front();

            if (heap.size() == 1) {
                heap.pop_back();
                currentSize = 0;
                return root;
            }

            heap[0] = heap.back();
            heap.pop_back();
            currentSize = static_cast<int>(heap.size());

            heapifyDown(0);

            return root;
        }

        /*
        1. Check if the heap is empty. If true, throw an underflow_error.
        2. Access and return the root element at index 0 (the minimum in a min-heap).
        */
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

        /*
        1. Forward to getMin() to inspect the root minimum element without removing it.
        */
        // *CHANGES*: Added peek() alias for getMin().
        const T& peek() const {
            // Associated with Step 1: Forward call to getMin()
            return getMin();
        }

        /*
        1. Search the heap array for the first occurrence of key; if not found, throw runtime_error.
        2. If the node to delete is already the last element, remove it and return.
        3. Replace the key at index i with the last element in the array.
        4. Remove the last element from the array.
        5. Apply heapifyUp and heapifyDown on index i to restore the heap property.
        */
        // *CHANGES*:
        // 1. Fixed undefined size() call by using heap.size().
        // 2. Qualified std::runtime_error.
        // 3. Handled edge case where key is the last element (avoid out-of-bounds indexing in heapifyUp/Down).
        // 4. Properly updated currentSize.
        void deleteKey(const T& key) {
            int index = -1;
            // Associated with Step 1: Find the index of the node to delete
            for (size_t i = 0; i < heap.size(); ++i) {
                if (heap[i] == key) {
                    index = static_cast<int>(i);
                    break;
                }
            }

            if (index == -1) {
                throw std::runtime_error("Key not found in the heap");
            }

            // *CHANGES*: Prevents out-of-bounds sift operations when deleting the last element.
            if (static_cast<size_t>(index) == heap.size() - 1) {
                heap.pop_back();
                currentSize = static_cast<int>(heap.size());
                return;
            }

            heap[index] = heap.back();
            heap.pop_back();
            currentSize = static_cast<int>(heap.size());

            heapifyUp(static_cast<size_t>(index));
            heapifyDown(static_cast<size_t>(index));
        }

        /*
        1. Check if the index i is valid (within the array bounds). If not, throw an out_of_range exception.
        2. Check if the new value is less than or equal to the current value. If not, throw an invalid_argument exception.
        3. Update the value at index i to the new value.
        4. Call heapifyUp on index i to restore the min-heap property.
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

        /*
        1. Iterate through all elements in the heap array from index 0 to size - 1.
        2. Print each element to std::cout followed by a newline.
        */
        // Prints the heap elements
        // *CHANGES*:
        // 1. Fixed syntax error 'for(const &T elt: heap)' to 'for(const T& elt: heap)'.
        // 2. Marked method const.
        void printHeap() const {
            // Associated with Step 1 & 2: Iterate and print each element
            for (const T& elt : heap) {
                std::cout << elt << " \n";  
            }
        }

        // ==========================================
        // Heap-Sort Support
        // ==========================================

        /*
        1. Construct a min-heap from the input vector using the O(n) heapify constructor.
        2. Allocate a result vector reserving space for input.size() elements.
        3. While the heap is not empty, repeatedly call extractMin() and append the minimum element to the result vector.
        4. Return the resulting vector containing all input elements in ascending order.
        */
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

        /*
        1. Make a local copy of the current heap instance so that the operation is non-destructive.
        2. Allocate a result vector reserving space for copy.size() elements.
        3. While the copied heap is not empty, repeatedly call extractMin() and append to the result vector.
        4. Return the resulting sorted vector in ascending order.
        */
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
