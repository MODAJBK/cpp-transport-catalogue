#include "test_transport_catalogue.h"

int main(){
	//TestTransportCatalogue();
	
	TransportCatalogue catalogue;
	InputReader reader;
	StatReader out_reader;
	EnterInputRequests(catalogue, reader);
	EnterOutPutRequests(catalogue, out_reader);
	
}