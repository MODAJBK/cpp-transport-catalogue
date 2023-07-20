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

//-------------------------JsonReader-------------------------

void JsonReader::RequestParsing(std::istream& input) {
    const Document json_doc = std::move(Load(input));
    const auto& input_requests = json_doc.GetRoot().AsMap().at("base_requests"s).AsArray();
    const auto& output_requests = json_doc.GetRoot().AsMap().at("stat_requests"s).AsArray();
    const auto& render_settings = json_doc.GetRoot().AsMap().at("render_settings"s).AsMap();
    const auto& routing_settings = json_doc.GetRoot().AsMap().at("routing_settings"s).AsMap();
    for (const auto& request : input_requests) {
        if (request.AsMap().at("type"s) == "Bus"s) add_bus_data_.push_back(std::move(ParseAddBus(request.AsMap())));
        else {
            auto [add_stop, set_distance] = ParseAddStop(request.AsMap());
            add_stop_data_.push_back(std::move(add_stop));
            set_distance_data_.push_back(std::move(set_distance));
        }
    }
    for (const auto& request : output_requests) {
        if (request.AsMap().at("type"s) == "Bus"s) stat_requests_data_.push_back(std::move(ParseGetBusInfo(request.AsMap())));
        else if (request.AsMap().at("type"s) == "Stop"s) stat_requests_data_.push_back(std::move(ParseGetStopInfo(request.AsMap())));
        else if (request.AsMap().at("type"s) == "Route"s) stat_requests_data_.push_back(std::move(ParseGetRoute(request.AsMap())));
        else stat_requests_data_.push_back(std::move(ParseDrawMap(request.AsMap())));
    }
    render_settings_ = std::move(ParseRenderSettings(render_settings));
    routings_settings_ = std::move(ParseRoutingSettings(routing_settings));
}

RoutingSettings JsonReader::ParseRoutingSettings(const json::Dict& request) const {
    RoutingSettings result;
    result.bus_wait_time_min = request.at("bus_wait_time"s).AsInt();
    result.bus_velocity_kmph = request.at("bus_velocity"s).AsDouble();
    return result;
}

RenderSettings JsonReader::ParseRenderSettings(const json::Dict& request) const {
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

svg::Color JsonReader::ParseColor(const json::Node& color) const {
    if (color.IsArray()) {
        const auto& color_arr = color.AsArray();
        if (color_arr.size() == 3) {
            return svg::Rgb(color_arr[0].AsInt(), color_arr[1].AsInt(), color_arr[2].AsInt());
        }
        return svg::Rgba(color_arr[0].AsInt(), color_arr[1].AsInt(), color_arr[2].AsInt(), color_arr[3].AsDouble());
    }
    return color.AsString();
}

BaseRequestBus JsonReader::ParseAddBus(const Dict& request) const {
    BaseRequestBus result;
    result.name = request.at("name"s).AsString();
    result.is_roundtrip = request.at("is_roundtrip"s).AsBool();
    auto& stops = request.at("stops"s).AsArray();
    for (const auto& stop : stops) {
        result.stops.push_back(stop.AsString());
    }
    return result;
}

std::pair<BaseRequestStop, BaseRequestDistance> JsonReader::ParseAddStop(const Dict& request) const {
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

StatRequest JsonReader::ParseGetBusInfo(const Dict& request) const {
    return { request.at("id"s).AsInt(), OutRequestType::ROUTE_INFO, request.at("name"s).AsString() };
}

StatRequest JsonReader::ParseGetStopInfo(const Dict& request) const {
    return { request.at("id"s).AsInt(), OutRequestType::STOP_INFO, request.at("name"s).AsString() };
}

StatRequest JsonReader::ParseDrawMap(const Dict& request) const {
    return { request.at("id"s).AsInt(), OutRequestType::DRAW_MAP, std::string() };
}

StatRequest JsonReader::ParseGetRoute(const json::Dict& request) const {
    return { request.at("id"s).AsInt(), OutRequestType::GET_ROUTE,
             std::string(request.at("from"s).AsString() + '_' + request.at("to"s).AsString()) };
}

DataForDraw JsonReader::GetDataForMapDrawing() const {
    return { render_settings_, stop_names_, route_names_ };
}

DataForRouteBuilding JsonReader::GetDataForRouteBuilding() const {
    return { routings_settings_, stop_names_, route_names_ };
}

void JsonReader::CompleteInputRequests(TransportCatalogue& catalogue) {
    for (const auto& request : add_stop_data_) {
        CompleteAddStop(catalogue, request);
    }
    for (const auto& request : set_distance_data_) {
        CompleteSetDistance(catalogue, request);
    }
    for (const auto& request : add_bus_data_) {
        CompleteAddBus(catalogue, request);
    }
}

void JsonReader::CompleteAddStop(TransportCatalogue& catalogue,
    const BaseRequestStop& request) const {
    catalogue.AddStop(request.stop_info);
}

void JsonReader::CompleteSetDistance(TransportCatalogue& catalogue,
    const BaseRequestDistance& request) const {
    for (const auto& [stop, distance] : request.stop_to_distance) {
        catalogue.SetDistance(catalogue.FindStop(request.name)->stop_name,
                              catalogue.FindStop(stop)->stop_name, distance);
    }
}

void JsonReader::CompleteAddBus(TransportCatalogue& catalogue,
    const BaseRequestBus& request) {
    std::vector<std::string_view> bus_stops;
    std::for_each(request.stops.begin(), request.stops.end(),
                  [&](const auto& stop) { 
                      bus_stops.push_back(catalogue.FindStop(stop)->stop_name);
                      stop_names_.insert(catalogue.FindStop(stop)->stop_name); });
    route_names_.insert(request.name);
    catalogue.AddBus(request.name,
                     request.is_roundtrip ? RouteType::RING_ROUTE : RouteType::LINER_ROUTE,
                     bus_stops);
}

void JsonReader::CompleteOutputRequests(TransportCatalogue& catalogue, const RouteBuilder& route_builder,
                                        const std::string& complete_map, std::ostream& out) const {
    out << '[' << '\n';
    size_t size = stat_requests_data_.size();
    for (size_t i = 0; i < size; ++i) {
        switch (stat_requests_data_[i].type) {
        case OutRequestType::ROUTE_INFO:
            Print(GetRouteRequestResult(catalogue, stat_requests_data_[i]), out);
            break;
        case OutRequestType::STOP_INFO:
            Print(GetStopRequestResult(catalogue, stat_requests_data_[i]), out);
            break;
        case OutRequestType::DRAW_MAP:
            Print(GetMapDrawingResult(complete_map, stat_requests_data_[i]), out);
            break;
        case OutRequestType::GET_ROUTE:
            Print(GetRouteBuildingResult(route_builder, stat_requests_data_[i]), out);
            break;
        }
        if (i != size - 1) out << ",\n";
    }
    out << '\n' << ']' << '\n';
}

Document JsonReader::GetRouteRequestResult(TransportCatalogue& catalogue, const StatRequest& request) const {
    auto result = catalogue.GetBusInfo(request.name);
    Node json_out;
    if (!result.has_value()) {
        json_out = Builder{}.StartDict()
                            .Key("request_id"s).Value(request.id)
                            .Key("error_message"s).Value("not found"s)
                            .EndDict()
                            .Build();
        return Document(json_out);
    }
    auto& [stops, unique_stops, distance, curvature] = result.value();
    json_out = Builder{}.StartDict()
                        .Key("curvature"s).Value(curvature)
                        .Key("request_id"s).Value(request.id)
                        .Key("route_length"s).Value(distance)
                        .Key("stop_count"s).Value(static_cast<int>(stops))
                        .Key("unique_stop_count"s).Value(static_cast<int>(unique_stops))
                        .EndDict()
                        .Build();
    return Document(json_out);
}

Document JsonReader::GetStopRequestResult(TransportCatalogue& catalogue, const StatRequest& request) const {
    auto routes = catalogue.GetStopInfo(request.name);
    Node json_out;
    if (!routes.has_value()) {
        json_out = Builder{}.StartDict()
                            .Key("request_id"s).Value(request.id)
                            .Key("error_message"s).Value("not found"s)
                            .EndDict()
                            .Build();
        return Document(json_out);
    }
    Builder builder;
    builder.StartDict().Key("buses"s).StartArray();
    for (const auto& route : routes.value()) {
        builder.Value(route);
    }
    builder.EndArray().Key("request_id"s).Value(request.id).EndDict();
    json_out = builder.Build();
    return Document(json_out);
}

json::Document JsonReader::GetMapDrawingResult(const std::string& complete_map, const StatRequest& request) const {
    Node json_out = Builder{}.StartDict()
                             .Key("map"s).Value(complete_map)
                             .Key("request_id"s).Value(request.id)
                             .EndDict()
                             .Build();
    return Document(json_out);
}

Document JsonReader::GetRouteBuildingResult(const RouteBuilder& route_builder, const StatRequest& request) const {
    Node json_out;
    const auto& graph = route_builder.GetRouteGraph();
    auto result = route_builder.BuildRouteBetweenTwoStops(request.name.substr(0, request.name.find('_')),
                                                          request.name.substr(request.name.find('_') + 1, request.name.npos));
    if (!result.has_value()) {
        json_out = Builder{}.StartDict()
                            .Key("request_id"s).Value(request.id)
                            .Key("error_message"s).Value("not found"s)
                            .EndDict()
                            .Build();
        return Document(json_out);
    }
    Builder json_builder;
    json_builder.StartDict()
                .Key("request_id"s).Value(request.id)
                .Key("total_time"s).Value(result.value().weight.spend_time)
                .Key("items"s).StartArray();
    for (const auto edge_id : result.value().edges) {
        const auto& edge = graph.GetEdge(edge_id);
        if (edge.weight.span_count == 0) {
            json_builder.StartDict()
                        .Key("type"s).Value("Wait"s)
                        .Key("stop_name").Value(std::string(edge.weight.edge_type))
                        .Key("time"s).Value(edge.weight.spend_time)
                        .EndDict();
        }
        else {
            json_builder.StartDict()
                .Key("type"s).Value("Bus"s)
                .Key("bus").Value(std::string(edge.weight.edge_type))
                .Key("span_count"s).Value(edge.weight.span_count)
                .Key("time"s).Value(edge.weight.spend_time)
                .EndDict();
        }
    }
    json_out = json_builder.EndArray().EndDict().Build();
    return Document(json_out);
}