
#include <QApplication>

#include "bncnetrequest.h"

using namespace std;

// Main Program
/////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {

  QApplication app(argc, argv, false);

  QUrl url;
  bncNetRequest req;

  req.request(url);

  return app.exec();
}
