#include <cassert>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
#include "map_producer.hpp"

using map_producer::MapDefinition;

/*
1. Examine horizontal and vertical neighbors of each free map cell.
2. Return true when at least one 4-connected pair is free.
3. Return false when no such pair exists.
*/
bool has_two_4_connected_free_cells(const MapDefinition& map) {
    // Associated with Step 1: Visit each cell and only forward neighbors.
    for (std::size_t y = 0; y < map.height; ++y) {
        for (std::size_t x = 0; x < map.width; ++x) {
            const std::size_t index = y * map.width + x;
            if (!map_producer::is_free(map.data[index])) {
                continue;
            }
            // Associated with Step 2: Check right and lower 4-connected neighbors.
            if (x + 1 < map.width && map_producer::is_free(map.data[index + 1])) {
                return true;
            }
            if (y + 1 < map.height &&
                map_producer::is_free(map.data[index + map.width])) {
                return true;
            }
        }
    }
    // Associated with Step 3: No free adjacent pair was found.
    return false;
}

/*
1. Create the supplied default map.
2. Check its required metadata and row-major dimensions.
3. Check its occupancy semantics and minimum free-cell requirement.
*/
void test_default_map_definition() {
    // Associated with Step 1: Construct the reusable default map.
    const MapDefinition map = map_producer::make_default_map();

    // Associated with Step 2: Verify nontrivial metadata and exact cell count.
    assert(map.frame_id == "map");
    assert(map.resolution == 0.5);
    assert(map.origin_x == -1.0);
    assert(map.origin_y == 2.0);
    assert(map.width > 0 && map.height > 0);
    assert(map.data.size() == map.width * map.height);
    assert(map_producer::is_valid(map));

    // Associated with Step 3: Verify documented occupancy classifications.
    assert(map_producer::is_free(0));
    assert(map_producer::is_free(49));
    assert(map_producer::is_blocked(-1));
    assert(map_producer::is_blocked(50));
    assert(map_producer::is_blocked(100));
    assert(has_two_4_connected_free_cells(map));
}

/*
1. Serialize a custom valid map with non-default metadata.
2. Verify all required OccupancyGrid-like fields and identity orientation.
3. Verify data order is retained as data[y * width + x].
*/
void test_serialization_preserves_metadata_and_row_major_data() {
    // Associated with Step 1: Define metadata independent of the producer default.
    MapDefinition map;
    map.frame_id = "custom_map";
    map.resolution = 0.25;
    map.width = 3;
    map.height = 2;
    map.origin_x = -3.5;
    map.origin_y = 7.25;
    map.origin_z = 1.0;
    map.data = {0, 50, -1, 49, 100, 0};
    const middleware::JsonValue message = map_producer::to_occupancy_grid(map);

    // Associated with Step 2: Verify required structure and fixed orientation.
    assert(message["header"]["frame_id"].as_string() == "custom_map");
    assert(message["info"]["resolution"].as_number() == 0.25);
    assert(message["info"]["width"].as_int() == 3);
    assert(message["info"]["height"].as_int() == 2);
    assert(message["info"]["origin"]["position"]["x"].as_number() == -3.5);
    assert(message["info"]["origin"]["position"]["y"].as_number() == 7.25);
    assert(message["info"]["origin"]["position"]["z"].as_number() == 1.0);
    assert(message["info"]["origin"]["orientation"]["x"].as_int() == 0);
    assert(message["info"]["origin"]["orientation"]["y"].as_int() == 0);
    assert(message["info"]["origin"]["orientation"]["z"].as_int() == 0);
    assert(message["info"]["origin"]["orientation"]["w"].as_int() == 1);

    // Associated with Step 3: Verify exact row-major data sequence.
    const std::vector<middleware::JsonValue>& data = message["data"].as_array();
    assert(data.size() == 6);
    for (std::size_t index = 0; index < data.size(); ++index) {
        assert(data[index].as_int() == map.data[index]);
    }
    assert(data[1].as_int() == 50); // data[0 * width + 1]
    assert(data[3].as_int() == 49); // data[1 * width + 0]
}

/*
1. Construct map definitions that violate one invariant each.
2. Verify validation rejects each malformed definition.
3. Verify serialization refuses malformed input.
*/
void test_invalid_maps_are_rejected() {
    // Associated with Step 1: Start from a valid map and introduce invalid fields.
    MapDefinition invalid = map_producer::make_default_map();
    invalid.width = 0;

    // Associated with Step 2: Validate dimension, resolution, origin, count, and overflow failures.
    assert(!map_producer::is_valid(invalid));
    invalid = map_producer::make_default_map();
    invalid.resolution = 0.0;
    assert(!map_producer::is_valid(invalid));
    invalid = map_producer::make_default_map();
    invalid.origin_y = std::numeric_limits<double>::infinity();
    assert(!map_producer::is_valid(invalid));
    invalid = map_producer::make_default_map();
    invalid.data.pop_back();
    assert(!map_producer::is_valid(invalid));
    invalid = map_producer::make_default_map();
    invalid.width = std::numeric_limits<std::size_t>::max();
    invalid.height = 2;
    assert(!map_producer::is_valid(invalid));

    // Associated with Step 3: Ensure malformed definitions cannot become publications.
    bool threw = false;
    try {
        map_producer::to_occupancy_grid(invalid);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw);
}

/*
1. Subscribe to /map through generic middleware.
2. Publish a custom valid map through the reusable producer.
3. Verify the subscriber receives the serialized map unchanged.
*/
void test_publish_map_uses_generic_middleware() {
    // Associated with Step 1: Create a normal topic subscriber.
    middleware::Middleware middleware;
    std::vector<middleware::JsonValue> received;
    const middleware::SubscriptionId subscription = middleware.subscribe(
        "/map", [&](const middleware::JsonValue& message) { received.push_back(message); });

    // Associated with Step 2: Publish independent map metadata and data.
    MapDefinition map;
    map.frame_id = "external_style";
    map.resolution = 2.0;
    map.width = 2;
    map.height = 2;
    map.origin_x = 8.0;
    map.origin_y = -4.0;
    map.data = {0, 100, -1, 49};
    map_producer::publish_map(middleware, map);

    // Associated with Step 3: Verify exactly one ordinary topic delivery.
    assert(received.size() == 1);
    assert(received[0]["header"]["frame_id"].as_string() == "external_style");
    assert(received[0]["info"]["resolution"].as_number() == 2.0);
    assert(received[0]["info"]["origin"]["position"]["x"].as_number() == 8.0);
    assert(received[0]["data"].as_array()[2].as_int() == -1);
    assert(middleware.unsubscribe("/map", subscription));
}

/*
1. Execute all map producer tests.
2. Print a success message after assertions pass.
*/
int main() {
    // Associated with Step 1: Run focused component and middleware integration tests.
    test_default_map_definition();
    test_serialization_preserves_metadata_and_row_major_data();
    test_invalid_maps_are_rejected();
    test_publish_map_uses_generic_middleware();

    // Associated with Step 2: Report deterministic test completion.
    std::cout << "All map producer tests passed." << std::endl;
    return 0;
}
