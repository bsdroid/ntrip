
#include <QApplication>

#include "bncnetrequest.h"

using namespace std;

// Main Program
/////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {

  QApplication app(argc, argv, false);


  bncNetRequest* req = new bncNetRequest();

  QUrl url("http://ntrip2c:rtcm2c@euref-ip.bkg.bund.de:2111");
  req->request(url);

  return app.exec();
}
