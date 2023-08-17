#pragma once

#include <cmath>
#include <algorithm>
#include <memory>
#include <vector>

#include "transport_catalogue.h"
#include "geo.h"
#include "domain.h"
#include "svg.h"

struct RenderSettings {
    RenderSettings() = default;

    double width = 0.0;
    double height = 0.0;
    double padding = 0.0;

    double line_width = 0.0;
    double stop_radius = 0.0;

    int bus_label_font_size = 0;
    std::pair<double, double> bus_label_offset;

    int stop_label_font_size = 0;
    std::pair<double, double> stop_label_offset;

    svg::Color underlayer_color;
    double underlayer_width = 0.0;

    std::vector<svg::Color> color_palette;
};

inline const double EPSILON = 1e-6;
bool IsZero(double value);

class SphereProjector {
public:

    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
        double max_width, double max_height, double padding)
        : padding_(padding) //
    {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        }
        else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        }
        else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const;

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

class MapRender {
public:

    template <typename Container>
    MapRender(const Container& container, RenderSettings settings)
        : proj_(std::begin(container), std::end(container),
            settings.width, settings.height,
            settings.padding)
        , settings_(std::move(settings))
    {}

    void DrawTransportCatalogue(const TransportCatalogue& catalogue,
                                std::set<std::string_view>& route_names,
                                std::set<std::string_view>& stop_names,
                                std::ostream& out);

private:

    SphereProjector proj_;
    RenderSettings settings_;
    std::vector<svg::Polyline> route_maps_;
    std::vector<svg::Text> route_names_;
    std::vector<svg::Circle> stop_dots_;
    std::vector<svg::Text> stop_names_;

    void DrawRoutes(const TransportCatalogue&, const std::set<std::string_view>& route_names);
    void DrawStops(const TransportCatalogue&, const std::set<std::string_view>& stop_names);

    void GetRouteMapPicture(size_t, RouteType, std::vector<Stop*>*);
    void GetRouteNamePicture(size_t, const std::string name, geo::Coordinates);
    void GetStopCirclePicture(geo::Coordinates);
    void GetStopNamePicture(const std::string& name, geo::Coordinates);

};

template <typename DrawableIterator>
void DrawPicture(DrawableIterator begin, DrawableIterator end, svg::ObjectContainer& target) {
    for (auto it = begin; it != end; ++it) {
        target.Add(std::move(*it));
    }
}

template <typename Container>
void DrawPicture(const Container& container, svg::ObjectContainer& target) {
    using namespace std;
    DrawPicture(begin(container), end(container), target);
}