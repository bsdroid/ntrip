
TEMPLATE             = lib
CONFIG              += plugin static
TARGET               = $$qtLibraryTarget(GnssCenter_svgMap)
QT                  += svg
INCLUDEPATH         += ../qwt ../main

##unix:LIBS  += -L../../qwt -lqwt
##win32:LIBS += -L../../qwt/release -lqwt

debug:OBJECTS_DIR   = .obj/debug
debug:MOC_DIR       = .moc/debug
release:OBJECTS_DIR = .obj/release
release:MOC_DIR     = .moc/release

HEADERS   = svgmap.h

SOURCES   = svgmap.cpp

RESOURCES = svgmap.qrc
