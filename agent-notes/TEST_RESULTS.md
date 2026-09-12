# Test Results: Map Production and /map Publication

## Summary

The focused map-producer test suite passed. Both runtime binaries compile with
the new map component. The pre-existing TCP middleware regression suite could
not run because its fixed local port 127.0.0.1:9095 was unavailable in this
environment; this is recorded as an environment gap, not a map defect.

## Test environment

- Compiler command: c++ -std=c++20 -Wall -Wextra -Wpedantic -Iinclude
- Focused command: make test_map
- Build commands: make build, make hub

## Tests performed

### Default-map validity

- **Requirement/risk:** positive dimensions and resolution, nonzero metadata,
  exact cell count, free/blocked occupancy semantics, and at least two
  4-connected free cells.
- **Method:** Constructed make_default_map() and inspected its definition.
- **Expected behavior:** a valid map with at least one horizontally or
  vertically adjacent pair of free cells.
- **Observed behavior:** the map has 4 x 3 cells, 0.5 resolution, origin
  (-1, 2), valid data length, and qualifying free cells.
- **Result:** pass.

### JSON schema and metadata preservation

- **Requirement/risk:** required fields, identity orientation, arbitrary valid
  metadata, and row-major data preservation.
- **Method:** Serialized a custom 3 x 2 map with non-default frame,
  resolution, origin, and occupancy values.
- **Expected behavior:** serialized JSON preserves metadata and maps
  data[y * width + x] directly to the supplied array element.
- **Observed behavior:** all required nested fields and orientation values were
  present; each data value retained its index.
- **Result:** pass.

### Invalid definitions

- **Requirement/risk:** malformed generated maps must not be published.
- **Method:** Tested zero width, zero resolution, infinite origin, short data,
  and overflowing dimensions.
- **Expected behavior:** validation rejects each case and serialization throws.
- **Observed behavior:** every malformed definition was rejected.
- **Result:** pass.

### Generic middleware publication

- **Requirement/risk:** the producer must publish a map without embedding
  application behavior in middleware.
- **Method:** Subscribed to /map on a fresh Middleware instance, then called
  publish_map with independent metadata.
- **Expected behavior:** exactly one normal topic message reaches the
  subscriber with unchanged metadata and cell data.
- **Observed behavior:** one message was delivered and preserved the custom
  frame, resolution, origin, and occupancy entries.
- **Result:** pass.

### Build verification

- **Requirement/risk:** normal application and hub entry points integrate the
  component.
- **Method:** Ran make build and make hub.
- **Expected behavior:** warning-clean compilation.
- **Observed behavior:** both completed successfully.
- **Result:** pass.

## Failures

No map-production failures observed.

The existing make test_middleware test failed before its first TCP case because
RosbridgeServer::start() could not bind port 9095. Retrying produced the same
assertion at tests/test_middleware.cpp:209. The map test does not bind a TCP
port and passed; the available evidence points to local port availability
rather than the new map implementation.

## Remaining gaps

The existing fixed-port TCP middleware and heap-service integration tests could
not be re-run while port 9095 remains unavailable. The focused map component,
in-process publication, and both production binaries were verified.

---

# A* Planner Verification

## Summary

make test_path_planner passed, as did builds of both runtime entry points.

## Tests performed

- No /map request: /plan_path returned false.
- Arbitrary 3 x 3 map with nonzero origin and 0.5 resolution: returned a
  five-pose shortest path around a blocked center, with correct cell centers.
- Successful request: exactly one matching /path publication was observed.
- Replacement 2 x 2 map: later request used replacement metadata and rejected
  unreachable cells without republishing the prior path.
- Start equals goal: returned one pose and published it.

## Remaining gaps

Existing fixed-port TCP tests remain unavailable because port 9095 is occupied
in this environment. Direct middleware service and topic integration passed.
