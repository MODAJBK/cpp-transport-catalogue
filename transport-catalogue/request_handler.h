#pragma once

#include <iostream>
#include <sstream>

#include "json_reader.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

std::string CompleteMapDrawing(JsonReader&, TransportCatalogue&);

void CompleteInputRequests(JsonReader&, TransportCatalogue&, std::istream&);

void CompleteOutputRequests(JsonReader&, TransportCatalogue&, std::ostream&);

void RunTransportCatalogue();