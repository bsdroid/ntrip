
# Switch to debug configuration
# -----------------------------
### CONFIG -= release
### CONFIG += debug

RESOURCES += bns.qrc

# Location of the NewMat Library
# ------------------------------
unix:NEWMAT_ROOT = $$(NEWMAT_ROOT)
win32:NEWMAT_ROOT = ../../Source/newmat

# Get rid of mingwm10.dll
# -----------------------
win32:QMAKE_LFLAGS                 -= -mthreads
win32:QMAKE_CXXFLAGS_EXCEPTIONS_ON -= -mthreads
win32:QMAKE_LFLAGS_EXCEPTIONS_ON   -= -mthreads

debug:OBJECTS_DIR=.obj/debug
debug:MOC_DIR=.moc/debug
release:OBJECTS_DIR=.obj/release
release:MOC_DIR=.moc/release

# Include Path and additional Libraries
# -------------------------------------
INCLUDEPATH = . $$NEWMAT_ROOT/include
LIBS        = -L$$NEWMAT_ROOT/lib -lnewmat

HEADERS =             bns.h   bnswindow.h   bnshlpdlg.h   bnshtml.h   \
          bnseph.h    bnsutils.h

SOURCES = bnsmain.cpp bns.cpp bnswindow.cpp bnshlpdlg.cpp bnshtml.cpp \
          bnseph.cpp  bnsutils.cpp

RC_FILE = bns.rc

QT += network

