#include "transport_router.h"

using namespace graph;

//-------------------------EdgeWeight-------------------------

EdgeWeight EdgeWeight::operator+(const EdgeWeight& other) const {
	EdgeWeight result(*this);
	result.spend_time += other.spend_time;
	result.span_count += other.span_count;
	return result;
}

EdgeWeight& EdgeWeight::operator+=(const EdgeWeight& other) {
	spend_time += other.spend_time;
	span_count += other.span_count;
	return *this;
}

bool EdgeWeight::operator<(const EdgeWeight& other) const {
	return spend_time < other.spend_time;
}

bool EdgeWeight::operator>(const EdgeWeight& other) const {
	return spend_time > other.spend_time;
}

//-------------------------RouteBuilder-------------------------

RouteBuilder::RouteBuilder(size_t stops_count, RoutingSettings settings)
	: route_graph_(stops_count * 2)
	, routing_settings_(std::move(settings))
{}

const RouteBuilder::RouteGraph& RouteBuilder::BuildGraph(const TransportCatalogue& catalogue,
	                                                     const std::set<std::string_view>& route_names,
	                                                     const std::set<std::string_view>& stop_names) {
	BuildSubgraphForStops(stop_names);
	BuildSubgraphForRoutes(catalogue, route_names);
	return route_graph_;
}

const RouteBuilder::RouteGraph& RouteBuilder::GetRouteGraph() const {
	return route_graph_;
}

void RouteBuilder::SetRouter(std::unique_ptr<RouteBuilder::TcRouter>&& router) {
	router_ptr_ = std::move(router);
}

std::optional<RouteBuilder::RouteData> RouteBuilder::BuildRouteBetweenTwoStops(std::string_view stop_from, std::string_view stop_to) const {
	if (!vertex_to_stop_.count(stop_from) || !vertex_to_stop_.count(stop_to)) {
		return {};
	}
	const auto [id_from, _] = vertex_to_stop_.at(stop_from);
	const auto [id_to, __] = vertex_to_stop_.at(stop_to);
	return router_ptr_->BuildRoute(id_from, id_to);
}

void RouteBuilder::BuildSubgraphForStops(const std::set<std::string_view>& stop_names) {
	VertexId id = 0;
	for (const auto stop : stop_names) {
		vertex_to_stop_[stop] = { id, (id + 1) };
		route_graph_.AddEdge(GetStopEdge(id, (id + 1), std::string(stop)));
		id += 2;
	}
}

void RouteBuilder::BuildSubgraphForRoutes(const TransportCatalogue& catalogue, 
	                                      const std::set<std::string_view>& route_names) {
	for (const auto route : route_names) {
		const auto& [route_stops, route_type] = catalogue.FindBus(route);
		switch (route_type) {
		case RouteType::LINER_ROUTE:
			BuildSubgraphForLinerRoute(catalogue, route_stops, std::string(route));
			break;
		case RouteType::RING_ROUTE:
			BuildSubgraphForRingRoute(catalogue, route_stops, std::string(route));
			break;
		}
	}
}

void RouteBuilder::BuildSubgraphForLinerRoute(const TransportCatalogue& catalogue,
	                                          std::vector<Stop*>* route_stops, 
	                                          const std::string& route_name) {
	BuildSubgraphForLinerRouteInDirection(catalogue, false, route_stops, route_name);
	BuildSubgraphForLinerRouteInDirection(catalogue, true, route_stops, route_name);
}

void RouteBuilder::BuildSubgraphForLinerRouteInDirection(const TransportCatalogue& catalogue, bool is_reverse,
	                                                     std::vector<Stop*>* route_stops, const std::string& route_name) {
	size_t total_stops_count = route_stops->size();
	size_t start_val, end_val, inc;
	if (is_reverse) {
		start_val = total_stops_count - 1;
		end_val = -1;
		inc = -1;
	}
	else {
		start_val = 0;
		end_val = total_stops_count;
		inc = 1;
	}
	for (size_t from = start_val; from != end_val; from += inc) {
		EdgeWeight total_weight = {};
		const auto [_, id_from_ride] = vertex_to_stop_.at(route_stops->at(from)->stop_name);
		for (size_t to = from + inc; to != end_val; to += inc) {
			if (std::abs(static_cast<int>(from - to)) != 1) {
				const auto [id_to_wait, _] = vertex_to_stop_.at(route_stops->at(to)->stop_name);
				double distance = catalogue.GetDistanceBetweenTwoStops(route_stops->at(to - inc),
					                                                   route_stops->at(to));
				total_weight += GetRouteEdgeWeight(distance, route_name);
				route_graph_.AddEdge({ id_from_ride, id_to_wait, total_weight });
			}
			else {
				const auto [id_to_wait, _] = vertex_to_stop_.at(route_stops->at(to)->stop_name);
				double distance = catalogue.GetDistanceBetweenTwoStops(route_stops->at(from),
					                                                   route_stops->at(to));
				total_weight += GetRouteEdgeWeight(distance, route_name);
				route_graph_.AddEdge({ id_from_ride, id_to_wait, total_weight });
			}
		}
	}
}

void RouteBuilder::BuildSubgraphForRingRoute(const TransportCatalogue& catalogue,
	                                         std::vector<Stop*>* route_stops, 
	                                         const std::string& route_name) {
	size_t total_stops_count = route_stops->size();
	for (size_t from = 0; from < total_stops_count; ++from) {
		EdgeWeight total_weight = {};
		const auto [_, id_from_ride] = vertex_to_stop_.at(route_stops->at(from)->stop_name);
		for (size_t to = from + 1; to < total_stops_count; ++to) {
			if (std::abs(static_cast<int>(from - to)) != 1) {
				const auto [id_to_wait, _] = vertex_to_stop_.at(route_stops->at(to)->stop_name);
			    double distance = catalogue.GetDistanceBetweenTwoStops(route_stops->at(to - 1),
				                                                       route_stops->at(to));
			    total_weight += GetRouteEdgeWeight(distance, route_name);
			    route_graph_.AddEdge({ id_from_ride, id_to_wait, total_weight });
			}
			else {
				const auto [id_to_wait, _] = vertex_to_stop_.at(route_stops->at(to)->stop_name);
				double distance = catalogue.GetDistanceBetweenTwoStops(route_stops->at(from),
					                                                   route_stops->at(to));
				total_weight += GetRouteEdgeWeight(distance, route_name);
				route_graph_.AddEdge({ id_from_ride, id_to_wait, total_weight });
			}
		}
	}
}

RouteBuilder::RouteEdge RouteBuilder::GetStopEdge(VertexId from, VertexId to, const std::string& type) const {
	EdgeWeight stop_weight = { static_cast<double>(routing_settings_.bus_wait_time_min), 0, type };
	return { from, to, stop_weight };
}

EdgeWeight RouteBuilder::GetRouteEdgeWeight(double distance, const std::string& type) const {
	return { CalculateTime(distance, routing_settings_.bus_velocity_kmph), 1, type };
}

double CalculateTime(double distance_m, double speed_kmph) {
	static const int MINUTES_IN_ONE_HOUR = 60;
	static const int METERS_IN_ONE_KILOMETER = 1000;
	double speed_mpm = speed_kmph * METERS_IN_ONE_KILOMETER / MINUTES_IN_ONE_HOUR;
	return distance_m / speed_mpm;
}