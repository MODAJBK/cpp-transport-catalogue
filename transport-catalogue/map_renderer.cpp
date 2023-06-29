#include "map_renderer.h"

using namespace svg;
using namespace std::string_literals;

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
    return {
        (coords.lng - min_lon_) * zoom_coeff_ + padding_,
        (max_lat_ - coords.lat) * zoom_coeff_ + padding_
    };
}

svg::Polyline GetMapPicture(const RenderSettings& settings, 
                            SphereProjector& proj, size_t& color_index,
                            std::vector<Stop*>* route_stops, RouteType route_type) {
    Polyline route_map;
    for (const auto stop : *route_stops) {
        route_map.AddPoint(proj(stop->coordinates));
    }
    if (route_type == RouteType::LINER_ROUTE) {
        int size = route_stops->size();
        for (auto index = size - 2; index != -1; --index) {
            route_map.AddPoint(proj(route_stops->at(index)->coordinates));
        }
    }
    route_map.SetStrokeColor(settings.color_palette[color_index]).
              SetFillColor("none"s).
              SetStrokeWidth(settings.line_width).
              SetStrokeLineCap(StrokeLineCap::ROUND).
              SetStrokeLineJoin(StrokeLineJoin::ROUND);
    if (++color_index == settings.color_palette.size()) color_index = 0;
    return route_map;
}

std::pair<svg::Text, svg::Text> GetRouteNamePicture(const RenderSettings& settings, 
                                                    SphereProjector& proj, size_t& color_index,
                                                    const std::string name, geo::Coordinates x_y) {
    Text route_name, route_substrate;
    route_name.SetPosition(proj(x_y)).
               SetOffset(Point(settings.bus_label_offset.first, settings.bus_label_offset.second)).
               SetFontSize(settings.bus_label_font_size).
               SetFontFamily("Verdana"s).
               SetFontWeight("bold"s).
               SetFillColor(settings.color_palette[color_index]).
               SetData(name);
    route_substrate.SetPosition(proj(x_y)).
                    SetOffset(Point(settings.bus_label_offset.first, settings.bus_label_offset.second)).
                    SetFontSize(settings.bus_label_font_size).
                    SetFontFamily("Verdana"s).
                    SetFontWeight("bold"s).
                    SetFillColor(settings.underlayer_color).
                    SetStrokeColor(settings.underlayer_color).
                    SetData(name).
                    SetStrokeWidth(settings.underlayer_width).
                    SetStrokeLineCap(StrokeLineCap::ROUND).
                    SetStrokeLineJoin(StrokeLineJoin::ROUND);
    const auto result = std::move(std::make_pair(route_name, route_substrate));
    return result;
}

svg::Circle GetStopCirclePicture(const RenderSettings& settings, SphereProjector& proj, geo::Coordinates x_y) {
    Circle stop_dot;
    stop_dot.SetCenter(proj(x_y)).
             SetRadius(settings.stop_radius).
             SetFillColor("white"s);
    return stop_dot;
}

std::pair<svg::Text, svg::Text> GetStopNamePicture(const RenderSettings& settings, SphereProjector& proj,
                                                   const std::string& name, geo::Coordinates x_y) {
    Text stop_name, stop_substrate;
    stop_name.SetPosition(proj(x_y)).
              SetOffset(Point(settings.stop_label_offset.first, settings.stop_label_offset.second)).
              SetFontSize(settings.stop_label_font_size).
              SetFontFamily("Verdana"s).
              SetFillColor("black"s).
              SetData(name);
    stop_substrate.SetPosition(proj(x_y)).
                   SetOffset(Point(settings.stop_label_offset.first, settings.stop_label_offset.second)).
                   SetFontSize(settings.stop_label_font_size).
                   SetFontFamily("Verdana"s).
                   SetFillColor(settings.underlayer_color).
                   SetStrokeColor(settings.underlayer_color).
                   SetStrokeWidth(settings.underlayer_width).
                   SetStrokeLineCap(StrokeLineCap::ROUND).
                   SetStrokeLineJoin(StrokeLineJoin::ROUND).
                   SetData(name);
    const auto result = std::move(std::make_pair(stop_name, stop_substrate));
    return result;
}