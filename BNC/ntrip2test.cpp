
#include <QApplication>

#include "bncnetrequest.h"

using namespace std;

// Main Program
/////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {

  QApplication app(argc, argv, false);


  bncNetRequest* req1 = new bncNetRequest();
  QUrl url1("http://euref-ip.bkg.bund.de:2111/");
  req1->request(url1);

  bncNetRequest* req2 = new bncNetRequest();
  QUrl url2("http://ntrip2c:rtcm2c@euref-ip.bkg.bund.de:2111/TEST2");
  req2->request(url2);

  return app.exec();
}
