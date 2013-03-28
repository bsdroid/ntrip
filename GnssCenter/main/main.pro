
TEMPLATE     = app
TARGET       = ../GnssCenter

QT          += svg
CONFIG      += debug

debug:OBJECTS_DIR   = .obj/debug
debug:MOC_DIR       = .moc/debug
release:OBJECTS_DIR = .obj/release
release:MOC_DIR     = .moc/release

INCLUDEPATH += ../qwt

unix:LIBS  += -L../qwt -lqwt
win32:LIBS += -L../qwt/release -lqwt

HEADERS +=                app.h       mdiarea.h   \
           settings.h     mainwin.h   plugininterface.h

SOURCES += GnssCenter.cpp app.cpp     mdiarea.cpp \
           settings.cpp   mainwin.cpp

exists(map) {
  INCLUDEPATH += map
  HEADERS     += map/svgmap.h
  SOURCES     += map/svgmap.cpp
  RESOURCES   += map/svgmap.qrc
}

exists(inpedit) {
  INCLUDEPATH += inpedit

  HEADERS += inpedit/keyword.h   inpedit/panel.h      inpedit/inpedit.h   \
             inpedit/selwin.h    inpedit/lineedit.h   inpedit/uniline.h

  SOURCES += inpedit/keyword.cpp inpedit/panel.cpp    inpedit/inpedit.cpp \
             inpedit/selwin.cpp  inpedit/lineedit.cpp inpedit/uniline.cpp
}
