#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <tuple>
#include <variant>

#include "json.h"
#include "geo.h"
#include "domain.h"
#include "map_renderer.h"

enum class OutRequestType {
    ROUTE_INFO,
    STOP_INFO,
    DRAW_MAP
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

using BaseRequest = std::variant<BaseRequestStop, BaseRequestDistance, BaseRequestBus>;
using InputData = std::tuple<std::vector<BaseRequest>, std::vector<StatRequest>, RenderSettings>;

InputData RequestsParsing(std::istream&);

BaseRequestBus ParseAddBus(const json::Dict&);
std::pair<BaseRequestStop, BaseRequestDistance> ParseAddStop(const json::Dict&);
StatRequest ParseGetBusInfo(const json::Dict&);
StatRequest ParseGetStopInfo(const json::Dict&);
StatRequest ParseDrawMap(const json::Dict&);
RenderSettings ParseRenderSettings(const json::Dict&);
svg::Color ParseColor(const json::Node&);