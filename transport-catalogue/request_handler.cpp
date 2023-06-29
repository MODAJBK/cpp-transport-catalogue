#include "request_handler.h"

using namespace std::string_literals;

//Gets request data from parsing JSON-data
void RequestHandler::GetRequestData(std::istream& input) {
	auto [base_requests, stat_requests, render_settings] = RequestsParsing(input);
	base_requests_ = std::move(base_requests);
	stat_requests_ = std::move(stat_requests);
	render_settings_ = std::move(render_settings);
}

void RequestHandler::CompleteInputRequests(TransportCatalogue& catalogue) {
	std::partition(base_requests_.begin(), base_requests_.end(),
		[](const auto& request) {return std::holds_alternative<BaseRequestStop>(request); });
	for (const auto& request : base_requests_) {
		if (std::holds_alternative<BaseRequestStop>(request)) {
			CompleteAddStop(catalogue, std::get<BaseRequestStop>(request));
		}
		else if (std::holds_alternative<BaseRequestDistance>(request)) {
			CompleteSetDistance(catalogue, std::get<BaseRequestDistance>(request));
		}
		else {
			CompleteAddBus(catalogue, std::get<BaseRequestBus>(request));
		}
	}
}

void RequestHandler::CompleteAddStop(TransportCatalogue& catalogue, 
	                                 const BaseRequestStop& request) const {
	catalogue.AddStop(request.stop_info);
}

void RequestHandler::CompleteSetDistance(TransportCatalogue& catalogue,
	                                     const BaseRequestDistance& request) const {
	for (const auto& [stop, distance] : request.stop_to_distance) {
		catalogue.SetDistance(catalogue.FindStop(request.name)->stop_name, 
			                  catalogue.FindStop(stop)->stop_name, distance);
	}
}

void RequestHandler::CompleteAddBus(TransportCatalogue& catalogue,
	                                const BaseRequestBus& request) {
	std::vector<std::string_view> bus_stops;
	std::for_each(request.stops.begin(), request.stops.end(),
		          [&](const auto& stop) { bus_stops.push_back(catalogue.FindStop(stop)->stop_name);
	                                      stop_names_.insert(catalogue.FindStop(stop)->stop_name); });
	route_names_.insert(request.name);
	catalogue.AddBus(request.name,
		             request.is_roundtrip ? RouteType::RING_ROUTE : RouteType::LINER_ROUTE,
		             bus_stops);
}

//Completes requests according to the type
void RequestHandler::CompleteOutputRequests(TransportCatalogue& catalogue, std::ostream& out) const {
	out << '[' << '\n';
	size_t size = stat_requests_.size();
	for (size_t i = 0; i < size; ++i) {
		switch (stat_requests_[i].type) {
		case OutRequestType::ROUTE_INFO:
			PrintRouteRequestResult(catalogue, stat_requests_[i], out);
			break;
		case OutRequestType::STOP_INFO:
			PrintStopRequestResult(catalogue, stat_requests_[i], out);
			break;
		case OutRequestType::DRAW_MAP:
			PrintDrawMapRequestResult(catalogue, stat_requests_[i], out);
			break;
		}
		if (i != size - 1) out << ",\n";
	}
	out << '\n' << ']' << '\n';
}

//Prints ROUTE_INFO request data to the output stream
void RequestHandler::PrintRouteRequestResult(TransportCatalogue& catalogue, 
	                                         const StatRequest& request, std::ostream& out) const {
	auto result = catalogue.GetBusInfo(request.name);
	if (!result.has_value()) {
		out << "    {"s << '\n'
			<< "        \"request_id\": "s << request.id << ",\n"s
			<< "        \"error_message\": \"not found\"\n"s
			<< "    }"s;
		return;
	}
	auto& [stops, unique_stops, distance, curvature] = result.value();
	out << "    {"s << '\n'
		<< "        \"curvature\": "s << curvature << ",\n"s
		<< "        \"request_id\": " << request.id << ",\n"s
		<< "        \"route_length\": "s << distance << ",\n"s
		<< "        \"stop_count\": "s << stops << ",\n"s
		<< "        \"unique_stop_count\": "s << unique_stops << "\n"s
		<< "    }"s;
}

//Prints STOP_INFO request data to the output stream
void RequestHandler::PrintStopRequestResult(TransportCatalogue& catalogue, 
	                                        const StatRequest& request, std::ostream& out) const {
	auto routes = catalogue.GetStopInfo(request.name);
	if (!routes.has_value()) {
		out << "    {"s << '\n'
			<< "        \"request_id\": "s << request.id << ",\n"s
			<< "        \"error_message\": \"not found\"\n"s
			<< "    }"s;
		return;
	}
	out << "    {"s << '\n'
		<< "        \"buses\": [\n            "s;
	for (const auto& route : routes.value()) {
		out << '\"' << route << '\"';
		if (route != *routes.value().rbegin()) out << ", "s;
	}
	out << "\n        ],\n"s
		<< "        \"request_id\": "s << request.id << "\n"s
		<< "    }"s;
}

void RequestHandler::PrintDrawMapRequestResult(TransportCatalogue& catalogue, 
	                                           const StatRequest& request, std::ostream& out) const {
	std::ostringstream strm;
	CompleteMapDrawing(catalogue, strm);
	json::Node node(strm.str());
	out << "    {"s << '\n'
		<< "        \"map\": "s;
	json::Print(json::Document{node}, out);
	out << ",\n"s << "        \"request_id\": "s << request.id
		<< '\n' << "    }";
}

void RequestHandler::CompleteMapDrawing(TransportCatalogue& catalogue, std::ostream& out) const {
	size_t color_index = 0;
	svg::Document doc;
	auto geo_coords = catalogue.GetCoordinates();
	SphereProjector proj{
	   geo_coords.begin(), geo_coords.end(), 
	   render_settings_.width, render_settings_.height, 
	   render_settings_.padding
	};
	for (const auto route : route_names_) {
		const auto [stops, type] = catalogue.FindBus(route);
		if (!stops->empty()) {
			doc.Add(std::move(GetMapPicture(render_settings_, proj, color_index, stops, type)));
		}
	}
	color_index = 0;
	for (const auto route : route_names_) {
		const auto [stops, type] = catalogue.FindBus(route);
		if (!stops->empty()) {
			auto [route_name, route_underline] = GetRouteNamePicture(render_settings_, proj, 
				                                                           color_index,std::string(route), 
				                                                           stops->at(0)->coordinates);
			doc.Add(std::move(route_underline));
			doc.Add(std::move(route_name));
			if (type == RouteType::LINER_ROUTE && stops->at(0) != stops->at(stops->size() - 1)) {
				auto [route_name, route_underline] = GetRouteNamePicture(render_settings_, proj, 
					                                                           color_index, std::string(route), 
					                                                           stops->at(stops->size() - 1)->coordinates);
				doc.Add(std::move(route_underline));
				doc.Add(std::move(route_name));
			}
			if (++color_index == render_settings_.color_palette.size()) color_index = 0;
		}
	}
	for (const auto stop : stop_names_) {
		doc.Add(std::move(GetStopCirclePicture(render_settings_, proj, catalogue.FindStop(stop)->coordinates)));
	}
	for (const auto stop : stop_names_) {
		auto [stop_name, stop_underline] = GetStopNamePicture(render_settings_, proj, std::string(stop), catalogue.FindStop(stop)->coordinates);
		doc.Add(std::move(stop_underline));
		doc.Add(std::move(stop_name));
	}
	doc.Render(out);
}

void EnterRequestAndGetReply(TransportCatalogue& catalogue, RequestHandler& handler) {
	handler.GetRequestData(std::cin);
	handler.CompleteInputRequests(catalogue);
	handler.CompleteOutputRequests(catalogue, std::cout);
}