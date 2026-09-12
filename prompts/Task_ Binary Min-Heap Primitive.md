# Task: Binary Min-Heap Primitive

Implement the reusable binary min-heap primitive needed for Project 1.



This task is intentionally **below the middleware layer**. Do not implement services, topics, TCP/JSON, rosbridge behavior, or A* yet. The goal is to create and verify the underlying heap functionality that later phases can expose through `/heapify`, `/heap_sort`, and use for planning.

## Required behavior

Create a `MinHeap` class, struct, or equivalent abstraction appropriate for the selected language.

The heap should be backed by an array/vector-like container and maintain the binary min-heap invariant:

- for an element at index `i`, its left child is at `2*i + 1`;
- its right child is at `2*i + 2`;
- every parent is less than or equal to each child that exists;
- therefore, the minimum element is at the root.

Implement the heap logic from scratch. Do not use built-in or library heap / priority-queue implementations such as Python `heapq`, C++ `std::priority_queue`, Rust `BinaryHeap`, or equivalent helpers.

## Required operations

The primitive must support the functionality needed to build the later Project 1 heap interfaces.

At minimum, support:

- creating an empty heap;
- creating a heap from an arbitrary array of numeric values;
- inspecting/exporting the heap contents as an array;
- reading the current minimum;
- inserting a value while preserving the heap invariant;
- extracting/removing the minimum while preserving the heap invariant.

The implementation should use normal heap operations such as sift-up and sift-down rather than repeatedly sorting the entire container.

## Heapify behavior

Construction from an arbitrary array must transform the values into a valid binary min-heap.

The result must:

- contain exactly the same multiset of values as the input;
- preserve duplicates;
- satisfy the min-heap invariant;
- support empty input.

There is no required exact heap layout. Any valid min-heap arrangement is acceptable.

For example:

```text
input:
[3, 1, 2]

one valid result:
[1, 3, 2]
```

## Heap-sort support

The primitive must provide enough functionality to implement Project 1 heap sort from the heap itself.

Using the heap operations, an arbitrary numeric input must be able to produce all values in ascending order by repeatedly obtaining/removing the minimum.

Do not implement this by calling the language's general-purpose sort routine instead of exercising the heap.

For example:

```text
input:
[3, 1, 2]

sorted:
[1, 2, 3]
```

Whether this is exposed as a method on `MinHeap`, a separate heap-sort function built on `MinHeap`, or another simple interface should be decided during planning.

## Numeric requirements

Correctly handle:

- empty input;
- singleton input;
- duplicates;
- all-equal values;
- negative values;
- fractional values;
- mixtures of positive, negative, and fractional values;
- already heapified input;
- arbitrarily ordered input.

Assume input numeric values are finite.

## Scope

Keep this task limited to the reusable heap primitive.

Do **not** implement:

- `/heapify` as a service;
- `/heap_sort` as a service;
- middleware;
- publishers or subscribers;
- service clients or providers;
- TCP/JSON communication;
- the rosbridge-style gateway;
- occupancy-grid handling;
- A*.

Later work will wrap this primitive in the Project 1 communication interfaces.

## Acceptance criteria

The completed primitive should make it possible to demonstrate that:

1. An arbitrary input array can be converted into a valid binary min-heap.
2. Heap construction preserves the complete input multiset.
3. The minimum nonempty heap element is available at the root.
4. Insertion preserves the heap invariant.
5. Extract-min returns the current minimum and preserves the heap invariant afterward.
6. Repeated extract-min operations produce the original values in nondecreasing order.
7. Duplicate values are never lost.
8. Empty, singleton, negative, and fractional-value cases behave correctly.
9. No prohibited library heap or priority-queue implementation is used.
10. The implementation has no dependency on Project 1 middleware or A*.