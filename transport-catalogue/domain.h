#pragma once

#include <utility>
#include <string>
#include <vector>

#include "geo.h"

enum class RouteType {
	LINER_ROUTE,
	RING_ROUTE,
};

struct Stop {

	Stop() = default;
	explicit Stop(std::string name, geo::Coordinates coordinates);
	explicit Stop(std::string name, double latitude, double longitude);

	bool operator==(const Stop&) const;

	bool operator!=(const Stop&) const;

	std::string stop_name;
	geo::Coordinates coordinates;

};

struct Bus {

	Bus() = default;
	explicit Bus(std::string name, RouteType type, std::vector<Stop*> route_stops);

	bool operator==(const Bus&) const;

	bool operator!=(const Bus&) const;

	std::string route_name;
	RouteType route_type;
	std::vector<Stop*> route_stops;

};