
# Switch to debug configuration
# -----------------------------
CONFIG -= debug
CONFIG += release


DEFINES += NO_RTCM3_MAIN 
###DEFINES += DEBUG_RTCM2_2021
unix:DEFINES  += _TTY_POSIX_
win32:DEFINES += _TTY_WIN_

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

# Include Path
# ------------
INCLUDEPATH = . ./newmat ./RTCM3 ./RTCM3/clock_and_orbit ./RTCM3/rtcm3torinex

HEADERS = bnchelp.html bncgetthread.h    bncwindow.h   bnctabledlg.h  \
          bnccaster.h bncrinex.h bncapp.h bncutils.h   bnchlpdlg.h    \
          bncconst.h bnchtml.h bnctableitem.h bnczerodecoder.h        \
          bncnetquery.h bncnetqueryv1.h bncnetqueryv2.h               \
          bncnetqueryrtp.h bncsettings.h latencychecker.h             \
          bncipport.h bncnetqueryv0.h bncnetqueryudp.h                \ 
          bncnetqueryudp0.h bncudpport.h bnctime.h                    \ 
          bncserialport.h bncnetquerys.h bncfigure.h                  \ 
          bncfigurelate.h bncpppclient.h bncversion.h                 \ 
          bancroft.h bncmodel.h bncfigureppp.h bncrawfile.h           \ 
          RTCM/GPSDecoder.h RTCM/RTCM2.h RTCM/RTCM2Decoder.h          \
          RTCM/RTCM2_2021.h RTCM/rtcm_utils.h                         \
          RTCM3/RTCM3Decoder.h RTCM3/rtcm3torinex/rtcm3torinex.h      \
          RTCM3/RTCM3coDecoder.h                                      \
          RTCM3/clock_and_orbit/clock_orbit_rtcm.h                    \
          RTCM3/ephemeris.h RTCM3/timeutils.h                         \
          RTIGS/RTIGSDecoder.h RTIGS/rtigs_records.h                  \
          RTIGS/cgps_transform.h RTIGS/rtstruct.h                     \
          RTIGS/rtacp.h RTIGS/gpswro.h                                \
          GPSS/gpssDecoder.h

HEADERS       += serial/qextserialbase.h serial/qextserialport.h
unix:HEADERS  += serial/posix_qextserialport.h
win32:HEADERS += serial/win_qextserialport.h

HEADERS += newmat/controlw.h newmat/include.h newmat/myexcept.h  \
           newmat/newmatap.h newmat/newmat.h newmat/newmatio.h   \
           newmat/newmatrc.h newmat/newmatrm.h newmat/precisio.h

SOURCES = bncmain.cpp bncgetthread.cpp  bncwindow.cpp bnctabledlg.cpp \
          bnccaster.cpp bncrinex.cpp bncapp.cpp bncutils.cpp          \
          bncconst.cpp bnchtml.cpp bnchlpdlg.cpp bnctableitem.cpp     \
          bnczerodecoder.cpp bncnetqueryv1.cpp bncnetqueryv2.cpp      \
          bncnetqueryrtp.cpp bncsettings.cpp latencychecker.cpp       \
          bncipport.cpp bncnetqueryv0.cpp bncnetqueryudp.cpp          \
          bncnetqueryudp0.cpp bncudpport.cpp                          \
          bncserialport.cpp bncnetquerys.cpp bncfigure.cpp            \
          bncfigurelate.cpp bncpppclient.cpp bnctime.cpp              \
          bancroft.cpp bncmodel.cpp bncfigureppp.cpp bncrawfile.cpp   \
          RTCM/RTCM2.cpp RTCM/RTCM2Decoder.cpp                        \
          RTCM/RTCM2_2021.cpp RTCM/rtcm_utils.cpp                     \
          RTCM3/RTCM3Decoder.cpp RTCM3/rtcm3torinex/rtcm3torinex.c    \
          RTCM3/RTCM3coDecoder.cpp                                    \
          RTCM3/clock_and_orbit/clock_orbit_rtcm.c                    \
          RTCM3/ephemeris.cpp RTCM3/timeutils.cpp                     \
          RTIGS/RTIGSDecoder.cpp RTIGS/cgps_transform.cpp             \
          GPSS/gpssDecoder.cpp

SOURCES       += serial/qextserialbase.cpp serial/qextserialport.cpp
unix:SOURCES  += serial/posix_qextserialport.cpp
win32:SOURCES += serial/win_qextserialport.cpp

SOURCES += newmat/bandmat.cpp newmat/cholesky.cpp newmat/evalue.cpp  \
           newmat/fft.cpp newmat/hholder.cpp newmat/jacobi.cpp       \
           newmat/myexcept.cpp newmat/newfft.cpp newmat/newmat1.cpp  \
           newmat/newmat2.cpp newmat/newmat3.cpp newmat/newmat4.cpp  \
           newmat/newmat5.cpp newmat/newmat6.cpp newmat/newmat7.cpp  \
           newmat/newmat8.cpp newmat/newmat9.cpp newmat/newmatex.cpp \
           newmat/newmatrm.cpp newmat/nm_misc.cpp newmat/sort.cpp    \
           newmat/submat.cpp newmat/svd.cpp

RC_FILE = bnc.rc

QT += network

