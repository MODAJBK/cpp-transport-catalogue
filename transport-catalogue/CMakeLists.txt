cmake_minimum_required(VERSION 3.10)

project(TransportCatalogue CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

set(CATALOGUE_PROTO transport_catalogue.proto)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS svg.proto map_renderer.proto graph.proto transport_router.proto transport_catalogue.proto)

set(CATALOGUE_SOURCE domain.cpp geo.cpp json.cpp json_builder.cpp 
                     json_reader.cpp serialization.cpp svg.cpp
                     transport_catalogue.cpp request_handler.cpp
                     map_renderer.cpp transport_router.cpp)
set(CATALOGUE_HEADER domain.h geo.h json.h json_builder.h 
                     json_reader.h serialization.h svg.h
                     transport_catalogue.h request_handler.h
                     map_renderer.h transport_router.h
                     graph.h ranges.h router.h)
set(CATALOGUE_FILES main.cpp ${CATALOGUE_HEADER} ${CATALOGUE_SOURCE} ${CATALOGUE_PROTO})

add_executable(transport_catalogue ${PROTO_SRCS} ${PROTO_HDRS} ${CATALOGUE_FILES})

target_include_directories(transport_catalogue PRIVATE ${CATALOGUE_HEADER})
target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(transport_catalogue ${Protobuf_LIBRARY} Threads::Threads)