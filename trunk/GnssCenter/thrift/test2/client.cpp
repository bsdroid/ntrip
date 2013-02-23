#include <iostream>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "gen-cpp/myService.h"

using namespace std;
using namespace boost;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

class myServiceHandler : virtual public myServiceIf {
 public:
  myServiceHandler() {}
  void answer(std::string& answ, const std::string& question) {
    cout << "Server asks: " << question << endl;
    answ = "I am well, thanks.";
  }

};

int main(int argc, char** argv) {
  int port = 9090;
  shared_ptr<TSocket>          socket(new TSocket("localhost", port));
  shared_ptr<TTransport>       transport(new TBufferedTransport(socket));
  shared_ptr<TProtocol>        protocol(new TBinaryProtocol(transport));

  shared_ptr<myServiceHandler> handler(new myServiceHandler());
  shared_ptr<TProcessor>       processor(new myServiceProcessor(handler));

  try {
    transport->open();
   
    while (processor->process(protocol, protocol, 0)) {}

    transport->close();
  } catch (TException& exc) {
    cout << "Exception: " << exc.what() << endl;
  }

  return 0;
}
