
TEMPLATE     = app
TARGET       = ../GnssCenter
QTPLUGIN    += gnsscenter_inpedit gnsscenter_svgmap
QT          += svg
CONFIG      += debug

debug:OBJECTS_DIR   = .obj/debug
debug:MOC_DIR       = .moc/debug
release:OBJECTS_DIR = .obj/release
release:MOC_DIR     = .moc/release

INCLUDEPATH += ../qwt

LIBS  += -L../inpedit -lgnsscenter_inpedit
LIBS  += -L../svgmap  -lgnsscenter_svgmap
LIBS  += -L../qwt     -lqwt

HEADERS +=              app.h       mdiarea.h   \
           settings.h   mainwin.h   plugininterface.h

SOURCES += main.cpp     app.cpp     mdiarea.cpp \
           settings.cpp mainwin.cpp

