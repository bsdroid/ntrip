
QT += svg

TARGET = ../bnc

# Switch to debug configuration
# -----------------------------
CONFIG -= release
CONFIG += debug

DEFINES += NO_RTCM3_MAIN 
###DEFINES += DEBUG_RTCM2_2021
unix:DEFINES  += _TTY_POSIX_
win32:DEFINES += _TTY_WIN_

RESOURCES += bnc.qrc

unix:QMAKE_CFLAGS_RELEASE   -= -O2
unix:QMAKE_CXXFLAGS_RELEASE -= -O2

# Get rid of mingwm10.dll and libgcc_s_dw2-1.dll
# ----------------------------------------------
win32 {
  QMAKE_LFLAGS                 += -static-libgcc
  QMAKE_LFLAGS                 -= -mthreads
  QMAKE_CXXFLAGS_EXCEPTIONS_ON -= -mthreads
  QMAKE_LFLAGS_EXCEPTIONS_ON   -= -mthreads
}

debug:OBJECTS_DIR=.obj/debug
debug:MOC_DIR=.moc/debug
release:OBJECTS_DIR=.obj/release
release:MOC_DIR=.moc/release

# Include Path
# ------------
INCLUDEPATH = . ../newmat ./RTCM3 ./RTCM3/clock_and_orbit ./RTCM3/rtcm3torinex \
              ../qwt ../qwtpolar

# Additional Libraries
# --------------------
unix:LIBS  += -L../newmat -lnewmat -L../qwt -L../qwtpolar -lqwtpolar -lqwt
win32:LIBS += -L../newmat/release -L../qwt/release -L../qwtpolar/release \
              -lnewmat -lqwtpolar -lqwt

HEADERS = bnchelp.html bncgetthread.h    bncwindow.h   bnctabledlg.h  \
          bnccaster.h bncrinex.h bncapp.h bncutils.h   bnchlpdlg.h    \
          bncconst.h bnchtml.h bnctableitem.h bnczerodecoder.h        \
          bncnetquery.h bncnetqueryv1.h bncnetqueryv2.h               \
          bncnetqueryrtp.h bncsettings.h latencychecker.h             \
          bncipport.h bncnetqueryv0.h bncnetqueryudp.h                \ 
          bncnetqueryudp0.h bncudpport.h bnctime.h pppopt.h           \ 
          bncserialport.h bncnetquerys.h bncfigure.h                  \ 
          bncfigurelate.h bncpppclient.h bncversion.h                 \ 
          bancroft.h bncmodel.h bncfigureppp.h bncrawfile.h           \ 
          bnctides.h bncmap.h bncantex.h                              \
          bncephuser.h bncoutf.h bncclockrinex.h bncsp3.h             \
          bncbytescounter.h bncsslconfig.h reqcdlg.h                  \
          upload/bncrtnetdecoder.h upload/bncuploadcaster.h           \
          upload/bncrtnetuploadcaster.h upload/bnccustomtrafo.h       \
          upload/bncephuploadcaster.h bnccomb.h qtfilechooser.h       \
          RTCM/GPSDecoder.h RTCM/RTCM2.h RTCM/RTCM2Decoder.h          \
          RTCM/RTCM2_2021.h RTCM/rtcm_utils.h                         \
          RTCM3/RTCM3Decoder.h RTCM3/rtcm3torinex/rtcm3torinex.h      \
          RTCM3/rtcm3torinex/rtcm3torinexsupport.h                    \
          RTCM3/RTCM3coDecoder.h                                      \
          RTCM3/clock_and_orbit/clock_orbit_rtcm.h                    \
          RTCM3/ephemeris.h RTCM3/timeutils.h                         \
          GPSS/gpssDecoder.h GPSS/hassDecoder.h

HEADERS       += serial/qextserialbase.h serial/qextserialport.h
unix:HEADERS  += serial/posix_qextserialport.h
win32:HEADERS += serial/win_qextserialport.h

SOURCES = bncmain.cpp bncgetthread.cpp  bncwindow.cpp bnctabledlg.cpp \
          bnccaster.cpp bncrinex.cpp bncapp.cpp bncutils.cpp          \
          bncconst.cpp bnchtml.cpp bnchlpdlg.cpp bnctableitem.cpp     \
          bnczerodecoder.cpp bncnetqueryv1.cpp bncnetqueryv2.cpp      \
          bncnetqueryrtp.cpp bncsettings.cpp latencychecker.cpp       \
          bncipport.cpp bncnetqueryv0.cpp bncnetqueryudp.cpp          \
          bncnetqueryudp0.cpp bncudpport.cpp pppopt.cpp               \
          bncserialport.cpp bncnetquerys.cpp bncfigure.cpp            \
          bncfigurelate.cpp bncpppclient.cpp bnctime.cpp              \
          bancroft.cpp bncmodel.cpp bncfigureppp.cpp bncrawfile.cpp   \
          bnctides.cpp bncmap_svg.cpp bncantex.cpp                    \
          bncephuser.cpp bncoutf.cpp bncclockrinex.cpp bncsp3.cpp     \
          bncbytescounter.cpp bncsslconfig.cpp reqcdlg.cpp            \
          upload/bncrtnetdecoder.cpp upload/bncuploadcaster.cpp       \
          upload/bncrtnetuploadcaster.cpp upload/bnccustomtrafo.cpp   \
          upload/bncephuploadcaster.cpp qtfilechooser.cpp             \
          RTCM/GPSDecoder.cpp RTCM/RTCM2.cpp RTCM/RTCM2Decoder.cpp    \
          RTCM/RTCM2_2021.cpp RTCM/rtcm_utils.cpp                     \
          RTCM3/RTCM3Decoder.cpp RTCM3/rtcm3torinex/rtcm3torinex.c    \
          RTCM3/rtcm3torinex/rtcm3torinexsupport.c                    \
          RTCM3/RTCM3coDecoder.cpp                                    \
          RTCM3/clock_and_orbit/clock_orbit_rtcm.c                    \
          RTCM3/ephemeris.cpp RTCM3/timeutils.cpp                     \
          GPSS/gpssDecoder.cpp GPSS/hassDecoder.cpp

SOURCES       += serial/qextserialbase.cpp serial/qextserialport.cpp
unix:SOURCES  += serial/posix_qextserialport.cpp
win32:SOURCES += serial/win_qextserialport.cpp

RC_FILE = bnc.rc

QT += network

exists(combination/bnccomb.h) {
  DEFINES += USE_COMBINATION
  HEADERS += combination/bnccomb.h
  SOURCES += combination/bnccomb.cpp
}

exists(rinex/bncpostprocess.h) {
  DEFINES += USE_POSTPROCESSING
  HEADERS += rinex/bncpostprocess.h   rinex/rnxobsfile.h   \
             rinex/rnxnavfile.h       rinex/corrfile.h     \
             rinex/reqcedit.h         rinex/reqcanalyze.h  \
             rinex/graphwin.h         rinex/polarplot.h    \
             rinex/availplot.h
  SOURCES += rinex/bncpostprocess.cpp rinex/rnxobsfile.cpp \
             rinex/rnxnavfile.cpp     rinex/corrfile.cpp   \
             rinex/reqcedit.cpp       rinex/reqcanalyze.cpp \
             rinex/graphwin.cpp       rinex/polarplot.cpp   \
             rinex/availplot.cpp
}

