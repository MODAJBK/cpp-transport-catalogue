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

struct Stops {

	Stops() = default;
	explicit Stops(const std::string&, double, double);

	bool operator==(const Stops&) const;

	bool operator!=(const Stops&) const;

	std::string stop_name;
	double latitude = 0;
	double longitude = 0;

};

struct Buses {

	Buses() = default;
	explicit Buses(const std::string&, RouteType, const std::vector<Stops*>);

	std::string route_name;
	RouteType route_type;
	std::vector<Stops*> route_stops;

};

struct PairHasher {

	size_t operator()(std::pair<Stops*, Stops*>) const;

private:

	std::hash<const void*> ptr_hasher;

};

using BusInfo = std::tuple<size_t, size_t, double, double>;
using RouteInfo = std::pair<std::vector<Stops*>*, RouteType>;

class TransportCatalogue {
public:

	void AddStop(const Stops& bus_stop);
	void AddBus(const std::string, RouteType, const std::vector<std::string>&);
	void AddDistance(const std::string&, const std::string&, int);

	std::optional<BusInfo> GetBusInfo(const std::string&) const;
	std::optional<std::set<std::string>> GetStopInfo(const std::string&) const;
	std::unordered_map<std::pair<Stops*, Stops*>, int, PairHasher> GetDistancesIndex() const;

	Stops* FindStop(const std::string&) const;
	RouteInfo FindBus(const std::string&) const;

private:

	std::deque<Stops> bus_stops_;
	std::deque<Buses> bus_routes_;
	std::unordered_map<std::string_view, Stops*> bus_stops_index_;
	std::unordered_map<std::string_view, RouteInfo> bus_routes_index_;
	std::unordered_map<std::string, std::set<std::string>> route_to_stops_index_;
	std::unordered_map<std::pair<Stops*, Stops*>, int, PairHasher> stops_distance_index_;

	double ComputateDistance(Stops*, Stops*) const;

};