#pragma once

#include <cmath>
#include <algorithm>
#include <vector>

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
    // points_begin � points_end ������ ������ � ����� ��������� ��������� geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
        double max_width, double max_height, double padding)
        : padding_(padding) //
    {
        // ���� ����� ����������� ����� �� ������, ��������� ������
        if (points_begin == points_end) {
            return;
        }

        // ������� ����� � ����������� � ������������ ��������
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // ������� ����� � ����������� � ������������ �������
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // ��������� ����������� ��������������� ����� ���������� x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // ��������� ����������� ��������������� ����� ���������� y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // ������������ ��������������� �� ������ � ������ ���������,
            // ���� ����������� �� ���
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        }
        else if (width_zoom) {
            // ����������� ��������������� �� ������ ���������, ���������� ���
            zoom_coeff_ = *width_zoom;
        }
        else if (height_zoom) {
            // ����������� ��������������� �� ������ ���������, ���������� ���
            zoom_coeff_ = *height_zoom;
        }
    }

    // ���������� ������ � ������� � ���������� ������ SVG-�����������
    svg::Point operator()(geo::Coordinates coords) const;

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

svg::Polyline GetMapPicture(const RenderSettings&, SphereProjector&, 
                            size_t&, std::vector<Stop*>*, RouteType);


std::pair<svg::Text, svg::Text> GetRouteNamePicture(const RenderSettings&, SphereProjector&, 
                                                    size_t&, const std::string name, geo::Coordinates);

svg::Circle GetStopCirclePicture(const RenderSettings&, SphereProjector&, geo::Coordinates);

std::pair<svg::Text, svg::Text> GetStopNamePicture(const RenderSettings&, SphereProjector&,
                                                   const std::string& name, geo::Coordinates);