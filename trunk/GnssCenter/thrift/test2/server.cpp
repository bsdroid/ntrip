#include <iostream>
#include <string>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

#include "gen-cpp/myService.h"

using namespace std;
using namespace boost;
using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

class myService : virtual public myServiceIf {
 public:
  myService() {}
  void answer(std::string& answ, const std::string& question) {
    // implemented on the client-side only
  }
};

class myProcessorFactory : virtual public TProcessorFactory {
 public:
  myProcessorFactory() {};
  shared_ptr<TProcessor>   getProcessor(const TConnectionInfo& info) {
    shared_ptr<myService>  service(new myService());
    shared_ptr<TProcessor> processor(new myServiceProcessor(service));
    cout << "connection " << endl;
    return processor;
  }
};

int main(int argc, char **argv) {
  int port = 9090;
  shared_ptr<TServerSocket>      serverTransport(new TServerSocket(port));
  shared_ptr<myProcessorFactory> processorFactory(new myProcessorFactory());
  shared_ptr<TTransportFactory>  transportFactory(new TBufferedTransportFactory());
  shared_ptr<TProtocolFactory>   protocolFactory(new TBinaryProtocolFactory());

  TThreadedServer server(processorFactory, serverTransport,
                         transportFactory, protocolFactory);

  server.serve();

  return 0;
}

