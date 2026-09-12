# Task: Heap Services

Expose the existing binary min-heap functionality through the Project 1 service interface.

This task integrates two components that should already exist:

1. the reusable binary min-heap primitive;
2. the generic ROS-like middleware and service system.

Do not redesign either component unless a genuine integration defect requires a minimal change.

Do not implement map handling or A* yet.

## Goal

Provide the Project 1 services:

- `/heapify`
- `/heap_sort`

These must be ordinary services provided through the existing middleware and accessible to independent external clients through the existing TCP/JSON gateway.

Use the service request and response protocol defined in `ROSBRIDGE_PROTOCOL.md`.

The service implementations should delegate heap behavior to the existing heap primitive rather than duplicating heap algorithms inside the service layer.

---

## `/heapify`

Provide a service named:

```text
/heapify
```

Example arguments:

```json
{
  "values": [3.0, 1.0, 2.0]
}
```

On success, return service values containing:

```json
{
  "heap": [...]
}
```

The returned array must:

- contain exactly the same multiset of values as the input;
- preserve duplicates;
- satisfy the binary min-heap invariant;
- support empty input;
- support finite negative and fractional numeric values.

For every valid index `i`:

```text
heap[i] <= heap[2*i + 1]
```

when the left child exists, and:

```text
heap[i] <= heap[2*i + 2]
```

when the right child exists.

There is no required exact heap arrangement.

For example:

```text
input:
[3.0, 1.0, 2.0]

one valid result:
[1.0, 3.0, 2.0]
```

Other valid min-heap arrangements are acceptable.

Use the existing heap primitive to perform this operation.

Do not implement a second independent heap algorithm in the service handler.

---

## `/heap_sort`

Provide a service named:

```text
/heap_sort
```

Example arguments:

```json
{
  "numbers": [3.0, 1.0, 2.0]
}
```

On success, return service values containing:

```json
{
  "sorted": [1.0, 2.0, 3.0]
}
```

The returned array must:

- contain exactly the same multiset of values as the input;
- preserve duplicates;
- be in ascending numeric order;
- support empty input;
- support finite negative and fractional values.

Implement the operation using the existing heap primitive.

The heap behavior must be implemented from scratch. Do not replace it with:

- a general-purpose sorting routine;
- Python `heapq`;
- C++ `std::priority_queue`;
- Rust `BinaryHeap`;
- another built-in or library heap / priority-queue implementation.

The intended behavior should be achievable by constructing the min-heap and repeatedly obtaining/removing its minimum element.

---

## Service integration

Register both services through the existing middleware's generic service-provider mechanism.

External clients must be able to invoke them through the existing TCP/JSON gateway using the documented service protocol.

The integration should follow this conceptual path:

```text
external client
      |
      | call_service
      v
TCP/JSON gateway
      |
      v
generic service middleware
      |
      +---- /heapify ----> heap primitive
      |
      +---- /heap_sort --> heap primitive
      |
      v
service response
      |
      v
external client
```

Do not special-case these operations inside the TCP server if the existing architecture provides a generic service abstraction.

Application-specific heap behavior belongs in the heap service provider, not in the generic gateway or middleware.

---

## Request/response behavior

Preserve the service semantics already established by the middleware and `ROSBRIDGE_PROTOCOL.md`.

In particular:

- route `/heapify` requests to the `/heapify` provider;
- route `/heap_sort` requests to the `/heap_sort` provider;
- return the response to the requesting client;
- preserve the appropriate service request/response correlation information;
- support repeated calls;
- support calls from independent clients.

Do not introduce a new heap-specific wire protocol.

---

## Reuse of the heap primitive

The existing heap primitive should remain the single implementation of the heap algorithm.

The service layer should primarily be responsible for:

1. receiving service arguments;
2. extracting the numeric input;
3. invoking the appropriate heap functionality;
4. constructing the required service result;
5. returning it through the middleware.

Avoid copying sift-up, sift-down, heap construction, or extract-min logic into service handlers.

If the existing primitive does not expose a small piece of functionality needed for clean integration, prefer a minimal reusable change to the primitive over duplicating the algorithm.

---

## Scope

This task includes:

- registering `/heapify`;
- registering `/heap_sort`;
- connecting both services to the existing heap primitive;
- exposing both through the existing middleware and TCP/JSON gateway;
- any minimal runtime wiring needed to make the services available.

This task does **not** include:

- map handling;
- `/map`;
- `/plan_path`;
- `/path`;
- occupancy-grid interpretation;
- A* search;
- A* frontier management.

The later planner will depend on `/heap_sort`, but do not implement that planner in this task.

---

## Important separation of responsibilities

Maintain the existing component boundaries:

```text
Heap primitive
    owns:
        heap invariant
        heap construction
        insertion/extraction
        heap-based ordering

Heap service provider
    owns:
        /heapify behavior
        /heap_sort behavior
        conversion between service arguments/results
        and heap operations

Middleware
    owns:
        service registration
        routing
        request/response correlation

TCP/JSON gateway
    owns:
        external protocol handling
```

Do not move application logic into generic middleware merely because the first implemented services happen to be heap services.

---

## Acceptance criteria

The completed integration should make it possible to demonstrate that:

1. `/heapify` is available through the external service interface.
2. `/heap_sort` is available through the external service interface.
3. `/heapify` returns a valid binary min-heap.
4. `/heapify` preserves the complete input multiset.
5. `/heap_sort` returns values in ascending numeric order.
6. `/heap_sort` preserves the complete input multiset.
7. Both services correctly support empty input.
8. Both services preserve duplicates.
9. Both services correctly support finite negative and fractional values.
10. Repeated calls continue to behave correctly.
11. Calls from independent external clients are routed and correlated correctly.
12. Service handlers reuse the existing heap primitive rather than duplicating its implementation.
13. No prohibited built-in heap or priority-queue implementation is used.
14. `/heap_sort` actually exercises the heap implementation rather than substituting a general-purpose sort.
15. Generic middleware remains generic and does not contain heap-specific algorithm logic.
16. No map or A* functionality is added as part of this task.