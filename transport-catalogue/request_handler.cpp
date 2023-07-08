#include "request_handler.h"

void CompleteInputRequests(JsonReader& json_reader, TransportCatalogue& catalogue, std::istream& input) {
	json_reader.RequestParsing(input);
	json_reader.CompleteInputRequests(catalogue);
}

void CompleteOutputRequests(JsonReader& json_reader, TransportCatalogue& catalogue, std::ostream& out) {
	json_reader.CompleteOutputRequests(catalogue, CompleteMapDrawing(json_reader, catalogue), out);
}

std::string CompleteMapDrawing(JsonReader& json_reader, TransportCatalogue& catalogue) {
	std::ostringstream strm;
	auto geo_coords = catalogue.GetCoordinates();
	auto [render_settings, stop_names, route_names] = json_reader.GetDataForMapDrawing();
	MapRender map_render(geo_coords, std::move(render_settings));
	map_render.DrawRoutes(catalogue, route_names);
	map_render.DrawStops(catalogue, stop_names);
	map_render.DrawTransportCatalogue(strm);
	return strm.str();
}

void RunTransportCatalogue() {
	TransportCatalogue catalogue;
	JsonReader json_reader;
	CompleteInputRequests(json_reader, catalogue, std::cin);
	CompleteOutputRequests(json_reader, catalogue, std::cout);
}