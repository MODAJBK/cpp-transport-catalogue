#include "transport_catalogue.h"

using namespace geo;

size_t PairHasher::operator()(std::pair<Stop*, Stop*> obj) const {
	return ptr_hasher(obj.first) + 37 * ptr_hasher(obj.second);
}

//Adds information about stop (stop name, stop coordinates)
void TransportCatalogue::AddStop(const Stop& bus_stop) {
	bus_stops_.push_back(bus_stop);
	auto& last_added_stop = bus_stops_.back();
	bus_stops_index_[last_added_stop.stop_name] = &last_added_stop;
	stop_id_to_ptr_[last_added_stop.stop_id] = &last_added_stop;
	route_to_stops_index_[last_added_stop.stop_name] = {};
}

//Adds information about bus route (route name and type, list of route stops)
void TransportCatalogue::AddBus(const std::string& route_name, RouteType type, const std::vector<std::string_view>& stops) {
	std::vector<Stop*> bus_stops;
	std::for_each(stops.begin(), stops.end(),
		          [&](std::string_view stop_name) {
			          auto stop = FindStop(stop_name);
			          coordinates_.push_back(stop->coordinates);
			          route_to_stops_index_[stop->stop_name].insert(route_name);
			          bus_stops.push_back(std::move(stop)); 
		          });
	Bus bus_route(route_name, type, bus_stops);
	bus_routes_.push_back(std::move(bus_route));
	bus_routes_index_[bus_routes_.back().route_name] = { &bus_routes_.back().route_stops, type };
}

void TransportCatalogue::AddBus(const std::string& route_name, RouteType type, const std::vector<Stop*>& route_stops) {
	for (const auto& stop : route_stops) {
		coordinates_.push_back(stop->coordinates);
		route_to_stops_index_[stop->stop_name].insert(route_name);
	}
	Bus bus_route(route_name, type, route_stops);
	bus_routes_.push_back(std::move(bus_route));
	bus_routes_index_[bus_routes_.back().route_name] = { &bus_routes_.back().route_stops, type };
}

void TransportCatalogue::AddBus(const Bus& route) {
	AddBus(route.route_name, route.route_type, route.route_stops);
}

//Sets distance value betwenn two stops with names stop1 and stop2
void TransportCatalogue::SetDistance(std::string_view from_stop, std::string_view to_stop, double distance) {
	stops_distance_index_[{std::move(FindStop(from_stop)), std::move(FindStop(to_stop))}] = distance;
}

void TransportCatalogue::SetDistance(Stop* from_stop, Stop* to_stop, double distance) {
	stops_distance_index_[{ from_stop, to_stop }] = distance;
}

//Return output information about particular route (total route stops, unique stops, real distance (m) and curvature)
std::optional<BusInfo> TransportCatalogue::GetBusInfo(std::string_view route_name) const {
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
		coordinate_distance += ComputeDistance(from_stop->coordinates, to_stop->coordinates);
		real_distance += ComputeRealDistance(from_stop, to_stop, route_type);
		unique_stops.insert(to_stop->stop_name);
	}
	result = route_type == RouteType::LINER_ROUTE ? std::tuple{ end_index * 2 - 1, unique_stops.size(),
		real_distance, real_distance * 1.0 / (coordinate_distance * 2) }
	: std::tuple{ end_index, unique_stops.size(),
				  real_distance, real_distance * 1.0 / coordinate_distance };
	return result;
}

const std::deque<Stop>& TransportCatalogue::GetStops() const {
	return bus_stops_;
}

const std::deque<Bus>& TransportCatalogue::GetRoutes() const {
	return bus_routes_;
}

const std::unordered_map<std::pair<Stop*, Stop*>, double, PairHasher>& TransportCatalogue::GetDistances() const {
	return stops_distance_index_;
}

//Returns the total number of stops
size_t TransportCatalogue::GetStopsCount() const {
	return bus_stops_.size();
}

//Returns the total number of routes
size_t TransportCatalogue::GetRoutesCount() const {
	return bus_routes_.size();
}

//Returns distance(m) between stop_from and stop_to
double TransportCatalogue::GetDistanceBetweenTwoStops(std::string_view stop_from,
	std::string_view stop_to) const {
	return GetDistanceBetweenTwoStops(FindStop(stop_from), FindStop(stop_to));
}

double TransportCatalogue::GetDistanceBetweenTwoStops(Stop* stop_from, Stop* stop_to) const {
	if (stops_distance_index_.count({ stop_from, stop_to })) {
		return stops_distance_index_.at({ stop_from, stop_to });
	}
	return stops_distance_index_.at({ stop_to, stop_from });
}

//Returns output information about particular stop (list of routes passing through stop)
std::optional<std::set<std::string>> TransportCatalogue::GetStopInfo(std::string_view stop_name) const {
	if (route_to_stops_index_.count(stop_name) == 0) {
		return {};
	}
	return route_to_stops_index_.at(stop_name);
}

std::set<std::string_view> TransportCatalogue::GetStopNames() const {
	std::set<std::string_view> stop_names;
	for (const auto& stop : bus_stops_) {
		if (!GetStopInfo(stop.stop_name)->empty()) {
			stop_names.insert(stop.stop_name);
		}
	}
	return stop_names;
}

std::set<std::string_view> TransportCatalogue::GetRouteNames() const {
	std::set<std::string_view> route_names;
	for (const auto& route : bus_routes_) {
		route_names.insert(route.route_name);
	}
	return route_names;
}

//Returns vector with stops coordinates
std::vector<geo::Coordinates> TransportCatalogue::GetCoordinates() const {
	return coordinates_;
}

//Returns information (as pointer) about particular stop (stop name and coordinates)
Stop* TransportCatalogue::FindStop(std::string_view stop) const {
	assert(bus_stops_index_.count(stop));
	return bus_stops_index_.at(stop);
}

Stop* TransportCatalogue::FindStop(int id) const {
	assert(stop_id_to_ptr_.count(id));
	return stop_id_to_ptr_.at(id);
}

//Returns information about particulaer route (route type and list stops)
RouteInfo TransportCatalogue::FindBus(std::string_view route_name) const {
	assert(bus_routes_index_.count(route_name));
	return bus_routes_index_.at(route_name);
}

//Calculate distance between stop1 and stop2 (in both directions for linear route)
double TransportCatalogue::ComputeRealDistance(Stop* stop1, Stop* stop2, RouteType type) const {
	double result = 0;
	if (type == RouteType::LINER_ROUTE) {
		stops_distance_index_.count({ stop1, stop2 }) ? result += stops_distance_index_.at({ stop1, stop2 })
			: result += stops_distance_index_.at({ stop2, stop1 });
		stops_distance_index_.count({ stop2, stop1 }) ? result += stops_distance_index_.at({ stop2, stop1 })
			: result *= 2;
	}
	else {
		stops_distance_index_.count({ stop1, stop2 }) ? result += stops_distance_index_.at({ stop1, stop2 })
			: result += stops_distance_index_.at({ stop2, stop1 });
	}
	return result;
}