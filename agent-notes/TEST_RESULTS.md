# Test Results: Binary Min-Heap Primitive

## Summary
All 10 acceptance criteria defined in `promts/Task_ Binary Min-Heap Primitive.md` have been verified. All test suites compiled cleanly under C++20 with `-Wall -Wextra -Wpedantic` and passed without failures.

## Test Environment
- Compiler: Apple clang / g++ supporting `-std=c++20`
- Compilation flags: `-std=c++20 -Wall -Wextra -Wpedantic -Iinclude`
- Test command: `make test`

## Tests Performed

### 1. Empty Heap Handling
- **Requirement/Risk**: Creating an empty heap; calling `getMin()` or `extractMin()` on empty heap.
- **Method**: Instantiated empty `MinHeap<double>`, queried `isEmpty()`, `empty()`, `size()`, called `getMin()` and `extractMin()` in try/catch blocks, verified empty input to `heapSort`.
- **Expected Behavior**: Size is 0; `getMin()` and `extractMin()` throw `std::underflow_error`; empty input to `heapSort` returns empty vector.
- **Observed Behavior**: Exceptions caught as expected; size and vectors empty.
- **Result**: PASS

### 2. Singleton Heap Handling
- **Requirement/Risk**: Singleton input handling and edge cases when heap transitions between 1 and 0 elements.
- **Method**: Instantiated `MinHeap<double>({42.5})`, checked root, extracted minimum, verified empty afterward; tested single-element `heapSort({-17.2})`.
- **Expected Behavior**: Root element is returned; heap empties cleanly without attempting invalid sifts; sorted output matches single element.
- **Observed Behavior**: All assertions hold.
- **Result**: PASS

### 3. Heapify from Arbitrary Array & Multiset Preservation
- **Requirement/Risk**: Bottom-up construction from arbitrary vector; multiset preservation; invariant satisfaction.
- **Method**: Initialized `MinHeap` with `{9.0, 3.5, 7.2, 1.1, 8.4, 2.0, 5.5, 0.5, 6.3}`. Verified min-heap invariant for all parent-child pairs. Compared sorted inputs and sorted heap elements.
- **Expected Behavior**: Min-heap invariant holds for all indices; root is minimum (0.5); multiset identical to input.
- **Observed Behavior**: Invariant holds; multiset preserved.
- **Result**: PASS

### 4. Insertion Preserves Invariant
- **Requirement/Risk**: Successive insertions maintain min-heap invariant at each step.
- **Method**: Successively inserted unsorted integers `{10, 4, 15, 20, 0, 8, -5, 3}` into an initially empty heap, verifying invariant after each insertion.
- **Expected Behavior**: Invariant maintained after every insert; root holds minimum (-5).
- **Observed Behavior**: Invariant held across all steps.
- **Result**: PASS

### 5. Extract-Min Order & Invariant Preservation
- **Requirement/Risk**: Sift-down restores invariant after removal; successive extracts yield nondecreasing sequence.
- **Method**: Inserted array with positive, negative, fractional numbers and duplicates. Repeatedly extracted min until empty, verifying invariant after each extraction.
- **Expected Behavior**: Each extracted element is $\le$ the next; invariant preserved after every extract; multiset matches input.
- **Observed Behavior**: Nondecreasing order verified; invariant satisfied at all intermediate states.
- **Result**: PASS

### 6. Heap Sort
- **Requirement/Risk**: Arbitrary numeric input produces ascending sequence via heap operations alone without general-purpose sort.
- **Method**: Called `MinHeap::heapSort` on `[3.0, 1.0, 2.0]` and mixed vectors with negative, fractional, duplicate, and all-equal values.
- **Expected Behavior**: Returns `[1.0, 2.0, 3.0]`, handles mixed values and identical values correctly.
- **Observed Behavior**: Exactly matched expected output.
- **Result**: PASS

### 7. Duplicates and All-Equal Values
- **Requirement/Risk**: Duplicates are not lost; all-equal values do not cause infinite loops or corruptions.
- **Method**: Passed `{5, 5, 2, 2, 8, 8, 2}` and `{7, 7, 7, 7, 7}` through heap construction and sorting.
- **Expected Behavior**: All duplicate instances preserved.
- **Observed Behavior**: Multiset counts identical; sorting exact.
- **Result**: PASS

### 8. decreaseKey Operation
- **Requirement/Risk**: Decreasing a key properly sifts up; invalid index and key increase handled.
- **Method**: Decreased value at leaf from 50 to 5, checked root becomes 5; tested out-of-range index and invalid increase.
- **Expected Behavior**: Min-heap invariant restored; `std::out_of_range` on bad index; `std::invalid_argument` on key increase.
- **Observed Behavior**: Proper exceptions thrown and caught; invariant maintained.
- **Result**: PASS

### 9. deleteKey Operation
- **Requirement/Risk**: Deleting root, interior element, last element, or non-existent key.
- **Method**: Executed deletions for middle element (12), root, and last element in array; attempted deletion of missing key.
- **Expected Behavior**: Invariant maintained after each delete; size decremented; missing key throws `std::runtime_error`; last element deleted without out-of-bounds sift.
- **Observed Behavior**: All cases passed cleanly.
- **Result**: PASS

### 10. Backward Compatibility Alias
- **Requirement/Risk**: Existing references to `BinaryHeap<T>` remain functional.
- **Method**: Constructed and operated on `BinaryHeap<double>`.
- **Expected Behavior**: Behaves identically to `MinHeap<double>`.
- **Observed Behavior**: Verified identical behavior.
- **Result**: PASS

## Failures
None.

## Remaining Gaps
None. All requirements specified in `promts/Task_ Binary Min-Heap Primitive.md` are satisfied and verified.
