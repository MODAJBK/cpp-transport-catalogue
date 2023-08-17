#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>
#include <set>

#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"

class RequestHandler {
public:

	RequestHandler() = delete;
	RequestHandler(const TransportCatalogue&, MapRender&, const RoutingSettings&);
	RequestHandler(const TransportCatalogue&);

	std::string RenderMap();

	void BuildGraph();

	void SetRender(MapRender& render);

	MapRender& GetMapRender() const;
	RouteBuilder& GetRouteBuilder();
	const RouteBuilder& GetRouteBuilder() const;

private:

	const TransportCatalogue& catalogue_;
	MapRender* render_ = nullptr;
	RouteBuilder builder_ = {};

};