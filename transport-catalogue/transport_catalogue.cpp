#include "transport_catalogue.h"

Stops::Stops(const std::string& name, double x, double y)
	: stop_name(name)
	, latitude(std::move(x))
	, longitude(std::move(y))
{}

bool Stops::operator==(const Stops& other) const {
	return stop_name == other.stop_name && latitude == other.latitude && longitude == other.longitude;
}

bool Stops::operator!=(const Stops& other) const {
	return !(*this == other);
}

Buses::Buses(const std::string& name, RouteType type, const std::vector<Stops*> stops)
	: route_name(name)
	, route_type(type)
	, route_stops(stops)
{}

size_t PairHasher::operator()(std::pair<Stops*, Stops*> obj) const {
	return ptr_hasher(obj.first) + 37 * ptr_hasher(obj.second);
}

void TransportCatalogue::AddStop(const Stops& bus_stop) {
	bus_stops_.push_back(bus_stop);
	auto& last_added_stop = bus_stops_.back();
	bus_stops_index_[last_added_stop.stop_name] = &last_added_stop;
	route_to_stops_index_[last_added_stop.stop_name] = {};
	return;
}

void TransportCatalogue::AddBus(const std::string route_name, RouteType type, const std::vector<std::string>& stops) {
	std::vector<Stops*> bus_stops;
	std::for_each(stops.begin(), stops.end(),
		[&](const std::string& stop) {
			route_to_stops_index_[stop].insert(route_name);
			bus_stops.push_back(std::move(FindStop(stop)));});
	Buses bus_route(route_name, type, bus_stops);
	bus_routes_.push_back(std::move(bus_route));
	bus_routes_index_[bus_routes_.back().route_name] = { &bus_routes_.back().route_stops, type };
	return;
}

void TransportCatalogue::AddDistance(const std::string& stop1, const std::string& stop2, int distance) {
	stops_distance_index_[{std::move(FindStop(stop1)), std::move(FindStop(stop2))}] = distance;
	return;
}

std::optional<BusInfo> TransportCatalogue::GetBusInfo(const std::string& route_name) const {
	if (bus_routes_index_.count(route_name) == 0) return {};
	BusInfo result;
	double coordinate_distance = 0, real_distance = 0;
	std::unordered_set<std::string> unique_stops;
	auto [route_stops, route_type] = FindBus(route_name);
	size_t end_index = route_stops->size();
	unique_stops.insert(route_stops->at(0)->stop_name);
	for (size_t index = 0; index < end_index - 1; ++index) {
		auto from_stop = FindStop(route_stops->at(index)->stop_name);
		auto to_stop = FindStop(route_stops->at(index + 1)->stop_name);
		coordinate_distance += ComputeDistance({ from_stop->latitude, from_stop->longitude },
			                                   { to_stop->latitude, to_stop->longitude });
		if (stops_distance_index_.count({ from_stop, to_stop })) real_distance += ComputateDistance(from_stop, to_stop);
		else real_distance += ComputateDistance(to_stop, from_stop);
		if (route_type == RouteType::LINER_ROUTE) {
			if (stops_distance_index_.count({ to_stop, from_stop })) real_distance += ComputateDistance(to_stop, from_stop);
			else real_distance += ComputateDistance(from_stop, to_stop);
		}
		unique_stops.insert(to_stop->stop_name);
	}
	result = route_type == RouteType::LINER_ROUTE ? std::tuple{ end_index * 2 - 1, unique_stops.size(),
		                                                        real_distance, real_distance * 1.0 / (coordinate_distance * 2) }
	                                              : std::tuple{ end_index, unique_stops.size(),
				                                                real_distance, real_distance * 1.0 / coordinate_distance };
	return result;
}

std::optional<std::set<std::string>> TransportCatalogue::GetStopInfo(const std::string& stop_name) const {
	if (route_to_stops_index_.count(stop_name) == 0) {
		return {};
	}
	return route_to_stops_index_.at(stop_name);
}

std::unordered_map<std::pair<Stops*, Stops*>, int, PairHasher> TransportCatalogue::GetDistancesIndex() const {
	return stops_distance_index_;
}

Stops* TransportCatalogue::FindStop(const std::string& stop) const {
	assert(bus_stops_index_.count(stop));
	return bus_stops_index_.at(stop);
}

RouteInfo TransportCatalogue::FindBus(const std::string& route_name) const {
	assert(bus_routes_index_.count(route_name));
	return bus_routes_index_.at(route_name);
}

double TransportCatalogue::ComputateDistance(Stops* stop1, Stops* stop2) const {
	assert(stops_distance_index_.count({ stop1, stop2 }));
	return stops_distance_index_.at({ stop1, stop2 }) * 1.0;
}