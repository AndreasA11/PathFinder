# Plan: Binary Min-Heap Primitive

## Goal
Refactor and complete the binary min-heap implementation in `src/heap_sort_node.cpp` (and provide `include/min_heap.hpp`) to fully satisfy the requirements in `promts/Task_ Binary Min-Heap Primitive.md`. Add clear comments explaining logic and tag all modifications with `*CHANGES*` as required by project conventions. Update the `Makefile` and create unit tests to verify all acceptance criteria.

## Relevant Requirements
From `promts/Task_ Binary Min-Heap Primitive.md`:
1. **Core Abstraction**:
   - Create `MinHeap` class/struct backed by an array/vector container (`std::vector<T>`).
   - Binary min-heap invariant: root at index 0; for index $i$, left child is $2i + 1$, right child is $2i + 2$, parent is $(i - 1) / 2$.
   - Every parent $\le$ each child that exists; minimum element is at root.
   - Logic built from scratch without built-in heap/priority queue libraries (`std::priority_queue`, `std::make_heap`, etc.).
2. **Required Operations**:
   - Creating an empty heap.
   - Creating a heap from an arbitrary array/vector of numeric values ($O(n)$ bottom-up heapify).
   - Inspecting/exporting heap contents as an array (`toVector()` / `data()`).
   - Reading the current minimum (`getMin()` / `peek()`).
   - Inserting a value while preserving heap invariant (`insert()`).
   - Extracting/removing minimum while preserving heap invariant (`extractMin()`).
   - Support for heap sort (`heapSort()`) by repeatedly extracting the minimum element without standard sorting shortcuts.
3. **Numeric and Edge Case Support**:
   - Empty input, singleton input, duplicates, all-equal values.
   - Negative values, fractional/floating-point values (`double`, `float`).
   - Arbitrarily ordered input, already-heapified input.
4. **Scope Limitations**:
   - Strict separation below middleware layer: no ROS topics/services, TCP/JSON, or A* logic yet.

## Repository Observations
- The user's initial code in `src/heap_sort_node.cpp` defined `template <typename T> class BinaryHeap`.
- Compilation errors in current code:
  - Undeclared `runtime_error` (missing `#include <stdexcept>` and `std::` prefix).
  - Undeclared `size()` called in `deleteKey`.
  - Syntax error in `printHeap`: `for (const &T elt : heap)` instead of `const T& elt`.
- Incomplete functionality:
  - No vector/array constructor or bottom-up `heapify` routine.
  - No export/inspect method returning the internal vector/array.
  - No `heapSort` method or standalone function.
  - Uninitialized `int currentSize`.
  - `deleteKey` fails to handle the case where the key to delete is at the end of the heap, and attempts to sift out-of-bounds indices.
  - `decreaseKey` lacks validation that the new value is actually $\le$ current value.
  - `extractMin` attempts to sift down index 0 even when the heap becomes empty after removing the single root element.
- `Makefile` currently references `src/main.cpp` which does not exist, and has no `test` target.

## Proposed Architecture
1. **Class Definition**:
   - Class name `MinHeap<T>`, with alias `template <typename T> using BinaryHeap = MinHeap<T>;` for backward compatibility.
   - Backing storage: `std::vector<T> heap;`.
   - Helper indexing functions: `parent(size_t)`, `leftChild(size_t)`, `rightChild(size_t)`.
   - Sift operations: `heapifyUp(size_t index)` and `heapifyDown(size_t index)`.
2. **Public API**:
   - Constructors: Default constructor `MinHeap()`, vector constructor `MinHeap(const std::vector<T>& values)`, initializer-list constructor.
   - Capacity: `bool isEmpty() const noexcept`, `bool empty() const noexcept`, `size_t size() const noexcept`.
   - Inspection: `const std::vector<T>& data() const noexcept`, `std::vector<T> toVector() const`.
   - Access: `const T& getMin() const` (throws `std::underflow_error` on empty heap).
   - Modification:
     - `void insert(const T& key)` (sift-up).
     - `T extractMin()` (removes root, swaps with last, sifts down if non-empty, throws `std::underflow_error` on empty).
     - `void decreaseKey(size_t index, const T& newValue)` (validates bounds and `newValue <= heap[index]`, sifts up).
     - `void deleteKey(const T& key)` (finds element, replaces with last, sifts up or down safely).
     - `void heapify()` (bottom-up Floyd's heap construction in $O(n)$).
   - Sorting:
     - Static function `static std::vector<T> heapSort(const std::vector<T>& values)`.
     - Member function `std::vector<T> sort() const` / drain sort.
3. **Include Header & Linking**:
   - Header guards in `src/heap_sort_node.cpp`.
   - Header proxy `include/min_heap.hpp` including `src/heap_sort_node.cpp`.

## Implementation Steps
1. Update `src/heap_sort_node.cpp`:
   - Add required headers (`<vector>`, `<iostream>`, `<stdexcept>`, `<utility>`, `<cstddef>`).
   - Fix all compilation errors (`std::runtime_error`/`std::underflow_error`, `const T&`, `size()`).
   - Implement `MinHeap` and `BinaryHeap` alias.
   - Implement bottom-up `heapify()` and constructors.
   - Implement `toVector()` / `data()`.
   - Fix bounds and edge cases in `extractMin()`, `deleteKey()`, `decreaseKey()`, `heapifyUp()`, `heapifyDown()`.
   - Implement `heapSort()`.
   - Add detailed comments explaining the logic and `*CHANGES*` comments noting reasons for changes.
2. Provide `include/min_heap.hpp` header forwarding to `src/heap_sort_node.cpp`.
3. Create `tests/test_min_heap.cpp`:
   - Unit tests covering all acceptance criteria (empty, singleton, duplicates, negatives, floats, heapify, insert, extractMin, sort, decreaseKey, deleteKey).
4. Update `Makefile`:
   - Add `-Iinclude -Isrc` to `CXXFLAGS`.
   - Add `test_heap` and `test` targets to compile and execute tests.
   - Provide a working default build target.
5. Execute tests and verify all requirements pass.

## Edge Cases and Failure Modes
- Empty heap: `extractMin()` and `getMin()` throw appropriate exception.
- Single element: `extractMin()` correctly empties heap without invalid sift-down.
- Duplicates and all-equal values: heap invariant holds, multiset preserved.
- Negative and fractional numbers: proper floating-point comparisons.
- Deleting the last element in `deleteKey()`: avoids out-of-bounds sift operations.
- `decreaseKey` with invalid index or larger key: proper exception thrown.

## Verification Strategy
- Build with `-Wall -Wextra -Wpedantic` to ensure zero warnings.
- Run automated unit test suite checking all 10 acceptance criteria.
