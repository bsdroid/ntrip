
# Switch to debug configuration
# -----------------------------
CONFIG += release
#CONFIG += debug

RESOURCES += bns.qrc

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
INCLUDEPATH = . ./newmat

HEADERS =             bns.h   bnswindow.h   bnshlpdlg.h   bnshtml.h   \
          bnseph.h    bnsutils.h bnsrinex.h bnssp3.h bnsoutf.h        \
          bnscaster.h RTCM/clock_orbit_rtcm.h

HEADERS += newmat/controlw.h newmat/include.h newmat/myexcept.h  \
           newmat/newmatap.h newmat/newmat.h newmat/newmatio.h   \
           newmat/newmatrc.h newmat/newmatrm.h newmat/precisio.h

SOURCES = bnsmain.cpp bns.cpp bnswindow.cpp bnshlpdlg.cpp bnshtml.cpp  \
          bnseph.cpp  bnsutils.cpp bnsrinex.cpp bnssp3.cpp bnsoutf.cpp \
          bnscaster.cpp RTCM/clock_orbit_rtcm.c

SOURCES += newmat/bandmat.cpp newmat/cholesky.cpp newmat/evalue.cpp  \
           newmat/fft.cpp newmat/hholder.cpp newmat/jacobi.cpp       \
           newmat/myexcept.cpp newmat/newfft.cpp newmat/newmat1.cpp  \
           newmat/newmat2.cpp newmat/newmat3.cpp newmat/newmat4.cpp  \
           newmat/newmat5.cpp newmat/newmat6.cpp newmat/newmat7.cpp  \
           newmat/newmat8.cpp newmat/newmat9.cpp newmat/newmatex.cpp \
           newmat/newmatrm.cpp newmat/nm_misc.cpp newmat/sort.cpp    \
           newmat/submat.cpp newmat/svd.cpp

RC_FILE = bns.rc

QT += network

