
# Switch to debug configuration
# -----------------------------
CONFIG -= release
CONFIG += debug

RESOURCES += bns.qrc

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

HEADERS =             bns.h   bnswindow.h   bnshlpdlg.h   bnshtml.h   \
          bnseph.h    bnsutils.h

SOURCES = bnsmain.cpp bns.cpp bnswindow.cpp bnshlpdlg.cpp bnshtml.cpp \
          bnseph.cpp  bnsutils.cpp

RC_FILE = bns.rc

QT += network

