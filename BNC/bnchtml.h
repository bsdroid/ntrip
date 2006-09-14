
#ifndef BNCHTML_H
#define BNCHTML_H

#include <QTextBrowser>

class bncHtml : public QTextBrowser {
  Q_OBJECT

  public:
    bncHtml();
    ~bncHtml();

  public slots:
    void slotAnchorClicked(const QUrl& url);
};
#endif
