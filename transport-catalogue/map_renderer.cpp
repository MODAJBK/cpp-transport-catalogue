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

//-------------------------MapRender-------------------------

void MapRender::DrawRoutes(TransportCatalogue& catalogue, 
                           const std::set<std::string_view>& route_names) {
    size_t color_index = 0;
    for (const auto route : route_names) {
        const auto [stops, type] = std::move(catalogue.FindBus(route));
        const auto first_stop = stops->at(0);
        const auto last_stop = stops->at(stops->size() - 1);
        if (!stops->empty()) {
            GetRouteMapPicture(color_index, type, stops);
            GetRouteNamePicture(color_index, std::string(route), first_stop->coordinates);
            if (type == RouteType::LINER_ROUTE && first_stop != last_stop) {
                GetRouteNamePicture(color_index, std::string(route), last_stop->coordinates);
            }
        }
        if (++color_index == settings_.color_palette.size()) color_index = 0;
    }
}

void MapRender::DrawStops(TransportCatalogue& catalogue, 
                          const std::set<std::string_view>& stop_names) {
    for (const auto stop : stop_names) {
        const auto x_y = std::move(catalogue.FindStop(stop)->coordinates);
        GetStopCirclePicture(x_y);
        GetStopNamePicture(std::string(stop), x_y);
    }
}

void MapRender::DrawTransportCatalogue(std::ostream& out) {
    Document doc;
    DrawPicture(route_maps_, doc);
    DrawPicture(route_names_, doc);
    DrawPicture(stop_dots_, doc);
    DrawPicture(stop_names_, doc);
    doc.Render(out);
}

void MapRender::GetRouteMapPicture(size_t color_index, RouteType route_type,
                                   std::vector<Stop*>* route_stops) {
    Polyline route_map;
    for (const auto stop : *route_stops) {
        route_map.AddPoint(proj_(stop->coordinates));
    }
    if (route_type == RouteType::LINER_ROUTE) {
        int size = route_stops->size();
        for (auto index = size - 2; index != -1; --index) {
            route_map.AddPoint(proj_(route_stops->at(index)->coordinates));
        }
    }
    route_map.SetStrokeColor(settings_.color_palette[color_index]).
              SetFillColor("none"s).
              SetStrokeWidth(settings_.line_width).
              SetStrokeLineCap(StrokeLineCap::ROUND).
              SetStrokeLineJoin(StrokeLineJoin::ROUND);
    route_maps_.emplace_back(std::move(route_map));
}

void MapRender::GetRouteNamePicture(size_t color_index, const std::string name, 
                                    geo::Coordinates x_y) {
    Text route_name, route_substrate;
    route_name.SetPosition(proj_(x_y)).
               SetOffset(Point(settings_.bus_label_offset.first, settings_.bus_label_offset.second)).
               SetFontSize(settings_.bus_label_font_size).
               SetFontFamily("Verdana"s).
               SetFontWeight("bold"s).
               SetFillColor(settings_.color_palette[color_index]).
               SetData(name);
    route_substrate.SetPosition(proj_(x_y)).
                    SetOffset(Point(settings_.bus_label_offset.first, settings_.bus_label_offset.second)).
                    SetFontSize(settings_.bus_label_font_size).
                    SetFontFamily("Verdana"s).
                    SetFontWeight("bold"s).
                    SetFillColor(settings_.underlayer_color).
                    SetStrokeColor(settings_.underlayer_color).
                    SetData(name).
                    SetStrokeWidth(settings_.underlayer_width).
                    SetStrokeLineCap(StrokeLineCap::ROUND).
                    SetStrokeLineJoin(StrokeLineJoin::ROUND);
    route_names_.emplace_back(std::move(route_substrate));
    route_names_.emplace_back(std::move(route_name));
}

void MapRender::GetStopCirclePicture(geo::Coordinates x_y) {
    Circle stop_dot;
    stop_dot.SetCenter(proj_(x_y)).
             SetRadius(settings_.stop_radius).
             SetFillColor("white"s);
    stop_dots_.emplace_back(std::move(stop_dot));
}

void MapRender::GetStopNamePicture(const std::string& name, geo::Coordinates x_y) {
    Text stop_name, stop_substrate;
    stop_name.SetPosition(proj_(x_y)).
              SetOffset(Point(settings_.stop_label_offset.first, settings_.stop_label_offset.second)).
              SetFontSize(settings_.stop_label_font_size).
              SetFontFamily("Verdana"s).
              SetFillColor("black"s).
              SetData(name);
    stop_substrate.SetPosition(proj_(x_y)).
                   SetOffset(Point(settings_.stop_label_offset.first, settings_.stop_label_offset.second)).
                   SetFontSize(settings_.stop_label_font_size).
                   SetFontFamily("Verdana"s).
                   SetFillColor(settings_.underlayer_color).
                   SetStrokeColor(settings_.underlayer_color).
                   SetStrokeWidth(settings_.underlayer_width).
                   SetStrokeLineCap(StrokeLineCap::ROUND).
                   SetStrokeLineJoin(StrokeLineJoin::ROUND).
                   SetData(name);
    stop_names_.emplace_back(std::move(stop_substrate));
    stop_names_.emplace_back(std::move(stop_name));
}