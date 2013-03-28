#ifndef GnssCenter_UNILINE_H
#define GnssCenter_UNILINE_H

#include <QtGui>

namespace GnssCenter {

class t_keyword;

class t_uniLine : public QTableWidget {
  Q_OBJECT

 public:
  t_uniLine(const QString& fldMask, const t_keyword* keyword, 
            QWidget* parent = 0);
  ~t_uniLine();

 private slots:
  void slotItemClicked(QTableWidgetItem* item);

 private:
  const t_keyword* _keyword;
};

} // namespace GnssCenter

#endif
