#ifndef PATH_PLANNER_HPP
#define PATH_PLANNER_HPP

#include <algorithm>
#include <cmath>
#include <limits>
#include <mutex>
#include <optional>
#include <vector>
#include "min_heap.hpp"
#include "middleware/middleware.hpp"

namespace path_planner {
struct Cell { 
    size_t x, y; 
    bool operator==(const Cell& o) const { 
        return x == o.x && y == o.y; 
    } 
};
struct Map { 
    double resolution, ox, oy; 
    size_t width, height; 
    std::vector<int> data; 
};

/* 1. Validate an OccupancyGrid-like JSON message.
2. Copy its arbitrary metadata and row-major integer cells into a Map.
3. Return false instead of throwing for malformed input. */
inline bool parse_map(const middleware::JsonValue& j, Map& out) {
    try {
        if (!j.is_object() || !j.contains("header") || !j["header"].is_object() ||
            !j["header"].contains("frame_id") || !j["header"]["frame_id"].is_string() ||
            !j.contains("info") || !j["info"].is_object() || !j.contains("data") || !j["data"].is_array()) return false;
        const auto& i=j["info"]; if (!i.contains("resolution") || !i.contains("width") || !i.contains("height") || !i.contains("origin")) return false;
        Map m{}; auto size=[](const middleware::JsonValue& v,size_t& n) {
            return v.is_number() && std::isfinite(v.as_number()) && v.as_number()>0 &&
                   std::floor(v.as_number())==v.as_number() && v.as_number()<=static_cast<double>(std::numeric_limits<size_t>::max()) &&
                   ((n=static_cast<size_t>(v.as_number())),true); };
        if (!i["resolution"].is_number() || !std::isfinite(i["resolution"].as_number()) || i["resolution"].as_number()<=0 ||
            !size(i["width"],m.width) || !size(i["height"],m.height) || m.height>std::numeric_limits<size_t>::max()/m.width) return false;
        const auto& o=i["origin"]; if (!o.is_object() || !o.contains("position") || !o["position"].is_object() || !o.contains("orientation") || !o["orientation"].is_object()) return false;
        const auto& p=o["position"]; const auto& q=o["orientation"]; if (!p.contains("x") || !p.contains("y") || !p.contains("z") || !p["x"].is_number() || !p["y"].is_number() || !p["z"].is_number() ||
            !std::isfinite(p["x"].as_number()) || !std::isfinite(p["y"].as_number()) || !std::isfinite(p["z"].as_number())) return false;
        if(!q.contains("x")||!q.contains("y")||!q.contains("z")||!q.contains("w")||!q["x"].is_number()||!q["y"].is_number()||!q["z"].is_number()||!q["w"].is_number()||
           q["x"].as_number()!=0.0||q["y"].as_number()!=0.0||q["z"].as_number()!=0.0||q["w"].as_number()!=1.0)return false;
        m.resolution=i["resolution"].as_number(); m.ox=p["x"].as_number(); m.oy=p["y"].as_number();
        const auto& cells=j["data"].as_array(); if (cells.size()!=m.width*m.height) return false;
        for(const auto& v:cells) { if(!v.is_number() || !std::isfinite(v.as_number()) || std::floor(v.as_number())!=v.as_number() ||
            v.as_number()<std::numeric_limits<int>::min() || v.as_number()>std::numeric_limits<int>::max()) return false; m.data.push_back(static_cast<int>(v.as_number())); }
        out=std::move(m); return true;
    } catch (...) { return false; }
}

/* 1. Check map bounds.
2. Apply the required free occupancy range [0, 50). */
//is this cell inside the map?
inline bool free_cell(const Map& m, Cell c) {
    return c.x < m.width && c.y < m.height && 
     m.data[c.y * m.width + c.x] >= 0 && 
     m.data[c.y * m.width + c.x] < 50; 
}

/* 1. Use floor-based world-to-cell conversion.
2. Reject nonfinite, negative, or unrepresentable results. */
inline bool world_cell(const Map& m,double x,double y,Cell& c) {
    double a=std::floor((x -  m.ox) / m.resolution), b=std::floor((y - m.oy) / m.resolution);
    if(!std::isfinite(a) || !std::isfinite(b) || a<0 || b<0 || 
        a > static_cast<double>(std::numeric_limits<size_t>::max()) || 
        b > static_cast<double>(std::numeric_limits<size_t>::max())) {
            return false;
        }
    c={static_cast<size_t>(a),static_cast<size_t>(b)}; return true;
}

/* 1. Run A* from free start to free goal.
2. Expand only cardinal free neighbors using Manhattan distance.
3. Reconstruct a shortest ordered path or return empty when unreachable. */
std::vector<Cell> astar(const Map& map, Cell start, Cell goal)
{
    if (!free_cell(map, start) || !free_cell(map, goal)) {
        return {};
    }

    if (start == goal) {
        return {start};
    }

    // The shared project heap is a min-heap and orders nodes
    // by the A* priority.
    struct Node {
        size_t f;
        size_t g;
        Cell cell;
        bool operator<(const Node& other) const {
            if (f != other.f) {
                return f < other.f;
            }
            if (g != other.g) {
                return g < other.g;
            }
            if (cell.y != other.cell.y) {
                return cell.y < other.cell.y;
            }
            return cell.x < other.cell.x;
        }
    };

    const size_t cell_count = map.width * map.height;
    const size_t invalid = std::numeric_limits<size_t>::max();

    std::vector<size_t> cost(cell_count, invalid);
    std::vector<size_t> parent(cell_count, invalid);

    auto cell_id = [&](Cell cell) {
        return cell.y * map.width + cell.x;
    };

    // Manhattan distance: only horizontal and vertical movement.
    auto heuristic = [&](Cell cell) {
        const size_t dx = cell.x > goal.x
            ? cell.x - goal.x
            : goal.x - cell.x;
        const size_t dy = cell.y > goal.y
            ? cell.y - goal.y
            : goal.y - cell.y;
        return dx + dy;
    };

    MinHeap<Node> open_set;

    cost[cell_id(start)] = 0;
    open_set.insert({
        heuristic(start),
        0,
        start
    });

    // Four cardinal directions.
    const int dx[] = {1, -1, 0, 0};
    const int dy[] = {0, 0, 1, -1};

    while (!open_set.empty()) {
        Node current = open_set.extractMin();
        const size_t current_id = cell_id(current.cell);
        // Ignore outdated entries in the heap.
        if (current.g != cost[current_id]) {
            continue;
        }
        // Goal reached: reconstruct the path.
        if (current.cell == goal) {
            std::vector<Cell> path;
            for (size_t id = cell_id(goal); id != invalid; id = parent[id]) {
                path.push_back({
                    id % map.width,
                    id / map.width
                });
            }
            std::reverse(path.begin(), path.end());
            return path;
        }
        // Explore the four neighboring cells.
        for (int direction = 0; direction < 4; ++direction) {
            const long long x =
                static_cast<long long>(current.cell.x) + dx[direction];
            const long long y =
                static_cast<long long>(current.cell.y) + dy[direction];
            // Prevent unsigned coordinates from wrapping around.
            if (x < 0 || y < 0) {
                continue;
            }
            Cell neighbor{
                static_cast<size_t>(x),
                static_cast<size_t>(y)
            };
            if (!free_cell(map, neighbor)) {
                continue;
            }
            const size_t neighbor_id = cell_id(neighbor);
            const size_t new_cost = current.g + 1;
            if (new_cost < cost[neighbor_id]) {
                cost[neighbor_id] = new_cost;
                parent[neighbor_id] = current_id;
                const size_t priority = new_cost + heuristic(neighbor);
                open_set.insert({
                    priority,
                    new_cost,
                    neighbor
                });
            }
        }
    }   
    // No path exists.
    return {};
}

class Planner {
    middleware::Middleware& mw; middleware::SubscriptionId sub; bool advertised; mutable std::mutex lock; std::optional<Map> map;
    /* 1. Parse named request positions and optional numeric tolerance.
    2. Return false for malformed arguments. */
    static bool request(const middleware::JsonValue& a,double& sx,double& sy,double& gx,double& gy) {
        try { if(!a.is_object()||!a.contains("start")||!a.contains("goal"))return false; const auto& s=a["start"]["pose"]["position"];const auto& g=a["goal"]["pose"]["position"];
            if(!s["x"].is_number()||!s["y"].is_number()||!g["x"].is_number()||!g["y"].is_number())return false;sx=s["x"].as_number();sy=s["y"].as_number();gx=g["x"].as_number();gy=g["y"].as_number();
            return std::isfinite(sx)&&std::isfinite(sy)&&std::isfinite(gx)&&std::isfinite(gy)&&(!a.contains("tolerance")||(a["tolerance"].is_number()&&std::isfinite(a["tolerance"].as_number())));
        }catch(...){return false;}
    }
    /* 1. Convert ordered cells to center-positioned identity poses.
    2. Return a Path-like JSON payload. */
    static middleware::JsonValue path(const Map&m,const std::vector<Cell>& cells) {
        auto p=middleware::JsonValue::make_object(),h=middleware::JsonValue::make_object(),ps=middleware::JsonValue::make_array();h["frame_id"]="map";
        for(Cell c:cells){auto x=middleware::JsonValue::make_object(),ph=middleware::JsonValue::make_object(),po=middleware::JsonValue::make_object(),pos=middleware::JsonValue::make_object(),o=middleware::JsonValue::make_object();ph["frame_id"]="map";pos["x"]=m.ox+(c.x+.5)*m.resolution;pos["y"]=m.oy+(c.y+.5)*m.resolution;pos["z"]=0.0;o["x"]=0.0;o["y"]=0.0;o["z"]=0.0;o["w"]=1.0;po["position"]=pos;po["orientation"]=o;x["header"]=ph;x["pose"]=po;ps.push_back(x);}p["header"]=h;p["poses"]=ps;return p;
    }
    /* 1. Snapshot latest map and parse request.
    2. Run A*, publish only successful path, and return values.plan. */
    std::pair<bool,middleware::JsonValue> handle(const middleware::JsonValue&a) {
        Map m; {std::lock_guard<std::mutex> g(lock);if(!map)return {false,middleware::JsonValue::make_object()};m=*map;}double sx,sy,gx,gy;Cell s,t;
        if(!request(a,sx,sy,gx,gy)||!world_cell(m,sx,sy,s)||!world_cell(m,gx,gy,t))return {false,middleware::JsonValue::make_object()};auto cells=astar(m,s,t);if(cells.empty())return {false,middleware::JsonValue::make_object()};
        auto pl=path(m,cells),v=middleware::JsonValue::make_object();v["plan"]=pl;mw.publish("/path",pl);return {true,v};
    }
public:
    /* 1. Subscribe to /map and provide /plan_path through generic middleware. */
    explicit Planner(middleware::Middleware& middleware):mw(middleware) { sub=mw.subscribe("/map",[this](const auto&j){Map m;if(parse_map(j,m)){std::lock_guard<std::mutex>g(lock);map=std::move(m);}});advertised=mw.advertise_service("/plan_path",[this](const auto&a){return handle(a);}); }
    /* 1. Remove planner-owned registrations before middleware destruction. */
    ~Planner(){mw.unsubscribe("/map",sub);if(advertised)mw.unadvertise_service("/plan_path");}
};
}
#endif
