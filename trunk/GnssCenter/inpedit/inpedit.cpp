
/* -------------------------------------------------------------------------
 * RTNet GUI
 * -------------------------------------------------------------------------
 *
 * Class:      t_inpEdit
 *
 * Purpose:    RTNet Input File
 *
 * Author:     L. Mervart
 *
 * Created:    05-Jan-2013
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include "inpedit.h" 
#include "keyword.h" 
#include "panel.h" 

using namespace std;
using namespace GnssCenter;

Q_EXPORT_PLUGIN2(gnsscenter_inpedit, GnssCenter::t_inpEditFactory)

// Constructor
////////////////////////////////////////////////////////////////////////////
t_inpEdit::t_inpEdit() : QMainWindow() {
  _tabWidget = new t_tabWidget();
  setCentralWidget(_tabWidget);
  _tabWidget->setVisible(true);
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_inpEdit::~t_inpEdit() {
}
