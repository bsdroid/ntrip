
# Switch to debug configuration
# -----------------------------
###CONFIG -= release
###CONFIG += debug

DEFINES += NO_RTCM3_MAIN

RESOURCES += bnc.qrc

unix:QMAKE_CFLAGS_RELEASE   -= -O2
unix:QMAKE_CXXFLAGS_RELEASE -= -O2

# Get rid of mingwm10.dll
# -----------------------
win32:QMAKE_LFLAGS                 -= -mthreads
win32:QMAKE_CXXFLAGS_EXCEPTIONS_ON -= -mthreads
win32:QMAKE_LFLAGS_EXCEPTIONS_ON   -= -mthreads

debug:OBJECTS_DIR=.obj/debug
debug:MOC_DIR=.moc/debug
release:OBJECTS_DIR=.obj/release
release:MOC_DIR=.moc/release

HEADERS = bnchelp.html bncgetthread.h    bncwindow.h   bnctabledlg.h  \
          bnccaster.h bncrinex.h bncapp.h bncutils.h   bnchlpdlg.h    \
          bncconst.h bnchtml.h bnctableitem.h bnczerodecoder.h        \
          RTCM/GPSDecoder.h RTCM/RTCM2.h RTCM/RTCM2Decoder.h          \
          RTCM3/RTCM3Decoder.h RTCM3/rtcm3torinex.h                   \
          RTIGS/RTIGSDecoder.h RTIGS/rtigs_records.h                  \
          RTIGS/cgps_transform.h RTIGS/rtstruct.h RTIGS/rtacp.h

SOURCES = bncmain.cpp bncgetthread.cpp  bncwindow.cpp bnctabledlg.cpp \
          bnccaster.cpp bncrinex.cpp bncapp.cpp bncutils.cpp          \
          bncconst.cpp bnchtml.cpp bnchlpdlg.cpp bnctableitem.cpp     \
          bnczerodecoder.cpp                                          \
          RTCM/RTCM2.cpp RTCM/RTCM2Decoder.cpp                        \
          RTCM3/RTCM3Decoder.cpp RTCM3/rtcm3torinex.c                 \
          RTIGS/RTIGSDecoder.cpp RTIGS/cgps_transform.cpp

RC_FILE = bnc.rc

QT += network

