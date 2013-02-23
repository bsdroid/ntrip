#include <iostream>
#include <string>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/concurrency/Thread.h>

#include "gen-cpp/myService.h"

using namespace std;
using namespace boost;
using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using namespace ::apache::thrift::concurrency;

class myService : virtual public myServiceIf {
 public:
  myService() {}
  void answer(std::string& answ, const std::string& question) {
    // implemented on the client-side only
  }
};

class t_connection {
 public:
  shared_ptr<myService>  _service;
  shared_ptr<TProcessor> _processor;
  shared_ptr<TProtocol>  _protocolInp;
  shared_ptr<TProtocol>  _protocolOut;
  shared_ptr<TTransport> _transport;
};

class myProcessorFactory : public TProcessorFactory {
 public:
  myProcessorFactory() {};
  shared_ptr<TProcessor>   getProcessor(const TConnectionInfo& info) {
    shared_ptr<myService>  service(new myService());
    shared_ptr<TProcessor> processor(new myServiceProcessor(service));
    cout << "connection " << endl;
    
    t_connection connection;
    connection._service     = service;
    connection._processor   = processor;
    connection._protocolInp = info.input;
    connection._protocolOut = info.output;
    connection._transport   = info.transport;
   
    return processor;
  }
};

class t_serverThread : public Runnable {
 public:
  t_serverThread() {}
  void run() {
    int port = 9090;
    shared_ptr<TServerSocket>      serverTransport(new TServerSocket(port));
    shared_ptr<myProcessorFactory> processorFactory(new myProcessorFactory());
    shared_ptr<TTransportFactory>  transportFactory(new TBufferedTransportFactory());
    shared_ptr<TProtocolFactory>   protocolFactory(new TBinaryProtocolFactory());

    TThreadedServer server(processorFactory, serverTransport,
                           transportFactory, protocolFactory);
    server.serve();
  }
};

int main(int argc, char **argv) {

  t_serverThread serverThread;
  serverThread.run();

  while (true) {
    sleep(1);
  }

  return 0;
}

