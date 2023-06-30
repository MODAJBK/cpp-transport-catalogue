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
#include "domain.h"

struct PairHasher {

	size_t operator()(std::pair<Stop*, Stop*>) const;

private:

	std::hash<const void*> ptr_hasher;

};

using BusInfo = std::tuple<size_t, size_t, double, double>;
using RouteInfo = std::pair<std::vector<Stop*>*, RouteType>;

class TransportCatalogue {
public:

	void AddStop(const Stop& stop);
	void AddBus(const std::string& route_name, RouteType type, const std::vector<std::string_view>& route_stops);
	void SetDistance(std::string_view from_stop, std::string_view to_stop, double distnace);

	std::optional<BusInfo> GetBusInfo(std::string_view route_name) const;
	std::optional<std::set<std::string>> GetStopInfo(std::string_view stop_name) const;
	std::vector<geo::Coordinates> GetCoordinates() const;

	Stop* FindStop(std::string_view stop_name) const;
	RouteInfo FindBus(std::string_view route_name) const;

private:

	std::deque<Stop> bus_stops_;
	std::deque<Bus> bus_routes_;
	std::vector<geo::Coordinates> coordinates_;
	std::unordered_map<std::string_view, Stop*> bus_stops_index_;
	std::unordered_map<std::string_view, RouteInfo> bus_routes_index_;
	std::unordered_map<std::string_view, std::set<std::string>> route_to_stops_index_;
	std::unordered_map<std::pair<Stop*, Stop*>, double, PairHasher> stops_distance_index_;

	double ComputeRealDistance(Stop* stop1, Stop* stop2, RouteType type) const;

};