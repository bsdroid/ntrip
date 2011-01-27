/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      bncComb
 *
 * Purpose:    Combinations of Orbit/Clock Corrections
 *
 * Author:     L. Mervart
 *
 * Created:    22-Jan-2011
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>

#include "bnccomb.h"
#include "bncapp.h"
#include "bncsettings.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncComb::bncComb() {

  bncSettings settings;

  QStringList combineStreams = settings.value("combineStreams").toStringList();
  
  _nStreams = combineStreams.size();
  
  if (_nStreams >= 2) {
    QListIterator<QString> it(combineStreams);
    while (it.hasNext()) {
      QStringList hlp = it.next().split(" ");
      cout << "combination: " << hlp[0].toAscii().data() << endl;
    }
  }
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncComb::~bncComb() {
}

// 
////////////////////////////////////////////////////////////////////////////
void bncComb::processCorrLine(const QString& staID, const QString& line) {
  QMutexLocker locker(&_mutex);

  t_corr* newCorr = new t_corr();
  if (!newCorr->readLine(line) == success) {
    delete newCorr;
    return;
  }

  cout << staID.toAscii().data() << " " << line.toAscii().data() << endl;
}

