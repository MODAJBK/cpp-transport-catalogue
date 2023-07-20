#pragma once

#include <cmath>
#include <optional>
#include <memory>
#include <utility>
#include <string>
#include <string_view>
#include <set>
#include <unordered_map>

#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"

struct RoutingSettings {

	int bus_wait_time_min = 0;
	double bus_velocity_kmph = {};

};

struct EdgeWeight {

	EdgeWeight() = default;

	EdgeWeight operator+(const EdgeWeight&) const;
	EdgeWeight& operator+=(const EdgeWeight&);

	bool operator<(const EdgeWeight&) const;
	bool operator>(const EdgeWeight&) const;

	double spend_time = {};
	int span_count = {};
	std::string edge_type = {};
};

class RouteBuilder {

	using VertexPair = std::pair<graph::VertexId, graph::VertexId>; // first - WaitVertex, second - RideVertex
	using RouteEdge = graph::Edge<EdgeWeight>;
	using RouteGraph = graph::DirectedWeightedGraph<EdgeWeight>;

public:

	using RouteData = graph::Router<EdgeWeight>::RouteInfo;
	using TcRouter = graph::Router<EdgeWeight>;

	RouteBuilder() = delete;
	RouteBuilder(size_t stops_count, RoutingSettings);

	const RouteGraph& BuildGraph(const TransportCatalogue&, const std::set<std::string_view>& route_names,
		                         const std::set<std::string_view>& stop_names);
	std::optional<RouteData> BuildRouteBetweenTwoStops(std::string_view stop_from, std::string_view stop_to) const;

	void SetRouter(std::unique_ptr<TcRouter>&& router);

	const RouteGraph& GetRouteGraph() const;

private:

	RouteGraph route_graph_;
	RoutingSettings routing_settings_;
	std::unique_ptr<TcRouter> router_ptr_ = nullptr;
	std::unordered_map<std::string_view, VertexPair> vertex_to_stop_;


	void BuildSubgraphForStops(const std::set<std::string_view>& stop_names);
	void BuildSubgraphForRoutes(const TransportCatalogue&, const std::set<std::string_view>& route_names);
	void BuildSubgraphForLinerRoute(const TransportCatalogue&, std::vector<Stop*>* route_stops, const std::string& route_name);
	void BuildSubgraphForLinerRouteInDirection(const TransportCatalogue&, bool is_reverse, std::vector<Stop*>* route_stops, const std::string& route_name);
	void BuildSubgraphForRingRoute(const TransportCatalogue&, std::vector<Stop*>* route_stops, const std::string& route_name);

	RouteEdge GetStopEdge(graph::VertexId from, graph::VertexId to, const std::string& type) const;
	EdgeWeight GetRouteEdgeWeight(double distance, const std::string& type) const;

};

double CalculateTime(double distance_m, double speed_kmph);