#include <iostream>
#include <string>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/concurrency/Thread.h>
#include <thrift/concurrency/PosixThreadFactory.h>

#include "gen-cpp/myService.h"

using namespace std;
using namespace boost;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;
using namespace apache::thrift::concurrency;

class myService : virtual public myServiceIf {
 public:
  myService() {}
  void answer(const std::string& question) {
    // implemented on the client-side only
  }
};

class t_connection {
 public:
  shared_ptr<myService>       _service;
  shared_ptr<myServiceClient> _client;
  shared_ptr<TProcessor>      _processor;
  shared_ptr<TProtocol>       _protocolInp;
  shared_ptr<TProtocol>       _protocolOut;
  shared_ptr<TTransport>      _transport;
};

shared_ptr<t_connection> CONNECTION;

class myProcessorFactory : public TProcessorFactory {
 public:
  myProcessorFactory() {};
  shared_ptr<TProcessor>   getProcessor(const TConnectionInfo& info) {
    shared_ptr<myServiceClient> client(new myServiceClient(info.output));
    shared_ptr<myService>       service(new myService());
    shared_ptr<TProcessor>      processor(new myServiceProcessor(service));
    cout << "connection " << endl;

    CONNECTION.reset(new t_connection);   
    CONNECTION->_service     = service;
    CONNECTION->_client      = client;
    CONNECTION->_processor   = processor;
    CONNECTION->_protocolInp = info.input;
    CONNECTION->_protocolOut = info.output;
    CONNECTION->_transport   = info.transport;
   
    return processor;
  }
};

class t_serverThread : public apache::thrift::concurrency::Runnable {
 public:
  t_serverThread() {}
  ~t_serverThread() {}
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

  shared_ptr<PosixThreadFactory> threadFactory(new PosixThreadFactory);

  shared_ptr<t_serverThread> serverThread(new t_serverThread);  

  shared_ptr<Thread> thread = threadFactory->newThread(serverThread);
  thread->start();

  cout << "server thread started" << endl;

  while (true) {
    cout << "sleep ..." << endl;
    if (CONNECTION) {
      cout << "CONNECTION " << endl;
      CONNECTION->_client->answer("How are you?");
    }
    sleep(1);
  }

  return 0;
}

