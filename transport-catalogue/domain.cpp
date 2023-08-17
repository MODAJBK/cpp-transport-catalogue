#include "domain.h"

using namespace geo;

Stop::Stop()
	: stop_id(id_count++) {
}

Stop::Stop(std::string name, Coordinates coordinates)
	: stop_id(id_count++)
    , stop_name(std::move(name))
	, coordinates(std::move(coordinates))
{}

Stop::Stop(std::string name, double latitude, double longitude)
	: stop_id(id_count++)
	, stop_name(std::move(name))
	, coordinates(std::move(Coordinates(latitude, longitude)))
{}

bool Stop::operator==(const Stop& other) const {
	return stop_name == other.stop_name && coordinates == other.coordinates;
}

bool Stop::operator!=(const Stop& other) const {
	return !(*this == other);
}

Bus::Bus(std::string name, RouteType type, std::vector<Stop*> stops)
	: route_name(std::move(name))
	, route_type(type)
	, route_stops(std::move(stops))
{}

bool Bus::operator==(const Bus& other) const {
	return route_name == other.route_name && route_type == other.route_type && route_stops == other.route_stops;
}

bool Bus::operator!=(const Bus& other) const {
	return !(*this == other);
}