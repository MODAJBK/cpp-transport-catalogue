#pragma once

#include <iostream>
#include <sstream>
#include <exception>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "json.h"
#include "json_builder.h"
#include "geo.h"
#include "domain.h"
#include "request_handler.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"
#include "serialization.h"

class JsonReader {
public:

	JsonReader() = delete;
	explicit JsonReader(std::istream&);

	void BaseRequestsParsing(TransportCatalogue& catalogue) const;

	void StatRequestsParsing(const TransportCatalogue& catalogue, const std::string& map, 
		                     const RouteBuilder& route_builder, std::ostream&) const;

	SerializeSettings GetSerializationSettings() const;
	RenderSettings GetRenderSettings() const;
	RoutingSettings GetRoutingSettings() const;

private:

	json::Document requests_data_;

	void CompleteBaseRequests(TransportCatalogue& catalogue,
		                      const std::vector<json::Dict>& stop_requests,
		                      const std::vector<json::Dict>& bus_requests) const;
	void CompleteAddStop(TransportCatalogue&, const json::Dict& request) const;
	void CompleteSetDistance(TransportCatalogue&, const json::Dict& request) const;
	void CompleteAddBus(TransportCatalogue&, const json::Dict& request) const;

	json::Node::Value GetRouteRequestResult(const TransportCatalogue&, const json::Dict& request) const;
	json::Node::Value GetStopRequestResult(const TransportCatalogue&, const json::Dict& request) const;
	json::Node::Value GetMapDrawingResult(const std::string& map, const json::Dict& request) const;
	json::Node::Value GetRouteBuildingResult(const RouteBuilder&, const json::Dict& request) const;

	svg::Color ParseColor(const json::Node&) const;

};