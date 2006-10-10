
/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      bncHlpDlg
 *
 * Purpose:    Displays the help
 *
 * Author:     L. Mervart
 *
 * Created:    24-Sep-2006
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include "bnchlpdlg.h"
#include "bnchtml.h"

// Constructor
////////////////////////////////////////////////////////////////////////////
bncHlpDlg::bncHlpDlg(QWidget* parent, const QUrl& url) :
                    QDialog(parent) {

  const int ww = QFontMetrics(font()).width('w');

  bncHtml* _tb = new bncHtml;
  setWindowTitle("Help Contents");
  _tb->setSource(url);
  _tb->setReadOnly(true);
  connect(_tb, SIGNAL(backwardAvailable(bool)),
          this, SLOT(backwardAvailable(bool)));
  connect(_tb, SIGNAL(forwardAvailable(bool)),
          this, SLOT(forwardAvailable(bool)));

  QVBoxLayout* dlgLayout = new QVBoxLayout;
  dlgLayout->addWidget(_tb);

  QHBoxLayout* butLayout = new QHBoxLayout;

  _backButton = new QPushButton("Backward");
  _backButton->setMaximumWidth(10*ww);
  _backButton->setEnabled(false);
  connect(_backButton, SIGNAL(clicked()), _tb, SLOT(backward()));
  butLayout->addWidget(_backButton);

  _forwButton = new QPushButton("Forward");
  _forwButton->setMaximumWidth(10*ww);
  _forwButton->setEnabled(false);
  connect(_forwButton, SIGNAL(clicked()), _tb, SLOT(forward()));
  butLayout->addWidget(_forwButton);

  _closeButton = new QPushButton("Close");
  _closeButton->setMaximumWidth(10*ww);
  butLayout->addWidget(_closeButton);
  connect(_closeButton, SIGNAL(clicked()), this, SLOT(close()));

  dlgLayout->addLayout(butLayout);

  setLayout(dlgLayout);
  resize(60*ww, 60*ww);
  show();
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncHlpDlg::~bncHlpDlg() {
  delete _tb;
  delete _backButton;
  delete _forwButton;
  delete _closeButton;
}

// Slots
////////////////////////////////////////////////////////////////////////////
void bncHlpDlg::backwardAvailable(bool avail) {
  _backButton->setEnabled(avail);
}

void bncHlpDlg::forwardAvailable(bool avail) {
  _forwButton->setEnabled(avail);
}
