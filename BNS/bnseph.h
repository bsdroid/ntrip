#ifndef BNSEPH_H
#define BNSEPH_H

#include <QThread>
#include <QtNetwork>

struct gpsephemeris {
  int    flags;            /* GPSEPHF_xxx */
  int    satellite;        /*  SV ID   ICD-GPS data position */
  int    IODE;             /*          [s2w3b01-08]              */
  int    URAindex;         /*  [1..15] [s1w3b13-16]              */
  int    SVhealth;         /*          [s1w3b17-22]              */
  int    GPSweek;          /*          [s1w3b01-10]              */
  int    IODC;             /*          [s1w3b23-32,w8b01-08]     */
  int    TOW;              /*  [s]     [s1w2b01-17]              */
  int    TOC;              /*  [s]     [s1w8b09-24]              */
  int    TOE;              /*  [s]     [s2w10b1-16]              */
  double clock_bias;       /*  [s]     [s1w10b1-22, af0]         */
  double clock_drift;      /*  [s/s]   [s1w9b09-24, af1]         */
  double clock_driftrate;  /*  [s/s^2] [s1w9b01-08, af2]         */
  double Crs;              /*  [m]     [s2w3b09-24]              */
  double Delta_n;          /*  [rad/s] [s2w4b01-16 * Pi]         */
  double M0;               /*  [rad]   [s2w4b17-24,w5b01-24 * Pi]*/
  double Cuc;              /*  [rad]   [s2w6b01-16]              */
  double e;                /*          [s2w6b17-24,w6b01-24]     */
  double Cus;              /*  [rad]   [s2w8b01-16]              */
  double sqrt_A;           /*  [m^0.5] [s2w8b16-24,w9b01-24]     */
  double Cic;              /*  [rad]   [s3w3b01-16]              */
  double OMEGA0;           /*  [rad]   [s3w3b17-24,w4b01-24 * Pi]*/
  double Cis;              /*  [rad]   [s3w5b01-16]              */
  double i0;               /*  [rad]   [s3w5b17-24,w6b01-24 * Pi]*/
  double Crc;              /*  [m]     [s3w701-16]               */
  double omega;            /*  [rad]   [s3w7b17-24,w8b01-24 * Pi]*/
  double OMEGADOT;         /*  [rad/s] [s3w9b01-24 * Pi]         */
  double IDOT;             /*  [rad/s] [s3w10b9-22 * Pi]         */
  double TGD;              /*  [s]     [s1w7b17-24]              */
};

class t_bnseph : public QThread {
 Q_OBJECT
 public:
  t_bnseph(QObject* parent = 0);
  virtual ~t_bnseph();  
  virtual void run();  

 signals:
  void newMessage(const QByteArray msg);
  void error(const QByteArray msg);
 
 private:
  void readEph();
  QTcpSocket* _socket;
};
#endif
