#ifndef BNSEPH_H
#define BNSEPH_H

#include <QThread>
#include <QtNetwork>

class gpsEph {
 public:
  QString prn;
  int    GPSweek;          
  int    TOW;              //  [s]    
  int    TOC;              //  [s]    
  int    TOE;              //  [s]    
  int    IODE;             
  int    IODC;             

  double clock_bias;       //  [s]    
  double clock_drift;      //  [s/s]  
  double clock_driftrate;  //  [s/s^2]

  double Crs;              //  [m]    
  double Delta_n;          //  [rad/s]
  double M0;               //  [rad]  
  double Cuc;              //  [rad]  
  double e;                //         
  double Cus;              //  [rad]  
  double sqrt_A;           //  [m^0.5]
  double Cic;              //  [rad]  
  double OMEGA0;           //  [rad]  
  double Cis;              //  [rad]  
  double i0;               //  [rad]  
  double Crc;              //  [m]    
  double omega;            //  [rad]  
  double OMEGADOT;         //  [rad/s]
  double IDOT;             //  [rad/s]

  double TGD;              //  [s]    
};

class t_bnseph : public QThread {
 Q_OBJECT
 public:
  t_bnseph(QObject* parent = 0);
  virtual ~t_bnseph();  
  virtual void run();  

 signals:
  void newEph(gpsEph* eph);
  void newMessage(const QByteArray msg);
  void error(const QByteArray msg);
 
 private:
  void readEph();
  QTcpSocket* _socket;
};
#endif
