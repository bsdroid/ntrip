
/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      bncHtml
 *
 * Purpose:    HTML Browser
 *
 * Author:     L. Mervart
 *
 * Created:    14-Sep-2006
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>

#include "bnchtml.h" 

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncHtml::bncHtml() : QTextBrowser() {

  connect(this,SIGNAL(anchorClicked(const QUrl&)),
          this,SLOT(slotAnchorClicked(const QUrl&)));
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncHtml::~bncHtml() {

}

// 
////////////////////////////////////////////////////////////////////////////
void bncHtml::slotAnchorClicked(const QUrl& url) {

  cout << "A>" << url.toString().toAscii().data() << "<B" << endl;
}
