#ifndef MAP_PRODUCER_HPP
#define MAP_PRODUCER_HPP

#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
#include "middleware/middleware.hpp"

namespace map_producer {

struct MapDefinition {
    std::string frame_id{"map"};
    double resolution{0.5};
    std::size_t width{0};
    std::size_t height{0};
    double origin_x{0.0};
    double origin_y{0.0};
    double origin_z{0.0};
    std::vector<int> data;
};

/*
1. Check whether the occupancy value falls in the documented free range.
2. Return true only for values from zero through forty-nine.
*/
inline bool is_free(int occupancy) noexcept {
    // Associated with Step 1 & 2: Apply the free-cell occupancy rule.
    return occupancy >= 0 && occupancy < 50;
}

/*
1. Check whether the occupancy value is free.
2. Return the inverse so unknown and occupied values are blocked.
*/
inline bool is_blocked(int occupancy) noexcept {
    // Associated with Step 1 & 2: Apply the blocked-cell occupancy rule.
    return !is_free(occupancy);
}

/*
1. Reject zero dimensions and nonpositive or nonfinite resolution.
2. Reject nonfinite origin coordinates.
3. Reject dimensions whose product overflows size_t.
4. Verify that data contains exactly one integer occupancy value per cell.
5. Return true only when all OccupancyGrid-like invariants hold.
*/
inline bool is_valid(const MapDefinition& map, std::string* error = nullptr) {
    // Associated with Step 1: Validate dimensions and resolution.
    if (map.width == 0 || map.height == 0) {
        if (error != nullptr) *error = "map width and height must be positive";
        return false;
    }
    if (!std::isfinite(map.resolution) || map.resolution <= 0.0) {
        if (error != nullptr) *error = "map resolution must be finite and positive";
        return false;
    }

    // Associated with Step 2: Validate origin coordinates.
    if (!std::isfinite(map.origin_x) || !std::isfinite(map.origin_y) ||
        !std::isfinite(map.origin_z)) {
        if (error != nullptr) *error = "map origin coordinates must be finite";
        return false;
    }

    // Associated with Step 3: Prevent size_t multiplication overflow.
    if (map.height > std::numeric_limits<std::size_t>::max() / map.width) {
        if (error != nullptr) *error = "map dimensions overflow";
        return false;
    }

    // Associated with Step 4 & 5: Check exact row-major cell count.
    if (map.data.size() != map.width * map.height) {
        if (error != nullptr) *error = "map data length must equal width * height";
        return false;
    }
    return true;
}

/*
1. Validate the supplied map definition.
2. Create header and info objects with the map frame and metadata.
3. Create the required axis-aligned identity origin pose.
4. Copy cells in their existing row-major order into a JSON array.
5. Return a valid OccupancyGrid-like JSON object.
*/
inline middleware::JsonValue to_occupancy_grid(const MapDefinition& map) {
    // Associated with Step 1: Reject invalid map definitions before serialization.
    std::string error;
    if (!is_valid(map, &error)) {
        throw std::invalid_argument(error);
    }

    // Associated with Step 2: Create header and map information objects.
    middleware::JsonValue message = middleware::JsonValue::make_object();
    middleware::JsonValue header = middleware::JsonValue::make_object();
    middleware::JsonValue info = middleware::JsonValue::make_object();
    header["frame_id"] = map.frame_id;
    info["resolution"] = map.resolution;
    info["width"] = map.width;
    info["height"] = map.height;

    // Associated with Step 3: Serialize the identity orientation and origin position.
    middleware::JsonValue origin = middleware::JsonValue::make_object();
    middleware::JsonValue position = middleware::JsonValue::make_object();
    middleware::JsonValue orientation = middleware::JsonValue::make_object();
    position["x"] = map.origin_x;
    position["y"] = map.origin_y;
    position["z"] = map.origin_z;
    orientation["x"] = 0;
    orientation["y"] = 0;
    orientation["z"] = 0;
    orientation["w"] = 1;
    origin["position"] = position;
    origin["orientation"] = orientation;
    info["origin"] = origin;

    // Associated with Step 4: Preserve the map's supplied row-major data layout.
    middleware::JsonValue cells = middleware::JsonValue::make_array();
    for (int occupancy : map.data) {
        cells.push_back(occupancy);
    }
    message["header"] = header;
    message["info"] = info;
    message["data"] = cells;

    // Associated with Step 5: Return complete OccupancyGrid-like payload.
    return message;
}

/*
1. Construct a compact map with nonzero origin and non-unit resolution.
2. Populate row-major occupancy data with free and blocked cells.
3. Return the definition without interpreting it for planning.
*/
inline MapDefinition make_default_map() {
    // Associated with Step 1: Set deliberately general metadata.
    MapDefinition map;
    map.frame_id = "map";
    map.resolution = 0.5;
    map.width = 4;
    map.height = 3;
    map.origin_x = -1.0;
    map.origin_y = 2.0;
    map.origin_z = 0.0;

    // Associated with Step 2: Store cells as data[y * width + x].
    map.data = {
        0, 0, 100, 0,
        0, 50, 0, 0,
        0, 0, 0, -1
    };

    // Associated with Step 3: Return reusable map data only.
    return map;
}

/*
1. Convert a validated map definition into an OccupancyGrid-like message.
2. Publish that message to the ordinary /map middleware topic.
*/
inline void publish_map(middleware::Middleware& mw, const MapDefinition& map) {
    // Associated with Step 1: Serialize only a valid map.
    const middleware::JsonValue message = to_occupancy_grid(map);

    // Associated with Step 2: Delegate delivery to generic middleware.
    mw.publish("/map", message);
}

} // namespace map_producer

#endif // MAP_PRODUCER_HPP
