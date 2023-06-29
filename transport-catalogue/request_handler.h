#pragma once

#include <iostream>
#include <sstream>
#include <utility>
#include <string>
#include <string_view>
#include <vector>
#include <variant>
#include <algorithm>

#include "domain.h"
#include "json_reader.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

class RequestHandler {
public:

	void GetRequestData(std::istream&);

	void CompleteInputRequests(TransportCatalogue&);
	void CompleteAddStop(TransportCatalogue&, const BaseRequestStop&) const;
	void CompleteSetDistance(TransportCatalogue&, const BaseRequestDistance&) const;
	void CompleteAddBus(TransportCatalogue&, const BaseRequestBus&);

	void CompleteOutputRequests(TransportCatalogue&, std::ostream&) const;

	void CompleteMapDrawing(TransportCatalogue&, std::ostream&) const;

	void PrintRouteRequestResult(TransportCatalogue&, const StatRequest&, std::ostream&) const;
	void PrintStopRequestResult(TransportCatalogue&, const StatRequest&, std::ostream&) const;
	void PrintDrawMapRequestResult(TransportCatalogue&, const StatRequest&, std::ostream&) const;

private:

	std::vector<BaseRequest> base_requests_;
	std::vector<StatRequest> stat_requests_;
	std::set<std::string_view> stop_names_;
	std::set<std::string_view> route_names_;
	RenderSettings render_settings_;

};

void EnterRequestAndGetReply(TransportCatalogue&, RequestHandler&);