#pragma once

#include <iostream>
#include <utility>
#include <string>
#include <vector>
#include <algorithm>

#include "transport_catalogue.h"

enum class InRequestType {
	ADD_STOP,
	ADD_DISTANCE,
	ADD_BUS
};

class InputReader {
public:

	void ParseReqestst(std::istream&);
	void CompleteRequests(TransportCatalogue&);

	void ParseAddStop(TransportCatalogue&, std::string_view) const;
	void ParseAddDistance(TransportCatalogue&, std::string_view) const;
	void ParseAddBus(TransportCatalogue&, std::string_view) const;

	std::vector<std::pair<InRequestType, std::string>> GetRequestQuery() const;

private:

	std::vector<std::pair<InRequestType, std::string>> request_query_;

	void AddRequestToQuery(std::string_view);

};

void EnterInputRequests(TransportCatalogue&, InputReader&);