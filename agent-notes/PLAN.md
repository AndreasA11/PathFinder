# Plan: A* Pathfinding Planner

## Goal

Implement a reusable planner application component that stores the newest valid
/map message, exposes /plan_path, computes a shortest four-connected path, and
publishes each successful result on /path.

## Relevant requirements

- Parse arbitrary valid occupancy maps with their width, height, resolution,
  origin, and row-major integer data.
- Treat only values in [0, 50) as free. Negative/unknown and values >= 50 are
  blocked.
- Convert world endpoints with floor((world - origin) / resolution), reject
  invalid, out-of-bounds, and blocked endpoints, and return promptly if no map
  or path exists.
- Use A* with four unit-cost cardinal moves and an admissible Manhattan
  heuristic. Successful paths must be shortest, start-to-goal ordered, and use
  cell centers with identity orientation.
- Return success values.plan with a Path-like header.frame_id of map; publish
  exactly that path on /path only after success.
- A later valid /map replaces the old one. Generic middleware and map producer
  must remain application-agnostic.

## Repository observations

- middleware::Middleware offers generic topic subscriptions, publications, and
  service registration. The gateway already forwards those operations.
- map_producer.hpp serializes compatible map messages but is deliberately not
  an interpreter. src/astar_node.cpp is empty.
- src/main.cpp and src/hub.cpp compose runtime services and startup map
  publication. Both require planner registration before their map publication
  so the planner receives it.
- Tests use direct in-process Middleware calls, suitable for deterministic
  planner coverage without the occupied fixed TCP port.

## Proposed architecture

Add include/path_planner.hpp. It will contain a Planner class with:

- a mutex-protected current parsed map;
- a /map callback that accepts only fully valid messages and atomically replaces
  current state;
- independently testable world/cell conversion and A* search over Cell values;
- a service handler that parses named start/goal/tolerance arguments, invokes
  A*, formats Path-like JSON, and publishes the same JSON to /path;
- a registration helper that installs the /map subscription and /plan_path
  provider on a supplied Middleware instance.

## Interfaces and data flow

External or internal /map publication -> Planner map callback -> current map.

/plan_path args -> request parser -> floor world-to-cell conversion -> A* core
-> values.plan response and Middleware publish(/path, plan).

## Implementation steps

1. Implement path_planner.hpp with parsed-map validation, occupancy/bounds
   helpers, world/cell conversions, Manhattan A*, request parsing, response
   construction, and middleware registration. Add required block and inline
   implementation comments.
2. Integrate registration in main.cpp and hub.cpp before their startup map
   publication; leave generic middleware and map producer unchanged.
3. Add planner tests covering no map, malformed maps/requests, nonzero origin
   and non-unit resolution, row-major obstacles, unknown cells, no path,
   bounds, start equals goal, shortest four-connected behavior, /path success
   publication, failure nonpublication, and map replacement.
4. Add a Makefile target and run targeted builds/tests plus available
   regressions.
5. Record implementation and test results in agent-notes.

## Edge cases and failure modes

- Reject missing/wrong JSON types, non-integral or overflowing dimensions,
  nonfinite coordinates, bad resolution, and mismatched map data.
- Do not let invalid later maps replace the last valid map.
- Include stale priority-queue entries safely; reconstruct only after reaching
  goal; prevent row wrapping by explicit x/y bounds.
- Catch malformed request parsing and report result:false without publishing
  /path. Tolerance is accepted if numeric but does not change Project 1 search.

## Open questions and assumptions

- The documented request example has a minor extra quote in goal.orientation.z;
  orientation is not needed for grid planning, while named start/goal positions
  and optional numeric tolerance are enforced.
- A planner process must be registered before startup map publication. External
  maps remain ordinary later topic publications and replace planner state.

## Verification strategy

- Run direct unit/integration tests against the documented middleware service
  interface and inspect returned poses as cells.
- Verify path length with a BFS oracle or known optimal maps; assert every step
  is cardinal, free, and in bounds.
- Verify replacement and absence of /path on all failure cases.
- Compile both normal runtime entry points and run existing feasible tests.
