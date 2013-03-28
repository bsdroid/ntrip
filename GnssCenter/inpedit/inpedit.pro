
TEMPLATE             = lib
CONFIG              += plugin static debug
TARGET               = $$qtLibraryTarget(gnsscenter_inpedit)
INCLUDEPATH         += ../main

debug:OBJECTS_DIR   = .obj/debug
debug:MOC_DIR       = .moc/debug
release:OBJECTS_DIR = .obj/release
release:MOC_DIR     = .moc/release

HEADERS = keyword.h   panel.h      inpedit.h   \
          selwin.h    lineedit.h   uniline.h

SOURCES = keyword.cpp panel.cpp    inpedit.cpp \
          selwin.cpp  lineedit.cpp uniline.cpp
