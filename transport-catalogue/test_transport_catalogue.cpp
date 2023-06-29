#include "test_transport_catalogue.h"

using namespace std::string_literals;

void AssertImpl(bool value, const std::string& expr_str, const std::string& file, const std::string& func, unsigned line,
    const std::string& hint) {
    if (!value) {
        std::cout << file << "("s << line << "): "s << func << ": "s;
        std::cout << "ASSERT("s << expr_str << ") failed."s;
        if (!hint.empty()) {
            std::cout << " Hint: "s << hint;
        }
        std::cout << std::endl;
        abort();
    }
}

std::vector<std::string_view> VectStringToVectStringView(const std::vector<std::string>& source) {
    std::vector<std::string_view> result;
    result.reserve(source.size());
    for (const auto& s : source) {
        result.emplace_back(std::string_view(s));
    }
    return result;
}

std::ostream& operator<<(std::ostream& out, const Stop& bus_stops) {
    out << "{"s << bus_stops.stop_name
        << ", "s << bus_stops.coordinates.lat
        << ", "s << bus_stops.coordinates.lng << "}"s;
    return out;
}

std::ostream& operator<<(std::ostream& out, const StatRequest& request) {
    out << "{"s << request.id
        << ", "s << static_cast<int>(request.type)
        << ", "s << request.name << "}"s;
    return out;
 }

std::ostream& operator<<(std::ostream& out, RouteType type) {
    out << static_cast<int>(type);
    return out;
}

json::Document LoadJSON(const std::string& s) {
    std::istringstream strm(s);
    return json::Load(strm);
}

void TestBusStopAdding() {
    TransportCatalogue catalogue;
    Stop bus_stop("Krystall"s, 55.611087, 37.208290);
    Stop bus_stop2("Teplitsya"s, 56.611087, 38.208290);
    catalogue.AddStop(bus_stop);
    catalogue.AddStop(bus_stop2);
    auto result = catalogue.FindStop(bus_stop.stop_name);
    ASSERT_EQUAL_HINT(*result, bus_stop, "Error in bus stop adding"s);
    result = catalogue.FindStop(bus_stop2.stop_name);
    ASSERT_EQUAL_HINT(*result, bus_stop2, "Error in bus stop adding"s);
}

void TestBusRouteAdding() {
    TransportCatalogue catalogue;
    std::string route_name = "35"s;
    std::vector <std::string> route_stops = { "Krystall"s, "Teplyitca"s, "Center"s };
    for (int i = 0; i < route_stops.size(); ++i) {
        Stop bus_stop(route_stops[i], 55.611087 + static_cast<double>(i), 37.208290 + static_cast<double>(i));
        catalogue.AddStop(std::move(bus_stop));
    }
    catalogue.AddBus(route_name, std::move(RouteType::LINER_ROUTE), VectStringToVectStringView(route_stops));
    auto [bus_stops, bus_type] = catalogue.FindBus(route_name);
    ASSERT_EQUAL_HINT(bus_stops->size(), route_stops.size(), "Error in bus route adding"s);
    for (int i = 0; i < bus_stops->size(); ++i) {
        ASSERT_EQUAL_HINT(*bus_stops->at(i), *catalogue.FindStop(route_stops[i]), "Error in bus route adding"s);
    }
}

void TestStopDistanceAdding() {
    TransportCatalogue catalogue;
    Stop stop1("Krystall"s, 55.611087, 37.20829), stop2("Center"s, 55.595884, 37.209755);
    std::string route_name = { "36"s };
    std::vector<std::string> route_stops = { "Krystall"s, "Center"s };
    double distance1 = 1000, distance2 = 1500;
    catalogue.AddStop(stop1);
    catalogue.AddStop(stop2);
    catalogue.SetDistance(stop1.stop_name, stop2.stop_name, 1000);
    catalogue.SetDistance(stop2.stop_name, stop1.stop_name, 1500);
    catalogue.AddBus(route_name, RouteType::LINER_ROUTE, VectStringToVectStringView(route_stops));
    auto stop_ptr1 = catalogue.FindStop(stop1.stop_name);
    auto stop_ptr2 = catalogue.FindStop(stop2.stop_name);
    auto [_, __, distance, ___] = catalogue.GetBusInfo(route_name).value();
    ASSERT_EQUAL_HINT(distance, distance1 + distance2, "Error in stop distances adding"s);
}

void TestGetBusInfo() {
    TransportCatalogue catalogue;
    {
        std::string route_name = "256"s;
        std::vector<std::string> route_stops = { "Biryulyovo Zapadnoye"s,
                                                 "Biryusinka"s,
                                                 "Universam"s,
                                                 "Biryulyovo Tovarnaya"s,
                                                 "Biryulyovo Passazhirskaya"s,
                                                 "Biryulyovo Zapadnoye"s };
        std::vector<double> distances = { 1000.0, 540.0, 3300.0, 950.0, 220.0 };
        for (int i = 0; i < route_stops.size(); ++i) {
            Stop bus_stop(route_stops[i], 55.611087 + static_cast<double>(i), 37.208290 + static_cast<double>(i));
            catalogue.AddStop(std::move(bus_stop));
        }
        for (int i = 0; i < route_stops.size() - 1; ++i) {
            auto from_stop = catalogue.FindStop(route_stops[i]);
            auto to_stop = catalogue.FindStop(route_stops[i + 1]);
            catalogue.SetDistance(from_stop->stop_name, to_stop->stop_name, distances[i]);
        }
        catalogue.AddBus(route_name, RouteType::RING_ROUTE, VectStringToVectStringView(route_stops));
        auto [stops, unique_stops, distance, curvature] = catalogue.GetBusInfo(route_name).value();
        ASSERT_EQUAL_HINT(stops, route_stops.size(), "Error in GetBusInfo"s);
        ASSERT_EQUAL_HINT(unique_stops, route_stops.size() - 1, "Error in GetBusInfo"s);
        ASSERT_EQUAL_HINT(distance, 
                          std::accumulate(distances.begin(), distances.end(), 0, [](double res, double value) {return res += value;}),
                          "Error in GetBusInfo"s);
    }
    {
        std::string route_name = "750"s;
        std::vector<std::string> route_stops = { "Tolstopaltsevo"s,
                                                 "Marushkino"s,
                                                 "Rasskazovka"s};
        std::vector<double> distances = { 1000.0, 2000.0 };
        for (int i = 0; i < route_stops.size(); ++i) {
            Stop bus_stop(route_stops[i], 55.611087 + static_cast<double>(i), 37.208290 + static_cast<double>(i));
            catalogue.AddStop(std::move(bus_stop));
        }
        for (int i = 0; i < route_stops.size() - 1; ++i) {
            auto from_stop = catalogue.FindStop(route_stops[i]);
            auto to_stop = catalogue.FindStop(route_stops[i + 1]);
            catalogue.SetDistance(from_stop->stop_name, to_stop->stop_name, distances[i]);
        }
        catalogue.AddBus(route_name, std::move(RouteType::LINER_ROUTE), VectStringToVectStringView(route_stops));
        auto [stops, unique_stops, distance, curvature] = catalogue.GetBusInfo(route_name).value();
        ASSERT_EQUAL_HINT(stops, route_stops.size() * 2 - 1, "Error in GetBusInfo"s);
        ASSERT_EQUAL_HINT(unique_stops, route_stops.size(), "Error in GetBusInfo"s);
        ASSERT_EQUAL_HINT(distance, (1000.0 + 2000.0) * 2, "Error in GetBusInfo"s);
    }
}

void TestGetStopInfo() {
    TransportCatalogue catalogue;
    std::string route_name = "256"s;
    std::vector<std::string> route_stops = { "Biryulyovo Zapadnoye"s,
                                             "Biryusinka"s,
                                             "Universam"s,
                                             "Biryulyovo Tovarnaya"s,
                                             "Biryulyovo Passazhirskaya"s,
                                             "Biryulyovo Zapadnoye"s };
    for (int i = 0; i < route_stops.size(); ++i) {
        Stop bus_stop(route_stops[i], 55.611087 + static_cast<double>(i), 37.208290 + static_cast<double>(i));
        catalogue.AddStop(std::move(bus_stop));
    }
    catalogue.AddBus(route_name, std::move(RouteType::RING_ROUTE), VectStringToVectStringView(route_stops));
    auto result = catalogue.GetStopInfo("Biryusinka"s);
    ASSERT_EQUAL_HINT(result.value().size(), 1, "Error in getting information about stop"s);
    ASSERT_EQUAL_HINT(result.value().count(route_name), 1, "Error in getting information about stop"s);
    ASSERT_HINT(!catalogue.GetStopInfo("Empty"s).has_value(), "Error in getting information about stop"s);
}

void TestRequestParsing() {
    StatRequest s1(2, OutRequestType::ROUTE_INFO, "114"s),
                s2(1, OutRequestType::STOP_INFO, "Krystall"s);
    BaseRequestBus b("114"s, false, { "Zolotoryovo"s, "Krystall"s});
    const auto json_request = "{\n"
                              "\"base_requests\": \n["
                              "{ \"type\": \"Bus\",\n"
                                "\"name\" : \"114\",\n"
                                "\"stops\" : [\"Zolotoryovo\", \"Krystall\"] ,\n"
                                "\"is_roundtrip\" : false },\n"
                              "{ \"type\" : \"Stop\",\n"
                                "\"name\" : \"Krystall\",\n"
                                "\"latitude\" : 43.587795,\n"
                                "\"longitude\" : 39.716901,\n"
                                "\"road_distances\" : {\"Zolotoryovo\": 850} },\n"
                              "{ \"type\": \"Stop\",\n"
                                 "\"name\" : \"Zolotoryovo\",\n"
                                 "\"latitude\" : 43.581969,\n"
                                 "\"longitude\" : 39.719848,\n"
                                 "\"road_distances\" : {\"Krystall\": 850} } ],\n"
                              "\"stat_requests\": [\n"
                              "{ \"id\": 1, \"type\" : \"Stop\", \"name\" : \"Krystall\" },\n"
                              "{ \"id\": 2, \"type\" : \"Bus\", \"name\" : \"114\" } ]";
    std::istringstream strm(json_request);
    auto [input, output, _] = RequestsParsing(strm);
    auto& add_bus = std::get<BaseRequestBus>(input[0]);
    auto& add_stop = std::get<BaseRequestStop>(input[1]);
    auto& add_stop2 = std::get<BaseRequestStop>(input[3]);
    ASSERT_HINT(output[0].name == s2.name && output[0].id == s2.id, "Error in request parsing"s);
    ASSERT_HINT(output[1].name == s1.name && output[1].id == s1.id, "Error in request parsing"s);
    ASSERT_HINT(add_bus.name == b.name && add_bus.stops == b.stops, "Error in request parsing"s);
    ASSERT_HINT(add_stop.stop_info == Stop("Krystall"s, 43.587795, 39.716901), "Error in request parsing"s);
    ASSERT_HINT(add_stop2.stop_info == Stop("Zolotoryovo"s, 43.581969, 39.719848), "Error in request parsing"s);
}

void TestTransportCatalogue() {
    RUN_TEST(TestBusStopAdding);
    RUN_TEST(TestBusRouteAdding);
    RUN_TEST(TestStopDistanceAdding);
    RUN_TEST(TestGetBusInfo);
    RUN_TEST(TestGetStopInfo);
    RUN_TEST(TestRequestParsing);
}