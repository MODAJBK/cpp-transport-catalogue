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

	Stop();
	explicit Stop(std::string name, geo::Coordinates coordinates);
	explicit Stop(std::string name, double latitude, double longitude);

	bool operator==(const Stop&) const;

	bool operator!=(const Stop&) const;

	const int stop_id = 0;
	std::string stop_name;
	geo::Coordinates coordinates;

private:

	static inline int id_count = 0;

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