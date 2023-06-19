#pragma once

#include <iostream>
#include <utility>
#include <iomanip>
#include <string>
#include <vector>
#include <tuple>

#include "transport_catalogue.h"

enum class OutRequestType {
	ROUTE_INFO,
	STOP_INFO
};

class StatReader {
public:

	void ParseOutputReqest(std::istream&);
	void CompleteRequests(TransportCatalogue&) const;

private:

	std::vector<std::pair<OutRequestType, std::string>> requests_query_;

	void PrintRouteRequestResult(TransportCatalogue&, const std::string& request) const;
	void PrintStopRequestResult(TransportCatalogue&, const std::string& request) const;

};

void EnterOutPutRequests(TransportCatalogue&, StatReader&);