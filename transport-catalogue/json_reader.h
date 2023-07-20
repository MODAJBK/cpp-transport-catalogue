#pragma once

#include <iostream>
#include <string>
#include <tuple>
#include <vector>
#include <map>
#include <variant>
#include <set>

#include "json.h"
#include "json_builder.h"
#include "geo.h"
#include "domain.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"

enum class OutRequestType {
    ROUTE_INFO,
    STOP_INFO,
    DRAW_MAP,
    GET_ROUTE
};

struct BaseRequestBus {
    BaseRequestBus() = default;
    BaseRequestBus(std::string r_name, bool, 
                   const std::vector<std::string>& b_stops);

    bool operator==(const BaseRequestBus& other) const;
    bool operator!=(const BaseRequestBus& other) const;

    std::string name;
    bool is_roundtrip = false;
    std::vector<std::string> stops;
};

struct BaseRequestStop {
    BaseRequestStop() = default;
    BaseRequestStop(std::string r_name, double latitude, double longetude);

    bool operator==(const BaseRequestStop& other) const;
    bool operator!=(const BaseRequestStop& other) const;

    Stop stop_info;
};

struct BaseRequestDistance {
    BaseRequestDistance() = default;
    BaseRequestDistance(std::string r_name,
                        const std::vector<std::pair<std::string, double>>&);

    bool operator==(const BaseRequestDistance& other) const;
    bool operator!=(const BaseRequestDistance& other) const;

    std::string name;
    std::vector<std::pair<std::string, double>> stop_to_distance;
};

struct StatRequest {
    StatRequest() = default;
    StatRequest(int id, OutRequestType type, std::string name);

    bool operator==(const StatRequest& other) const;
    bool operator!=(const StatRequest& other) const;

    int id = 0;
    OutRequestType type;
    std::string name;
};

using DataForDraw = std::tuple<RenderSettings, std::set<std::string_view>, std::set<std::string_view>>;
using DataForRouteBuilding = std::tuple<RoutingSettings, std::set<std::string_view>, std::set<std::string_view>>;

class JsonReader {
public:

    JsonReader() = default;

    void RequestParsing(std::istream&);

    void CompleteInputRequests(TransportCatalogue&);
    void CompleteOutputRequests(TransportCatalogue&, const RouteBuilder&, const std::string&, std::ostream&) const;

    DataForDraw GetDataForMapDrawing() const;
    DataForRouteBuilding GetDataForRouteBuilding() const;

private:

    std::vector<BaseRequestStop> add_stop_data_;
    std::vector<BaseRequestDistance> set_distance_data_;
    std::vector<BaseRequestBus> add_bus_data_;
    std::vector<StatRequest> stat_requests_data_;

    RenderSettings render_settings_;
    RoutingSettings routings_settings_;
    std::set<std::string_view> stop_names_;
    std::set<std::string_view> route_names_;

    BaseRequestBus ParseAddBus(const json::Dict&) const;
    std::pair<BaseRequestStop, BaseRequestDistance> ParseAddStop(const json::Dict&) const;
    
    StatRequest ParseGetBusInfo(const json::Dict&) const;
    StatRequest ParseGetStopInfo(const json::Dict&) const;
    StatRequest ParseDrawMap(const json::Dict&) const;
    StatRequest ParseGetRoute(const json::Dict&) const;

    RoutingSettings ParseRoutingSettings(const json::Dict&) const;
    RenderSettings ParseRenderSettings(const json::Dict&) const;
    svg::Color ParseColor(const json::Node&) const;

    void CompleteAddStop(TransportCatalogue&, const BaseRequestStop&) const;
    void CompleteSetDistance(TransportCatalogue&, const BaseRequestDistance&) const;
    void CompleteAddBus(TransportCatalogue&, const BaseRequestBus&);

    json::Document GetRouteRequestResult(TransportCatalogue&, const StatRequest&) const;
    json::Document GetStopRequestResult(TransportCatalogue&, const StatRequest&) const;
    json::Document GetMapDrawingResult(const std::string&, const StatRequest&) const;
    json::Document GetRouteBuildingResult(const RouteBuilder&, const StatRequest&) const;
};