#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_map>

#include "svg.pb.h"
#include "graph.pb.h"
#include "transport_catalogue.h"
#include "transport_catalogue.pb.h"
#include "map_renderer.h"
#include "map_renderer.pb.h"
#include "transport_router.h"
#include "transport_router.pb.h"

struct SerializeSettings {
	std::filesystem::path path_to_db;
};

class Serializer {
public:

	Serializer() = default;
	explicit Serializer(SerializeSettings settings);

	void SetSettings(const SerializeSettings& settings);

	void SaveToFile(TransportCatalogue& catalogue, const RenderSettings& settings, const RouteBuilder& builder);

	RenderSettings GetFromFile(TransportCatalogue& catalogue, RouteBuilder& builder);

private:

	SerializeSettings settings_;
	tc_serialize::CatalogueData proto_data_;

	tc_serialize::TransportCatalogue SerializeCatalogue(TransportCatalogue& catalogue) const;
	tc_serialize::Stop SerializeStop(const Stop& stop) const;
	tc_serialize::Distances SerializeDistance(const Stop& stop_from, const Stop& stop_to, double distance) const;
	tc_serialize::Bus SerializeBus(const Bus& route) const;

	map_serialize::RenderSettings SerializeRenderSettings(const RenderSettings& settings) const;
	svg_serialize::Color SerializeColor(const svg::Color& color) const;

	router_serialize::TransportRouter SerializeCatalogueRouter(const RouteBuilder& builder) const;
	router_serialize::RouteSettings SerializeRouterSettings(const RoutingSettings& settings) const;
	graph_serialize::DirectedWeightedGraph SerializeGraph(const RouteBuilder::RouteGraph& graph) const;
	graph_serialize::Edge SerializeEdge(const RouteBuilder::RouteEdge& edge) const;
	graph_serialize::EdgeWeight SerializeWeight(const EdgeWeight& weight) const;
	graph_serialize::Router SerializeRouter(const RouteBuilder::TcRouter& router) const;

	void DeserializeCatalogue(TransportCatalogue& catalogue);
	Stop DeserializeStop(const tc_serialize::Stop& proto_stop) const;
	Bus DeserializeBus(const tc_serialize::Bus& proto_route, const TransportCatalogue&) const;
	void DeserializeDistance(const tc_serialize::Distances& proto_distance, TransportCatalogue&) const;
	
	RenderSettings DeserializeRenderSettings();
	svg::Color DeserializeColor(const svg_serialize::Color& proto_color) const;

	void DeserializeCatalogueRouter(RouteBuilder& builder);
	RoutingSettings DeserializeRoutingSettings(const router_serialize::RouteSettings& proto_settings) const;
	RouteBuilder::RouteGraph DeserializeGraph(const graph_serialize::DirectedWeightedGraph& proto_graph) const;
	RouteBuilder::RouteEdge DeserializeEdge(const graph_serialize::Edge& proto_edge) const;
	EdgeWeight DeserializeWeight(const graph_serialize::EdgeWeight& proto_weight) const;
	std::unique_ptr<RouteBuilder::TcRouter> DeserializeRouter(const graph_serialize::Router& proto_router) const;

};