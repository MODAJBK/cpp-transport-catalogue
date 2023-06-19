#include "stat_reader.h"

void StatReader::ParseOutputReqest(std::istream& input) {
	int requests_number = 0;
	std::string line;
	input >> requests_number >> std::ws;
	for (int i = 0; i < requests_number; ++i) {
		std::getline(input, line);
		OutRequestType type = line[0] == 'B' ? OutRequestType::ROUTE_INFO : OutRequestType::STOP_INFO;
		requests_query_.push_back({ type, line.substr(line.find_first_of(' ') + 1, line.npos) });
	}
	return;
}

void StatReader::CompleteRequests(TransportCatalogue& catalogue) const {
	for (const auto& [type, request] : requests_query_) {
		type == OutRequestType::ROUTE_INFO ? PrintRouteRequestResult(catalogue, request)
			                               : PrintStopRequestResult(catalogue, request);
	}
	return;
}

void StatReader::PrintRouteRequestResult(TransportCatalogue& catalogue, const std::string& line) const {
	using namespace std::string_literals;
	auto result = catalogue.GetBusInfo(line);
	if (!result.has_value()) {
		std::cout << "Bus "s << line << ": not found"s << '\n';
		return;
	}
	auto& [stops, unique_stops, distance, curvature] = result.value();
	std::cout << "Bus "s << line << ": "s
		<< stops << " stops on route, "s
		<< unique_stops << " unique stops, "s
		<< std::setprecision(6) <<  distance << " route length, "s
		<< curvature << " curvature"s << '\n';
	return;
}

void StatReader::PrintStopRequestResult(TransportCatalogue& catalogue, const std::string& line) const {
	using namespace std::string_literals;
	auto routes = catalogue.GetStopInfo(line);
	if (!routes.has_value()) {
		std::cout << "Stop "s << line << ": not found"s << '\n';
		return;
	}
	else if (routes.value().empty()) {
		std::cout << "Stop "s << line << ": no buses"s << '\n';
		return;
	}
	std::cout << "Stop "s << line << ": buses"s;
	for (const auto& route : routes.value()) {
		std::cout << ' ' << route;
	}
	std::cout << '\n';
}

void EnterOutPutRequests(TransportCatalogue& catalogue, StatReader& reader) {
	reader.ParseOutputReqest(std::cin);
	reader.CompleteRequests(catalogue);
	return;
}