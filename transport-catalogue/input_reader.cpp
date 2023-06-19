#include "input_reader.h"

void InputReader::ParseReqestst(std::istream& input) {
	int requsets_number = 0;
	std::string line;
	input >> requsets_number >> std::ws;
	request_query_.reserve(requsets_number);
	for (int i = 0; i < requsets_number; ++i) {
		std::getline(input, line);
		AddRequestToQuery(line);
	}
	return;
}

void InputReader::CompleteRequests(TransportCatalogue& catalogue) {
	std::partition(request_query_.begin(), request_query_.end(), 
		           [](const auto& request) {return request.first == InRequestType::ADD_STOP; });
	for (const auto& [request_type, request] : request_query_) {
		switch (request_type)
		{
		    case InRequestType::ADD_STOP:
				ParseAddStop(catalogue, request);
			    break;
			case InRequestType::ADD_DISTANCE:
				ParseAddDistance(catalogue, request);
				break;
			case InRequestType::ADD_BUS:
				ParseAddBus(catalogue, request);
				break;
		}
	}
	return;
}

void InputReader::AddRequestToQuery(std::string_view request) {
	if (request[0] == 'B') request_query_.push_back({ InRequestType::ADD_BUS, std::move(std::string(request)) }); // Only ADD_BUS request starts with "B" letter
	else {
		std::vector<std::string> request_words;
		size_t begin_of_field = request.find_first_of(' ') + 1; // +1 need to find position of the 1st stop name character
		size_t end_of_field = request.find(':'); // Stop name ends with ":" symbol
		request_words.push_back(std::move(std::string(request.substr(begin_of_field, end_of_field - begin_of_field))));
		for (int i = 0; i < 2; ++i) { // loop with two passes to add coordinates
			request.remove_prefix(end_of_field + 1); // +1 need to remove ":" symbol
			end_of_field = std::min(request.find(','), request.npos);
			request_words.push_back(std::move(std::string(request.substr(0, end_of_field))));
		}
		request_query_.push_back({ InRequestType::ADD_STOP, request_words[0] + ',' + request_words[1] + request_words[2]}); // comma needs to separate stop name and coordinates
		if (end_of_field != request.npos) 
			request_query_.push_back({ InRequestType::ADD_DISTANCE, request_words[0] + std::string(request.substr(end_of_field, request.npos))});
	}
	return;
}

std::vector<std::pair<InRequestType, std::string>> InputReader::GetRequestQuery() const {
	return request_query_;
}

void InputReader::ParseAddStop(TransportCatalogue& catalogue, std::string_view line) const {
	Stops bus_stop;
	size_t end_of_field = line.find(','); // Stop name separate with coordinates by comma symbol
	bus_stop.stop_name = std::move(line.substr(0, end_of_field));
	line.remove_prefix(end_of_field + 1); // +1 need to remove comma symbol
	end_of_field = line.find_last_of(' '); // digits separate with space symbol
	bus_stop.latitude = std::stod(std::string(line.substr(0, end_of_field)));
	line.remove_prefix(end_of_field + 1); // +1 need to remove space symbol
	bus_stop.longitude = std::stod(std::string(line));
	catalogue.AddStop(bus_stop);
	return;
}

//Format of ADD_DISTANCE request: "<from_stop> <distance_1>m to <to_stop1>, -/-/- <distance_N>m to <to_stopN>"
void InputReader::ParseAddDistance(TransportCatalogue& catalogue, std::string_view line) const {
	std::string stop_name, stop_name2;
	size_t end_of_field = line.find(','); //stop name ends with space symbol or comma symbol
	stop_name = std::move(line.substr(0, end_of_field));
	line.remove_prefix(end_of_field + 1); //+1 need to remove space symbol
	while (end_of_field != line.npos) {
		end_of_field = line.find('m'); //distance value end with letter "m"
		int distance = std::stoi(std::string(line.substr(0, end_of_field)));
		line.remove_prefix(end_of_field + 5); //+5 need to remove "m to " from request
		end_of_field = line.find(','); //distances between stops divide with comma symbol
		stop_name2 = std::move(line.substr(0, end_of_field));
		line.remove_prefix(end_of_field + 2); //+2 need to remove comma and space symbols
		catalogue.AddDistance(stop_name, stop_name2, distance);
	}
	return;
}

void InputReader::ParseAddBus(TransportCatalogue& catalogue, std::string_view line) const {
	bool is_route_liner;
	std::vector<std::string> bus_stops;
	std::string route_name;
	size_t begin_of_field = line.find_first_of(' ') + 1;
	size_t end_of_field = line.find(':');
	route_name = std::move(line.substr(begin_of_field, end_of_field - begin_of_field));
	line.remove_prefix(end_of_field + 1);
	is_route_liner = line.find('>') == line.npos ? true : false;
	RouteType route_type = is_route_liner ? RouteType::LINER_ROUTE : RouteType::RING_ROUTE;
	while (end_of_field != line.npos) {
		end_of_field = is_route_liner ? line.find('-') : line.find('>');
		std::string stop_name = std::string(line.substr(1, end_of_field - 2));
		bus_stops.push_back(std::move(stop_name));
		line.remove_prefix(end_of_field + 1);
	}
	catalogue.AddBus(std::move(route_name), std::move(route_type), std::move(bus_stops));
	return;
}

void EnterInputRequests(TransportCatalogue& catalogue, InputReader& reader) {
	reader.ParseReqestst(std::cin);
	reader.CompleteRequests(catalogue);
}