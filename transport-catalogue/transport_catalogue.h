#pragma once

#include <cinttypes>
#include <utility>
#include <functional>
#include <cassert>
#include <string>
#include <string_view>
#include <deque>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <tuple>
#include <algorithm>
#include <optional>

#include "geo.h"

enum class RouteType {
	LINER_ROUTE,
    RING_ROUTE,
};

struct Stop {

	Stop() = default;
	explicit Stop(std::string, double, double);

	bool operator==(const Stop&) const;

	bool operator!=(const Stop&) const;

	std::string stop_name;
	double latitude = 0;
	double longitude = 0;

};

struct Bus {

	Bus() = default;
	explicit Bus(std::string, RouteType, std::vector<Stop*>);

	bool operator==(const Bus&) const;

	bool operator!=(const Bus&) const;

	std::string route_name;
	RouteType route_type;
	std::vector<Stop*> route_stops;

};

struct PairHasher {

	size_t operator()(std::pair<Stop*, Stop*>) const;

private:

	std::hash<const void*> ptr_hasher;

};

using BusInfo = std::tuple<size_t, size_t, double, double>;
using RouteInfo = std::pair<std::vector<Stop*>*, RouteType>;

class TransportCatalogue {
public:

	void AddStop(Stop);
	void AddBus(const std::string&, RouteType, const std::vector<std::string_view>&);
	void AddDistance(std::string_view, std::string_view, double);

	std::optional<BusInfo> GetBusInfo(std::string_view) const;
	std::optional<std::set<std::string>> GetStopInfo(std::string_view) const;
	std::unordered_map<std::pair<Stop*, Stop*>, double, PairHasher> GetDistancesIndex() const;

	Stop* FindStop(std::string_view) const;
	RouteInfo FindBus(std::string_view) const;

private:

	std::deque<Stop> bus_stops_;
	std::deque<Bus> bus_routes_;
	std::unordered_map<std::string_view, Stop*> bus_stops_index_;
	std::unordered_map<std::string_view, RouteInfo> bus_routes_index_;
	std::unordered_map<std::string_view, std::set<std::string>> route_to_stops_index_;
	std::unordered_map<std::pair<Stop*, Stop*>, double, PairHasher> stops_distance_index_;

	double ComputeRealDistance(Stop*, Stop*, RouteType) const;

};