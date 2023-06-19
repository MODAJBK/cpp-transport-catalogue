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

std::ostream& operator<<(std::ostream& out, const Stops& bus_stops) {
    out << "-"s << bus_stops.stop_name << '\n'
        << "-"s << bus_stops.latitude << '\n'
        << "-"s << bus_stops.longitude;
    return out;
}

void TestBusStopAdding() {
    TransportCatalogue catalogue;
    Stops bus_stop("Krystall"s, 55.611087, 37.208290);
    Stops bus_stop2("Teplitsya"s, 56.611087, 38.208290);
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
        Stops bus_stop(route_stops[i], 55.611087 + static_cast<double>(i), 37.208290 + static_cast<double>(i));
        catalogue.AddStop(std::move(bus_stop));
    }
    catalogue.AddBus(route_name, std::move(RouteType::LINER_ROUTE), route_stops);
    auto [bus_stops, bus_type] = catalogue.FindBus(route_name);
    ASSERT_EQUAL_HINT(bus_stops->size(), route_stops.size(), "Error in bus route adding"s);
    for (int i = 0; i < bus_stops->size(); ++i) {
        ASSERT_EQUAL_HINT(*bus_stops->at(i), *catalogue.FindStop(route_stops[i]), "Error in bus route adding"s);
    }
}

void TestGetBusInfo() { /*
    TransportCatalogue catalogue;
    {
        std::string route_name = "256"s;
        std::vector<std::string> route_stops = { "Biryulyovo Zapadnoye"s,
                                                 "Biryusinka"s,
                                                 "Universam"s,
                                                 "Biryulyovo Tovarnaya"s,
                                                 "Biryulyovo Passazhirskaya"s,
                                                 "Biryulyovo Zapadnoye"s };
        for (int i = 0; i < route_stops.size(); ++i) {
            Stops bus_stop(route_stops[i], 55.611087 + static_cast<double>(i), 37.208290 + static_cast<double>(i));
            catalogue.AddStop(std::move(bus_stop));
        }
        catalogue.AddBus(route_name, RouteType::RING_ROUTE, route_stops);
        auto [stops, unique_stops, distance, curvature] = catalogue.GetBusInfo(route_name);
        //ASSERT_EQUAL_HINT(stops, route_stops.size(), "Error in GetBusInfo"s);
        //ASSERT_EQUAL_HINT(unique_stops, route_stops.size() - 1, "Error in GetBusInfo"s);
        //ASSERT_EQUAL_HINT(distance, ref_distance, "Error in GetBusInfo"s);
        std::cout << distance << " "s << std::setprecision(7) << curvature << '\n';
    }
    {
        std::string route_name = "750"s;
        std::vector<std::string> route_stops = { "Tolstopaltsevo"s,
                                                 "Marushkino"s,
                                                 "Rasskazovka"s};
        for (int i = 0; i < route_stops.size(); ++i) {
            Stops bus_stop(route_stops[i], 55.611087 + static_cast<double>(i), 37.208290 + static_cast<double>(i));
            catalogue.AddStop(std::move(bus_stop));
        }
        catalogue.AddBus(route_name, std::move(RouteType::LINER_ROUTE), route_stops);
        auto [stops, unique_stops, distance, curvature] = catalogue.GetBusInfo(route_name);
        //ASSERT_EQUAL_HINT(stops, route_stops.size() * 2 - 1, "Error in GetBusInfo"s);
        //ASSERT_EQUAL_HINT(unique_stops, route_stops.size(), "Error in GetBusInfo"s);
        //ASSERT_EQUAL_HINT(distance, ref_distance, "Error in GetBusInfo"s);
        std::cout << distance << " "s << std::setprecision(7) << curvature << '\n';
    } */
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
        Stops bus_stop(route_stops[i], 55.611087 + static_cast<double>(i), 37.208290 + static_cast<double>(i));
        catalogue.AddStop(std::move(bus_stop));
    }
    catalogue.AddBus(route_name, std::move(RouteType::RING_ROUTE), route_stops);
    auto result = catalogue.GetStopInfo("Biryusinka"s);
    ASSERT_EQUAL_HINT(result.value().size(), 1, "Error in getting information about stop"s);
    ASSERT_EQUAL_HINT(result.value().count(route_name), 1, "Error in getting information about stop"s);
    //ASSERT_HINT(catalogue.GetStopInfo("Empty"s).value().empty(), "Error in getting information about stop"s);
}

void TestParseInputRequest() {
    InputReader reader;
    reader.ParseReqestst(std::cin);
    auto result = reader.GetRequestQuery();
    for (const auto& [type, request] : result) {
        std::cout << static_cast<int>(type) << " : " << request << "_" << '\n';
    }
}

void TestParseAddStop() {
    TransportCatalogue catalogue;
    InputReader reader;
    std::string request = { "Marushkino, 55.611087 37.20829"s };
    Stops ref_stop("Marushkino"s, 55.611087, 37.20829);
    reader.ParseAddStop(catalogue, request);
    auto result = catalogue.FindStop("Marushkino"s);
    ASSERT_EQUAL_HINT(*result, ref_stop, "Error in ADD_STOP request parsing"s);
}

void TestParseAddDistance() {
    TransportCatalogue catalogue;
    InputReader reader;
    std::string request = { "Marushkino, 9900m to Rasskazovka, 100m to Biryusinka, 5500m to Krystall"s };
    std::vector<int> distances = { 0, 9900, 100, 5500 };
    std::vector<std::string> route_stops = { "Marushkino"s,
                                             "Rasskazovka"s,
                                             "Biryusinka"s,
                                             "Krystall"s };
    for (int i = 0; i < route_stops.size(); ++i) {
        Stops bus_stop(route_stops[i], 55.611087 + static_cast<double>(i), 37.208290 + static_cast<double>(i));
        catalogue.AddStop(std::move(bus_stop));
    }
    reader.ParseAddDistance(catalogue, request);
    auto result = catalogue.GetDistancesIndex();
    size_t index = 1;
    for (const auto& [stops, distance] : result) {
        auto [from_stop, to_stop] = stops;
        ASSERT_EQUAL_HINT(from_stop->stop_name, route_stops[0], "Error in ADD_DISTANCE request parsing"s);
        ASSERT_EQUAL_HINT(to_stop->stop_name, route_stops[index], "Error in ADD_DISTANCE request parsing"s);
        ASSERT_EQUAL_HINT(distance, distances[index++], "Error in ADD_DISTANCE request parsing"s);
    }
}

void TestTransportCatalogue() {
    RUN_TEST(TestBusStopAdding);
    RUN_TEST(TestBusRouteAdding);
    RUN_TEST(TestGetBusInfo);
    RUN_TEST(TestGetStopInfo);
    RUN_TEST(TestParseInputRequest);
    //RUN_TEST(TestParseAddStop);
    RUN_TEST(TestParseAddDistance);
}