#include "json_reader.h"

using namespace json;
using namespace std::string_literals;

//-------------------------BaseRequests-------------------------

BaseRequestBus::BaseRequestBus(std::string r_name, bool route_type,
                               const std::vector<std::string>& b_stops) 
    : name(std::move(r_name))
    , is_roundtrip(route_type)
    , stops(b_stops)
{}

bool BaseRequestBus::operator==(const BaseRequestBus& other) const {
    return name == other.name && stops == other.stops && is_roundtrip == other.is_roundtrip;
}

bool BaseRequestBus::operator!=(const BaseRequestBus& other) const {
    return !(*this == other);
}

BaseRequestStop::BaseRequestStop(std::string r_name, double latitude, double longetude)
    : stop_info(r_name, latitude, longetude)
{}

bool BaseRequestStop::operator==(const BaseRequestStop& other) const {
    return stop_info == other.stop_info;
}

bool BaseRequestStop::operator!=(const BaseRequestStop& other) const {
    return !(*this == other);
}

BaseRequestDistance::BaseRequestDistance(std::string r_name,
                                         const std::vector<std::pair<std::string, double>>& distances) 
    : name(std::move(r_name))
    , stop_to_distance(distances)
{}

bool BaseRequestDistance::operator==(const BaseRequestDistance& other) const {
    return name == other.name && stop_to_distance == other.stop_to_distance;
}

bool BaseRequestDistance::operator!=(const BaseRequestDistance& other) const {
    return !(*this == other);
}

//-------------------------StatRequest-------------------------

StatRequest::StatRequest(int r_id, OutRequestType r_type, std::string r_name) 
    : id(r_id)
    , type(r_type)
    , name(r_name)
{}

bool StatRequest::operator==(const StatRequest& other) const {
    return id == other.id && type == other.type && name == other.name;
}

bool StatRequest::operator!=(const StatRequest& other) const {
    return !(*this == other);
}

//-------------------------RequestParsing-------------------------

InputData RequestsParsing(std::istream& input) {
    std::vector<BaseRequest> base_requests;
    std::vector<StatRequest> stat_requests;
    const Document json_doc = std::move(Load(input));
    const auto& requests = json_doc.GetRoot().AsMap();
    const auto& input_requests = requests.at("base_requests"s).AsArray();
    const auto& render_settings = requests.at("render_settings"s).AsMap();
    const auto& output_requests = requests.at("stat_requests"s).AsArray();
    for (const auto& request : input_requests) {
        if (request.AsMap().at("type"s) == "Bus"s) base_requests.push_back(std::move(ParseAddBus(request.AsMap())));
        else { 
            auto [add_stop, set_distance] = std::move(ParseAddStop(request.AsMap()));
            base_requests.push_back(std::move(add_stop));
            base_requests.push_back(std::move(set_distance));
        }
    }
    for (const auto& request : output_requests) {
        if (request.AsMap().at("type"s) == "Bus"s) stat_requests.push_back(std::move(ParseGetBusInfo(request.AsMap())));
        else if (request.AsMap().at("type"s) == "Stop"s) stat_requests.push_back(std::move(ParseGetStopInfo(request.AsMap())));
        else stat_requests.push_back(std::move(ParseDrawMap(request.AsMap())));
    }
    return { base_requests, stat_requests, ParseRenderSettings(render_settings) };
}

RenderSettings ParseRenderSettings(const json::Dict& request) {
    RenderSettings result;
    const auto& bus_offsets = request.at("bus_label_offset"s).AsArray();
    const auto& stop_offsets = request.at("stop_label_offset"s).AsArray();
    const auto& colors = request.at("color_palette"s).AsArray();
    result.width = request.at("width"s).AsDouble();
    result.height = request.at("height"s).AsDouble();
    result.padding = request.at("padding"s).AsDouble();
    result.line_width = request.at("line_width"s).AsDouble();
    result.stop_radius = request.at("stop_radius"s).AsDouble();
    result.bus_label_font_size = request.at("bus_label_font_size"s).AsInt();
    result.bus_label_offset = { bus_offsets[0].AsDouble(), bus_offsets[1].AsDouble() };
    result.stop_label_font_size = request.at("stop_label_font_size"s).AsInt();
    result.stop_label_offset = { stop_offsets[0].AsDouble(), stop_offsets[1].AsDouble() };
    result.underlayer_color = ParseColor(request.at("underlayer_color"s));
    result.underlayer_width = request.at("underlayer_width"s).AsDouble();
    for (const auto& color : colors) {
        result.color_palette.push_back(std::move(ParseColor(color)));
    }
    return result;
}

svg::Color ParseColor(const json::Node& color) {
    if (color.IsArray()) {
        const auto& color_arr = color.AsArray();
        if (color_arr.size() == 3) {
            return svg::Rgb(color_arr[0].AsInt(), color_arr[1].AsInt(), color_arr[2].AsInt());
        }
        return svg::Rgba(color_arr[0].AsInt(), color_arr[1].AsInt(), color_arr[2].AsInt(), color_arr[3].AsDouble());
    }
    return color.AsString();
}

BaseRequestBus ParseAddBus(const Dict& request) {
    BaseRequestBus result;
    result.name = request.at("name"s).AsString();
    result.is_roundtrip = request.at("is_roundtrip"s).AsBool();
    auto& stops = request.at("stops"s).AsArray();
    for (const auto& stop : stops) {
        result.stops.push_back(stop.AsString());
    }
    return result;
}

std::pair<BaseRequestStop, BaseRequestDistance> ParseAddStop(const Dict& request) {
    BaseRequestStop add_stop;
    BaseRequestDistance set_distance;
    add_stop.stop_info = Stop(request.at("name"s).AsString(),
                              request.at("latitude"s).AsDouble(),
                              request.at("longitude"s).AsDouble());
    set_distance.name = request.at("name"s).AsString();
    auto& distances = request.at("road_distances").AsMap();
    for (const auto& [stop, distance] : distances) {
        set_distance.stop_to_distance.push_back({ stop, distance.AsDouble() });
    }
    return { add_stop, set_distance };
}

StatRequest ParseGetBusInfo(const Dict& request) {
    return { request.at("id"s).AsInt(), OutRequestType::ROUTE_INFO, request.at("name"s).AsString() };
}

StatRequest ParseGetStopInfo(const Dict& request) {
    return { request.at("id"s).AsInt(), OutRequestType::STOP_INFO, request.at("name"s).AsString() };
}

StatRequest ParseDrawMap(const Dict& request) {
    return { request.at("id"s).AsInt(), OutRequestType::DRAW_MAP, std::string() };
}