#include "serialization.h"


Serializer::Serializer(SerializeSettings settings)
    : settings_(std::move(settings)) {
}

void Serializer::SetSettings(const SerializeSettings& settings) {
    settings_ = settings;
}

//-------------------------Serialize-------------------------

void Serializer::SaveToFile(TransportCatalogue& catalogue, 
                            const RenderSettings& settings, 
                            const RouteBuilder& builder) {
    std::ofstream out_file(settings_.path_to_db, std::ios::binary);
    *proto_data_.mutable_catalogue() = SerializeCatalogue(catalogue);
    *proto_data_.mutable_render_settings() = SerializeRenderSettings(settings);
    *proto_data_.mutable_route_builder() = SerializeCatalogueRouter(builder);
    proto_data_.SerializeToOstream(&out_file);
}

tc_serialize::TransportCatalogue Serializer::SerializeCatalogue(TransportCatalogue& catalogue) const {
    tc_serialize::TransportCatalogue proto_catalogue;
    const auto& stops = catalogue.GetStops();
    const auto& routes = catalogue.GetRoutes();
    const auto& distances = catalogue.GetDistances();
    for (const auto& stop : stops) {
        *proto_catalogue.add_stops() = SerializeStop(stop);
    }
    for (const auto& [stops, distance] : distances) {
        const auto& [stop_from, stop_to] = stops;
        *proto_catalogue.add_distances() = SerializeDistance(*stop_from, *stop_to, distance);
    }
    for (const auto& route : routes) {
        *proto_catalogue.add_routes() = SerializeBus(route);
    }
    return proto_catalogue;
}

tc_serialize::Stop Serializer::SerializeStop(const Stop& stop) const {
    tc_serialize::Stop proto_stop;

    proto_stop.set_id(stop.stop_id);
    proto_stop.set_name(stop.stop_name);
    proto_stop.mutable_coordinates()->set_latitude(stop.coordinates.lat);
    proto_stop.mutable_coordinates()->set_longitude(stop.coordinates.lng);
    return proto_stop;
}

tc_serialize::Distances Serializer::SerializeDistance(const Stop& stop_from, 
                                                      const Stop& stop_to, 
                                                      double distance) const {
    tc_serialize::Distances proto_distance;
    proto_distance.set_stop_from_id(stop_from.stop_id);
    proto_distance.set_stop_to_id(stop_to.stop_id);
    proto_distance.set_distance(distance);
    return proto_distance;
}

tc_serialize::Bus Serializer::SerializeBus(const Bus& route) const {
    tc_serialize::Bus proto_bus;
    proto_bus.set_name(route.route_name);
    proto_bus.set_is_roundtrip(route.route_type == RouteType::RING_ROUTE);
    for (const auto& stop : route.route_stops) {
        proto_bus.add_stop_ids(stop->stop_id);
    }
    return proto_bus;
}

map_serialize::RenderSettings Serializer::SerializeRenderSettings(const RenderSettings& settings) const {
    map_serialize::RenderSettings proto_settings;
    proto_settings.set_width(settings.width);
    proto_settings.set_height(settings.height);
    proto_settings.set_padding(settings.padding);
    proto_settings.set_line_width(settings.line_width);
    proto_settings.set_stop_radius(settings.stop_radius);
    proto_settings.set_bus_label_font_size(settings.bus_label_font_size);
    proto_settings.mutable_bus_label_offset()->set_x(settings.bus_label_offset.first);
    proto_settings.mutable_bus_label_offset()->set_y(settings.bus_label_offset.second);
    proto_settings.set_stop_label_font_size(settings.stop_label_font_size);
    proto_settings.mutable_stop_label_offset()->set_x(settings.stop_label_offset.first);
    proto_settings.mutable_stop_label_offset()->set_y(settings.stop_label_offset.second);
    proto_settings.set_underlayer_width(settings.underlayer_width);
    *proto_settings.mutable_underlayer_color() = SerializeColor(settings.underlayer_color);
    for (const auto& color : settings.color_palette) {
        *proto_settings.add_color_palette() = SerializeColor(color);
    }
    return proto_settings;
}

svg_serialize::Color Serializer::SerializeColor(const svg::Color& color) const {
    svg_serialize::Color proto_color;
    if (std::holds_alternative<std::string>(color)) {
        proto_color.set_string_color(std::get<std::string>(color));
    }
    else if (std::holds_alternative<svg::Rgb>(color)) {
        auto& rgb = std::get<svg::Rgb>(color);
        proto_color.mutable_rgb_color()->set_r(rgb.red);
        proto_color.mutable_rgb_color()->set_g(rgb.green);
        proto_color.mutable_rgb_color()->set_b(rgb.blue);
    }
    else if (std::holds_alternative<svg::Rgba>(color)) {
        auto& rgba = std::get<svg::Rgba>(color);
        proto_color.mutable_rgba_color()->set_r(rgba.red);
        proto_color.mutable_rgba_color()->set_g(rgba.green);
        proto_color.mutable_rgba_color()->set_b(rgba.blue);
        proto_color.mutable_rgba_color()->set_o(rgba.opacity);
    }
    return proto_color;
}

router_serialize::TransportRouter Serializer::SerializeCatalogueRouter(const RouteBuilder& builder) const {
    router_serialize::TransportRouter proto_router;
    *proto_router.mutable_settings() = SerializeRouterSettings(builder.GetRoutingSettings());
    *proto_router.mutable_graph() = SerializeGraph(builder.GetRouteGraph());
    *proto_router.mutable_router() = SerializeRouter(builder.GetRouter());
    return proto_router;
}

router_serialize::RouteSettings Serializer::SerializeRouterSettings(const RoutingSettings& settings) const {
    router_serialize::RouteSettings proto_settings;
    proto_settings.set_bus_wait_time(settings.bus_wait_time_min);
    proto_settings.set_bus_velocity(settings.bus_velocity_kmph);
    return proto_settings;
}

graph_serialize::DirectedWeightedGraph Serializer::SerializeGraph(const RouteBuilder::RouteGraph& graph) const {
    graph_serialize::DirectedWeightedGraph proto_graph;
    for (const auto& edge : graph.GetEdges()) {
        *proto_graph.add_edges() = SerializeEdge(edge);
    }
    for (const auto& inc_list : graph.GetIncidenceLists()) {
        graph_serialize::IncidenceList* proto_inc_list = proto_graph.add_incidence_list();
        for (const auto edge_id : inc_list) {
            proto_inc_list->add_edges_id(edge_id);
        }
    }
    return proto_graph;
}

graph_serialize::Edge Serializer::SerializeEdge(const RouteBuilder::RouteEdge& edge) const {
    graph_serialize::Edge proto_edge;
    proto_edge.set_vertex_id_from(edge.from);
    proto_edge.set_vertex_id_to(edge.to);
    *proto_edge.mutable_weight() = SerializeWeight(edge.weight);
    return proto_edge;
}

graph_serialize::EdgeWeight Serializer::SerializeWeight(const EdgeWeight& weight) const {
    graph_serialize::EdgeWeight proto_weight;
    proto_weight.set_spend_time(weight.spend_time);
    proto_weight.set_span_count(weight.span_count);
    proto_weight.set_edge_type(weight.edge_type);
    return proto_weight;
}

graph_serialize::Router Serializer::SerializeRouter(const RouteBuilder::TcRouter& router) const {
    graph_serialize::Router proto_router;
    for (const auto& router_data : router.GetRoutesInternalData()) {
        graph_serialize::RoutesInternalData proto_routes_data;
        for (const auto& int_data : router_data) {
            graph_serialize::OptRouteInternalData proto_route_data;
            if (int_data.has_value()) {
                proto_route_data.mutable_data()->mutable_edge_weight()->set_spend_time(int_data.value().weight.spend_time);
                if (int_data.value().prev_edge.has_value()) {
                    proto_route_data.mutable_data()->mutable_prev_edge()->set_edge_id(int_data.value().prev_edge.value());
                }
            }
            *proto_routes_data.add_routes_int_data() = proto_route_data;
        }
        *proto_router.add_router_data() = proto_routes_data;
    }
    return proto_router;
}

//-------------------------Deserialize-------------------------

RenderSettings Serializer::GetFromFile(TransportCatalogue& catalogue, RouteBuilder& builder) {
    std::ifstream in_file(settings_.path_to_db, std::ios::binary);
    proto_data_.ParseFromIstream(&in_file);
    DeserializeCatalogue(catalogue);
    RenderSettings render_settings = DeserializeRenderSettings();
    DeserializeCatalogueRouter(builder);
    return render_settings;
}

void Serializer::DeserializeCatalogue(TransportCatalogue& catalogue) {
    tc_serialize::TransportCatalogue* proto_catalogue = proto_data_.mutable_catalogue();
    for (const auto& stop : proto_catalogue->stops()) {
        catalogue.AddStop(DeserializeStop(stop));
    }
    for (const auto& distance : proto_catalogue->distances()) {
        DeserializeDistance(distance, catalogue);
    }
    for (const auto& route : proto_catalogue->routes()) {
        catalogue.AddBus(DeserializeBus(route, catalogue));
    }
}

Stop Serializer::DeserializeStop(const tc_serialize::Stop& proto_stop) const {
    Stop source_stop;
    source_stop.stop_name = proto_stop.name();
    source_stop.coordinates.lat = proto_stop.coordinates().latitude();
    source_stop.coordinates.lng = proto_stop.coordinates().longitude();
    return source_stop;
}

void Serializer::DeserializeDistance(const tc_serialize::Distances& proto_distance, 
                                     TransportCatalogue& catalogue) const {
    Stop* stop_from = catalogue.FindStop(proto_distance.stop_from_id());
    Stop* stop_to = catalogue.FindStop(proto_distance.stop_to_id());
    double distance = proto_distance.distance();
    catalogue.SetDistance(stop_from, stop_to, distance);
}

Bus Serializer::DeserializeBus(const tc_serialize::Bus& proto_route,
                               const TransportCatalogue& catalogue) const {
    Bus source_route;
    source_route.route_name = proto_route.name();
    source_route.route_type = proto_route.is_roundtrip() ? RouteType::RING_ROUTE
                                                         : RouteType::LINER_ROUTE;
    for (const auto& stop_id : proto_route.stop_ids()) {
        source_route.route_stops.push_back(catalogue.FindStop(stop_id));
    }
    return source_route;
}

RenderSettings Serializer::DeserializeRenderSettings() {
    RenderSettings result;
    map_serialize::RenderSettings* proto_settings = proto_data_.mutable_render_settings();
    result.width = proto_settings->width();
    result.height = proto_settings->height();
    result.padding = proto_settings->padding();
    result.line_width = proto_settings->line_width();
    result.stop_radius = proto_settings->stop_radius();
    result.bus_label_font_size = proto_settings->bus_label_font_size();
    result.bus_label_offset = { proto_settings->mutable_bus_label_offset()->x(),
                                proto_settings->mutable_bus_label_offset()->y() };
    result.stop_label_font_size = proto_settings->stop_label_font_size();
    result.stop_label_offset = { proto_settings->mutable_stop_label_offset()->x(),
                                 proto_settings->mutable_stop_label_offset()->y() };
    result.underlayer_width = proto_settings->underlayer_width();
    result.underlayer_color = DeserializeColor(proto_settings->underlayer_color());
    for (const auto& color : proto_settings->color_palette()) {
        result.color_palette.push_back(DeserializeColor(color));
    }
    return result;
}

svg::Color Serializer::DeserializeColor(const svg_serialize::Color& proto_color) const {
    svg::Color color;
    switch (proto_color.color_case()) {
    case svg_serialize::Color::kStringColor:
    {
        color = proto_color.string_color();
        break;
    }
    case svg_serialize::Color::kRgbColor:
    {
        auto& proto_rgb = proto_color.rgb_color();
        color = svg::Rgb(proto_rgb.r(), proto_rgb.g(), proto_rgb.b());
        break;
    }
    case svg_serialize::Color::kRgbaColor:
    {
        auto& proto_rgba = proto_color.rgba_color();
        color = svg::Rgba(proto_rgba.r(), proto_rgba.g(), proto_rgba.b(), proto_rgba.o());
        break;
    }
    }
    return color;
}

void Serializer::DeserializeCatalogueRouter(RouteBuilder& builder) {
    router_serialize::TransportRouter* proto_route_builder = proto_data_.mutable_route_builder();
    builder.SetRoutingSettings(DeserializeRoutingSettings(proto_route_builder->settings()));
    builder.SetGraph(DeserializeGraph(proto_route_builder->graph()));
    auto builder_ptr = std::move(DeserializeRouter(proto_route_builder->router()));
    builder_ptr->SetGraph(builder.GetRouteGraph());
    builder.SetRouter(std::move(builder_ptr));
}

RoutingSettings Serializer::DeserializeRoutingSettings(const router_serialize::RouteSettings& proto_settings) const {
    RoutingSettings result;
    result.bus_wait_time_min = proto_settings.bus_wait_time();
    result.bus_velocity_kmph = proto_settings.bus_velocity();
    return result;
}

RouteBuilder::RouteGraph Serializer::DeserializeGraph(const graph_serialize::DirectedWeightedGraph& proto_graph) const {
    RouteBuilder::RouteGraph result;
    std::vector<RouteBuilder::RouteEdge> graph_edges;
    std::vector<RouteBuilder::RouteGraph::IncidenceList> graph_incidence_list;
    for (const auto& proto_edge : proto_graph.edges()) {
        RouteBuilder::RouteEdge edge = std::move(DeserializeEdge(proto_edge));
        graph_edges.push_back(std::move(edge));
    }
    for (const auto& proto_incidence_list : proto_graph.incidence_list()) {
        std::vector<graph::EdgeId> id_list;
        for (const auto& proto_edge_id : proto_incidence_list.edges_id()) {
            id_list.push_back(proto_edge_id);
        }
        graph_incidence_list.push_back(std::move(id_list));
    }
    result.SetEdges(graph_edges);
    result.SetIncidenceList(graph_incidence_list);
    return result;
}

RouteBuilder::RouteEdge Serializer::DeserializeEdge(const graph_serialize::Edge& proto_edge) const {
    RouteBuilder::RouteEdge result;
    result.from = proto_edge.vertex_id_from();
    result.to = proto_edge.vertex_id_to();
    result.weight = std::move(DeserializeWeight(proto_edge.weight()));
    return result;
}

EdgeWeight Serializer::DeserializeWeight(const graph_serialize::EdgeWeight& proto_weight) const {
    EdgeWeight result;
    result.spend_time = proto_weight.spend_time();
    result.span_count = proto_weight.span_count();
    result.edge_type = proto_weight.edge_type();
    return result;
}

std::unique_ptr<RouteBuilder::TcRouter> Serializer::DeserializeRouter(const graph_serialize::Router& proto_router) const {
    auto router_ptr = std::make_unique<RouteBuilder::TcRouter>();
    auto& routes_internal_data = router_ptr->GetRoutesInternalData();
    size_t data_size = proto_router.router_data_size();
    routes_internal_data.resize(data_size);
    for (int i = 0; i < data_size; ++i) {
        size_t internal_size = proto_router.router_data(i).routes_int_data_size();
        routes_internal_data[i].resize(internal_size);
        for (int j = 0; j < internal_size; ++j) {
            if (proto_router.router_data(i).routes_int_data(j).has_data()) {
                RouteBuilder::TcRouter::RouteInternalData int_data;
                auto& proto_data = proto_router.router_data(i).routes_int_data(j).data();
                int_data.weight.spend_time = proto_data.edge_weight().spend_time();
                if (proto_data.has_prev_edge()) {
                    int_data.prev_edge = proto_data.prev_edge().edge_id();
                }
                else {
                    int_data.prev_edge = {};
                }
                routes_internal_data[i][j] = std::move(int_data);
            }
            else {
                routes_internal_data[i][j] = {};
            }
        }
    }
    return router_ptr;
}