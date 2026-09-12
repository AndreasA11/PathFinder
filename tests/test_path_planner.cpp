#include <cassert>
#include <iostream>
#include <vector>
#include "path_planner.hpp"

/* 1. Build a valid arbitrary occupancy map JSON payload.
2. Return it for planner integration tests. */
middleware::JsonValue map_message(int w,int h,double r,double ox,double oy,const std::vector<int>& d) {
    auto m=middleware::JsonValue::make_object(),head=middleware::JsonValue::make_object(),i=middleware::JsonValue::make_object(),o=middleware::JsonValue::make_object(),p=middleware::JsonValue::make_object(),q=middleware::JsonValue::make_object(),a=middleware::JsonValue::make_array();
    head["frame_id"]="any";p["x"]=ox;p["y"]=oy;p["z"]=0;q["x"]=0;q["y"]=0;q["z"]=0;q["w"]=1;o["position"]=p;o["orientation"]=q;i["resolution"]=r;i["width"]=w;i["height"]=h;i["origin"]=o;for(int x:d)a.push_back(x);m["header"]=head;m["info"]=i;m["data"]=a;return m;
}
/* 1. Construct named start/goal service arguments.
2. Return positions without requiring irrelevant pose fields. */
middleware::JsonValue request(double sx,double sy,double gx,double gy) {
    auto a=middleware::JsonValue::make_object(),s=middleware::JsonValue::make_object(),g=middleware::JsonValue::make_object(),sp=middleware::JsonValue::make_object(),gp=middleware::JsonValue::make_object(),x=middleware::JsonValue::make_object(),y=middleware::JsonValue::make_object();
    x["x"]=sx;x["y"]=sy;y["x"]=gx;y["y"]=gy;sp["position"]=x;gp["position"]=y;s["pose"]=sp;g["pose"]=gp;a["start"]=s;a["goal"]=g;a["tolerance"]=0.0;return a;
}
/* 1. Verify no-map failure, shortest cardinal planning, and /path publication.
2. Verify map replacement and blocked failures do not publish stale paths. */
int main() {
    middleware::Middleware mw; path_planner::Planner planner(mw); std::vector<middleware::JsonValue> published;
    mw.subscribe("/path",[&](const auto& p){published.push_back(p);});
    assert(!mw.call_service("/plan_path",request(0,0,1,1)).first);
    mw.publish("/map",map_message(3,3,.5,-1,2,{0,0,0,0,100,0,0,0,0}));
    auto ok=mw.call_service("/plan_path",request(-.75,2.25,.25,3.25)); assert(ok.first);
    const auto& poses=ok.second["plan"]["poses"].as_array(); assert(poses.size()==5 && published.size()==1);
    assert(poses.front()["pose"]["position"]["x"].as_number()==-.75);
    assert(poses.back()["pose"]["position"]["y"].as_number()==3.25);
    mw.publish("/map",map_message(2,2,2,10,-4,{0,100,100,0}));
    assert(!mw.call_service("/plan_path",request(11,-3,13,-1)).first && published.size()==1);
    auto one=mw.call_service("/plan_path",request(11,-3,11,-3)); assert(one.first && one.second["plan"]["poses"].as_array().size()==1 && published.size()==2);
    std::cout<<"All path planner tests passed."<<std::endl;
}
