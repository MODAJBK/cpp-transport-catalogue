#include <fstream>
#include <iostream>
#include <string_view>

#include "json_reader.h"
#include "serialization.h"
#include "request_handler.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

void MakeBase() {
    TransportCatalogue catalogue;
    Serializer serializer;
    JsonReader json_reader(std::cin);
    serializer.SetSettings(json_reader.GetSerializationSettings());
    json_reader.BaseRequestsParsing(catalogue);
    MapRender render(catalogue.GetCoordinates(), json_reader.GetRenderSettings());
    RequestHandler handler(catalogue, render, json_reader.GetRoutingSettings());
    handler.BuildGraph();
    serializer.SaveToFile(catalogue, json_reader.GetRenderSettings(), handler.GetRouteBuilder());
}

void Complete() {
    TransportCatalogue catalogue;
    Serializer serializer;
    JsonReader json_reader(std::cin);
    serializer.SetSettings(json_reader.GetSerializationSettings());
    RequestHandler handler(catalogue);
    RenderSettings render_settings = serializer.GetFromFile(catalogue, handler.GetRouteBuilder());
    MapRender render(catalogue.GetCoordinates(), render_settings);
    handler.SetRender(render);
    handler.GetRouteBuilder().SetStopToVertexId(catalogue.GetStopNames());
    std::string map = handler.RenderMap();
    json_reader.StatRequestsParsing(catalogue, map, handler.GetRouteBuilder(), std::cout);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        MakeBase();
    }
    else if (mode == "process_requests"sv) {
        Complete();
    }
    else {
        PrintUsage();
        return 1;
    }
}