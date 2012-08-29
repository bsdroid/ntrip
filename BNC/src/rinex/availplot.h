#ifndef AVAILPLOT_H
#define AVAILPLOT_H

#include <QtCore>
#include <qwt_plot.h>

class t_availData;

class t_availPlot: public QwtPlot {
 Q_OBJECT

public:
  t_availPlot(QWidget* parent, QMap<QString, t_availData>* availDataMap);

private:

};

#endif
