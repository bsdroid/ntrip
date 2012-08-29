#ifndef AVAILPLOT_H
#define AVAILPLOT_H

#include <QtCore>
#include <qwt_plot.h>

class t_availPlot: public QwtPlot {
 Q_OBJECT

public:
  t_availPlot(QWidget* parent, QMap<QString, QVector<int> >* prnAvail);

private:

};

#endif
