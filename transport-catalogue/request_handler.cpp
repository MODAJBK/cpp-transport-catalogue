#include "request_handler.h"

RequestHandler::RequestHandler(const TransportCatalogue& catalogue,
                               MapRender& render,
                               const RoutingSettings& routing_settings)
    : catalogue_(catalogue)
    , render_(&render)
    , builder_(catalogue_.GetStopNames().size(), routing_settings){
}

RequestHandler::RequestHandler(const TransportCatalogue& catalogue)
    :catalogue_(catalogue) {
}

std::string RequestHandler::RenderMap() {
    std::set<std::string_view> stop_names = std::move(catalogue_.GetStopNames());
    std::set<std::string_view> route_names = std::move(catalogue_.GetRouteNames());
    std::ostringstream str_out;
    render_->DrawTransportCatalogue(catalogue_, route_names, stop_names, str_out);
    return str_out.str();
}

MapRender& RequestHandler::GetMapRender() const {
    return *render_;
}

void RequestHandler::BuildGraph() {
    std::set<std::string_view> stop_names = std::move(catalogue_.GetStopNames());
    std::set<std::string_view> route_names = std::move(catalogue_.GetRouteNames());
    builder_.BuildGraph(catalogue_, route_names, stop_names);
    auto router = std::make_unique<RouteBuilder::TcRouter>(builder_.GetRouteGraph());
    builder_.SetRouter(std::move(router));
}

void RequestHandler::SetRender(MapRender& render) {
    render_ = &render;
}

RouteBuilder& RequestHandler::GetRouteBuilder() {
    return builder_;
}

const RouteBuilder& RequestHandler::GetRouteBuilder() const {
    return builder_;
}