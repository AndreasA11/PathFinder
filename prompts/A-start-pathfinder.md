Task: A* Pathfinding Planner

Implement the A* pathfinding component needed for Project 1.

This task is focused on interpreting the current /map, planning a shortest 4-connected path, and exposing the planner through /plan_path. Do not implement or modify the generic publish/subscribe middleware or map-production component unless integration changes are necessary.

Do not use ROS, ROS 2, or ROS client/runtime libraries.

Goal

Build a reusable A* planner that:

subscribes to /map;
maintains the most recently received valid map;
accepts /plan_path service requests;
converts world coordinates to map cells;
computes a shortest 4-connected path using A*;
returns the path as ordered PoseStamped-like poses;
publishes successful paths on /path;
handles invalid or impossible planning requests without crashing or hanging.

The planner must operate on arbitrary valid maps supplied through /map.

Do not hard-code the map produced by the previous task.

/map subscription

Subscribe to:

/map

The planner must use the most recently received valid map.

A map contains:

header.frame_id
info.resolution
info.width
info.height
info.origin
data

The planner must correctly interpret:

map width;
map height;
map resolution;
map origin;
occupancy data.

Do not assume:

resolution = 1
origin.x = 0
origin.y = 0

Do not assume a particular map size or obstacle arrangement.

A later valid /map publication replaces the previously stored map.

The planner must therefore work when the autograder publishes an entirely different map after the project's own map has been produced.

Occupancy semantics

Interpret map cells using the following rules:

A cell is free exactly when:

0 <= occupancy < 50

A cell is blocked when:

occupancy < 0

or:

occupancy >= 50

This includes unknown cells such as:

-1

Unknown cells must not be traversed.

The planner must not assume that only 0 represents free space.

For a cell (x, y), its occupancy value is:

data[y * width + x]
/plan_path service

Provide the service:

/plan_path

The service request contains named arguments:

{
  "start": {
    "header": {
      "frame_id": "map"
    },
    "pose": {
      "position": {
        "x": -0.75,
        "y": 2.25,
        "z": 0.0
      },
      "orientation": {
        "x": 0.0,
        "y": 0.0,
        "z": 0.0,
        "w": 1.0
      }
    }
  },
  "goal": {
    "header": {
      "frame_id": "map"
    },
    "pose": {
      "position": {
        "x": 0.25,
        "y": 3.25,
        "z": 0.0
      },
      "orientation": {
        "x": 0.0,
        "y": 0.0",
        "z": 0.0,
        "w": 1.0
      }
    }
  },
  "tolerance": 0.0
}

Accept the tolerance argument.

For Project 1, tolerance does not need to alter planning behavior.

Historical positional-array arguments may optionally be supported, but named arguments are the required interface.

Request validation

Before planning, validate that:

a valid map has been received;
the start position can be converted to a map cell;
the goal position can be converted to a map cell;
the resulting cells are inside the map;
both endpoint cells are free.

If any of these conditions fail, return:

{
  "result": false
}

Do not crash.

Do not hang waiting indefinitely for a map.

Do not attempt to plan through invalid endpoints.

World-to-cell conversion

Convert world coordinates to map-cell coordinates using:

cell_x = floor((world_x - origin_x) / resolution)
cell_y = floor((world_y - origin_y) / resolution)

where:

world_x and world_y are the requested world coordinates;
origin_x and origin_y come from the map;
resolution comes from the map.

Do not round to the nearest cell.

Use floor-based conversion exactly as specified.

For example, with:

origin_x = -1.0
origin_y = 2.0
resolution = 0.5

a world position of:

x = -0.75
y = 2.25

maps to:

cell_x = 0
cell_y = 0
Cell-to-world conversion

When returning a path, convert each cell to the center of that cell:

world_x = origin_x + (cell_x + 0.5) * resolution
world_y = origin_y + (cell_y + 0.5) * resolution

Do not return the original requested endpoint coordinates unless they happen to equal the appropriate cell centers.

Every returned pose should represent the center of its corresponding map cell.

Map bounds

A cell is inside the map exactly when:

0 <= cell_x < width
0 <= cell_y < height

The planner must reject endpoints outside these bounds.

It must also ensure that every generated path cell remains inside the map.

Do not wrap around map boundaries.

Do not clamp an out-of-bounds endpoint into the map.

Movement model

Movement is strictly 4-connected.

From a cell (x, y), the only valid neighbors are:

(x + 1, y)
(x - 1, y)
(x, y + 1)
(x, y - 1)

Each movement has a cost of:

1

Do not allow diagonal movement.

Do not allow:

(x + 1, y + 1)
(x - 1, y + 1)
(x + 1, y - 1)
(x - 1, y - 1)

There is therefore no diagonal corner-cutting behavior to implement.

Do not teleport between non-adjacent cells.

Traversability

A neighbor may be added to the search only if it:

is inside the map bounds;
corresponds to a free cell.

Blocked and unknown cells must not be traversed.

The resulting path must never contain:

an out-of-bounds cell;
a blocked cell;
an unknown cell.
A* algorithm

Use A* search to find a shortest path from the start cell to the goal cell.

The search should use:

g(n) = cost from start to n
h(n) = estimated cost from n to goal
f(n) = g(n) + h(n)

For the 4-connected unit-cost movement model, a suitable heuristic is Manhattan distance:

h(n) = abs(x - goal_x) + abs(y - goal_y)

The heuristic must not overestimate the true remaining path cost.

The implementation may use any reasonable priority-queue/open-set representation.

The exact internal data structures are not prescribed.

Optimality

A successful path must contain the minimum possible number of moves between the start and goal cells.

Any path with the minimum cost is valid.

Do not require the planner to reproduce one particular route when multiple routes have the same cost.

For example, if two different paths both require 8 moves, either path is acceptable.

The planner must not return a longer path merely because of neighbor ordering or tie-breaking.

Path requirements

On successful planning, return:

result: true

with:

values.plan

containing a Path-like structure.

The path must contain:

header.frame_id = "map"

and an ordered:

poses

array.

Each pose should contain the corresponding cell-center position.

Use:

z = 0.0

and identity orientation:

x = 0.0
y = 0.0
z = 0.0
w = 1.0

Timestamps are unconstrained and may be zero.

Path ordering

The returned poses must be ordered from:

start → ... → goal

The first pose must correspond to the start cell.

The final pose must correspond to the goal cell.

Each consecutive pair of poses must represent adjacent 4-connected cells.

Start and goal behavior

A successful path must:

be nonempty;
include the free start cell;
include the free goal cell;
remain inside map bounds;
contain only free cells;
use only valid 4-connected movements;
have minimum move cost.

If:

start == goal

the planner must return a valid path containing exactly one cell.

That means the returned path contains one PoseStamped-like pose representing that cell's center.

/path publication

After every successful /plan_path request, publish the same ordered cell-center path on:

/path

The /path publication must correspond exactly to the successful planning result.

Do not publish /path when planning fails.

In particular, a failed request must not accidentally republish the previous successful path.

Failure behavior

Return:

{
  "result": false
}

when planning cannot be completed.

Failures include, but are not limited to:

no map has been received;
start position is outside the map;
goal position is outside the map;
start cell is blocked;
goal cell is blocked;
start cell is unknown;
goal cell is unknown;
no path exists between the endpoints;
malformed or otherwise invalid planning input.

Failure values and status text are unspecified.

The planner must:

return promptly;
not crash;
not hang indefinitely;
not publish /path.
No map assumptions

The planner must not depend on the example map from the map-production task.

For example, do not hard-code:

width = 4
height = 3
resolution = 0.5
origin = (-1.0, 2.0)

The autograder may independently publish maps with:

different dimensions;
different resolutions;
different origins;
different free/blocked layouts;
different start/goal locations.

The planner must process those maps correctly.

Map replacement behavior

If the planner receives:

map A

and later receives:

map B

then future planning requests must use:

map B

not map A.

Do not retain stale dimensions, resolution, origin, or occupancy data from the previous map.

The planner should treat each valid /map message as the current planning environment.

Separation of responsibilities

Keep the following responsibilities separate:

Map production

Responsible for creating and publishing an example map.

Map subscription/interpreting

Responsible for storing and interpreting the current /map.

Path planning

Responsible for converting endpoints to cells and running A*.

Middleware

Responsible for generic topic and service communication.

Do not add application-specific A* behavior to the generic middleware.

Do not implement /plan_path directly inside the middleware.

The service provider should invoke planner/application logic.

Reusability

The planner should be structured so that the core A* implementation can be tested independently from the communication layer where practical.

For example, the planning logic should conceptually be able to operate on:

map
start cell
goal cell

and return:

ordered cell path

The ROS-like service layer can then handle:

request parsing;
world/cell conversion;
response formatting;
/path publication.

The exact internal architecture is up to you.

Scope

Do not implement additional robotics behavior that is not required for Project 1.

Do not add:

robot localization;
SLAM;
dynamic obstacle avoidance;
diagonal movement;
motion planning;
trajectory generation;
velocity commands;
ROS navigation stack functionality;
ROS map-server functionality.

The planner only needs to find a shortest grid path through the supplied occupancy map.

Acceptance criteria

The completed A* planner should make it possible to demonstrate that:

The planner subscribes to /map.
The planner uses the most recently received valid map.
The planner provides /plan_path.
/plan_path accepts named start, goal, and tolerance arguments.
The planner correctly converts world coordinates to cells using floor-based conversion.
The planner correctly handles nonzero map origins.
The planner correctly handles non-unit map resolutions.
Map cells are interpreted using the required occupancy semantics.
Unknown/negative cells are not traversed.
Occupancy values >= 50 are not traversed.
Movement is strictly 4-connected.
Diagonal movement is never used.
Out-of-bounds cells are never included in a path.
Blocked cells are never included in a path.
A* finds a path when one exists.
The returned path has the minimum possible number of moves.
Equal-cost paths are all considered valid.
Returned positions are cell centers.
The returned path starts at the start cell.
The returned path ends at the goal cell.
start == goal produces a one-cell path.
Successful requests return result: true.
Successful requests include values.plan.
values.plan.header.frame_id is "map".
Returned poses use zero Z and identity orientation.
A successful request publishes the same path on /path.
Failed requests return result: false.
Failed requests do not publish /path.
Requests made before a map arrives fail cleanly rather than hanging or crashing.
Outside, blocked, unknown, and unreachable endpoints fail cleanly.
A later /map publication replaces the previous planning map.
The planner works with arbitrary valid grading maps rather than only the project's example map.
Generic middleware code contains no A* or map-specific application logic.
No ROS, ROS 2, or ROS runtime/client libraries are used.