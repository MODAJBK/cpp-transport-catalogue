#include "json_reader.h"

using namespace std::string_literals;
using namespace json;

//-------------------------JsonReader-------------------------

JsonReader::JsonReader(std::istream& input)
    : requests_data_(std::move(json::Load(input)))
{
}

SerializeSettings JsonReader::GetSerializationSettings() const {
    SerializeSettings settings;
    const auto& serialize_settings = requests_data_.GetRoot().AsMap().at("serialization_settings"s).AsMap();
    settings.path_to_db = serialize_settings.at("file"s).AsString();
    return settings;
}

RenderSettings JsonReader::GetRenderSettings() const {
    RenderSettings result;
    const auto& render_settings = requests_data_.GetRoot().AsMap().at("render_settings"s).AsMap();
    const auto& bus_offsets = render_settings.at("bus_label_offset"s).AsArray();
    const auto& stop_offsets = render_settings.at("stop_label_offset"s).AsArray();
    const auto& colors = render_settings.at("color_palette"s).AsArray();
    result.width = render_settings.at("width"s).AsDouble();
    result.height = render_settings.at("height"s).AsDouble();
    result.padding = render_settings.at("padding"s).AsDouble();
    result.line_width = render_settings.at("line_width"s).AsDouble();
    result.stop_radius = render_settings.at("stop_radius"s).AsDouble();
    result.bus_label_font_size = render_settings.at("bus_label_font_size"s).AsInt();
    result.bus_label_offset = { bus_offsets[0].AsDouble(), bus_offsets[1].AsDouble() };
    result.stop_label_font_size = render_settings.at("stop_label_font_size"s).AsInt();
    result.stop_label_offset = { stop_offsets[0].AsDouble(), stop_offsets[1].AsDouble() };
    result.underlayer_color = ParseColor(render_settings.at("underlayer_color"s));
    result.underlayer_width = render_settings.at("underlayer_width"s).AsDouble();
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

RoutingSettings JsonReader::GetRoutingSettings() const {
    RoutingSettings result;
    const auto& routing_settings = requests_data_.GetRoot().AsMap().at("routing_settings"s).AsMap();
    result.bus_wait_time_min = routing_settings.at("bus_wait_time"s).AsInt();
    result.bus_velocity_kmph = routing_settings.at("bus_velocity"s).AsDouble();
    return result;
}

//-------------------------BaseRequestsProcession-------------------------

void JsonReader::BaseRequestsParsing(TransportCatalogue& catalogue) const {
    const auto& base_requests = requests_data_.GetRoot().AsMap().at("base_requests"s).AsArray();
    std::vector<Dict> add_bus_requests;
    std::vector<Dict> add_stop_requests;
    for (const auto& request : base_requests) {
        const auto& request_data = request.AsMap();
        if (request_data.at("type"s) == "Bus"s) {
            add_bus_requests.push_back(request_data);
        }
        else if (request_data.at("type"s) == "Stop"s) {
            add_stop_requests.push_back(request_data);
        }
        else {
            throw std::invalid_argument("Invalid request type"s);
        }
    }
    CompleteBaseRequests(catalogue, add_stop_requests, add_bus_requests);
}

void JsonReader::CompleteBaseRequests(TransportCatalogue& catalogue,
                                      const std::vector<Dict>& stop_requests,
                                      const std::vector<Dict>& bus_requests) const {
    for (const auto& request : stop_requests) {
        CompleteAddStop(catalogue, request);
    }
    for (const auto& request : stop_requests) {
        CompleteSetDistance(catalogue, request);
    }
    for (const auto& request : bus_requests) {
        CompleteAddBus(catalogue, request);
    }
}

void JsonReader::CompleteAddStop(TransportCatalogue& catalogue, const Dict& request) const {
    Stop stop_to_add(request.at("name"s).AsString(),
                     request.at("latitude"s).AsDouble(),
                     request.at("longitude"s).AsDouble());
    catalogue.AddStop(std::move(stop_to_add));
}

void JsonReader::CompleteSetDistance(TransportCatalogue& catalogue, const Dict& request) const {
    std::string stop_from = request.at("name"s).AsString();
    const auto& distances = request.at("road_distances").AsMap();
    for (const auto& [stop_to, distance] : distances) {
        catalogue.SetDistance(catalogue.FindStop(stop_from)->stop_name,
                              catalogue.FindStop(stop_to)->stop_name,
                              distance.AsDouble());
    }
}

void JsonReader::CompleteAddBus(TransportCatalogue& catalogue, const Dict& request) const {
    std::string route_name = request.at("name"s).AsString();
    RouteType route_type = request.at("is_roundtrip"s).AsBool() ? RouteType::RING_ROUTE 
                                                                : RouteType::LINER_ROUTE;
    std::vector<std::string_view> route_stops;
    const auto& stops = request.at("stops"s).AsArray();
    for (const auto& stop : stops) {
        route_stops.push_back(catalogue.FindStop(stop.AsString())->stop_name);
    }
    catalogue.AddBus(route_name, route_type, route_stops);
}

//-------------------------StatRequestsProcession-------------------------

void JsonReader::StatRequestsParsing(const TransportCatalogue& catalogue, const std::string& map, 
                                     const RouteBuilder& route_builder, std::ostream& out) const {
    const auto& stat_requests = requests_data_.GetRoot().AsMap().at("stat_requests"s).AsArray();
    Builder out_json_builder; 
    out_json_builder.StartArray();
    for (const auto& request : stat_requests) {
        const auto& request_data = request.AsMap();
        if (request_data.at("type"s) == "Bus"s) {
            out_json_builder.Value(GetRouteRequestResult(catalogue, request_data));
        }
        else if (request_data.at("type"s) == "Stop"s) {
            out_json_builder.Value(GetStopRequestResult(catalogue, request_data));
        }
        else if (request_data.at("type"s) == "Map"s) {
            out_json_builder.Value(GetMapDrawingResult(map, request_data));
        }
        else if (request_data.at("type"s) == "Route"s) {
            out_json_builder.Value(GetRouteBuildingResult(route_builder, request_data));
        }
        else {
            throw std::invalid_argument("Invalid request type"s);
        }
    }
    out_json_builder.EndArray();
    Print(Document(out_json_builder.Build()), out);
}

Node::Value JsonReader::GetRouteRequestResult(const TransportCatalogue& catalogue, const Dict& request) const {
    auto result = catalogue.GetBusInfo(request.at("name"s).AsString());
    Node json_out;
    if (!result.has_value()) {
        json_out = Builder{}.StartDict()
                            .Key("request_id"s).Value(request.at("id"s).AsInt())
                            .Key("error_message"s).Value("not found"s)
                            .EndDict().Build();
        return json_out.GetValue();
    }
    auto& [stops, unique_stops, distance, curvature] = result.value();
    json_out = Builder{}.StartDict()
                        .Key("curvature"s).Value(curvature)
                        .Key("request_id"s).Value(request.at("id"s).AsInt())
                        .Key("route_length"s).Value(distance)
                        .Key("stop_count"s).Value(static_cast<int>(stops))
                        .Key("unique_stop_count"s).Value(static_cast<int>(unique_stops))
                        .EndDict().Build();
    return json_out.GetValue();
}

Node::Value JsonReader::GetStopRequestResult(const TransportCatalogue& catalogue, const Dict& request) const {
    auto routes = catalogue.GetStopInfo(request.at("name"s).AsString());
    Node json_out;
    if (!routes.has_value()) {
        json_out = Builder{}.StartDict()
                            .Key("request_id"s).Value(request.at("id"s).AsInt())
                            .Key("error_message"s).Value("not found"s)
                            .EndDict().Build();
        return json_out.GetValue();
    }
    Builder builder;
    builder.StartDict().Key("buses"s).StartArray();
    for (const auto& route : routes.value()) {
        builder.Value(route);
    }
    builder.EndArray().Key("request_id"s).Value(request.at("id"s).AsInt()).EndDict();
    json_out = builder.Build();
    return json_out.GetValue();
}

json::Node::Value JsonReader::GetMapDrawingResult(const std::string& map, const json::Dict& request) const {
    Node json_out = Builder{}.StartDict()
                             .Key("map"s).Value(map)
                             .Key("request_id"s).Value(request.at("id"s).AsInt())
                             .EndDict().Build();
    return json_out.GetValue();
}

json::Node::Value JsonReader::GetRouteBuildingResult(const RouteBuilder& route_builder, const json::Dict& request) const {
    Node json_out;
    const auto& graph = route_builder.GetRouteGraph();
    const std::string stop_from = request.at("from"s).AsString();
    const std::string stop_to = request.at("to"s).AsString();
    auto result = route_builder.BuildRouteBetweenTwoStops(stop_from, stop_to);
    if (!result.has_value()) {
        json_out = Builder{}.StartDict()
                            .Key("request_id"s).Value(request.at("id"s).AsInt())
                            .Key("error_message"s).Value("not found"s)
                            .EndDict().Build();
        return json_out.GetValue();
    }
    Builder json_builder;
    json_builder.StartDict()
                .Key("request_id"s).Value(request.at("id"s).AsInt())
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
    return json_out.GetValue();
}