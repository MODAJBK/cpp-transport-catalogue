#include "test_transport_catalogue.h"
#include "transport_catalogue.h"
#include "request_handler.h"

int main(){
	//TestTransportCatalogue();
	TransportCatalogue catalogue;
	RequestHandler handler;
	EnterRequestAndGetReply(catalogue, handler, std::cin, std::cout);
}