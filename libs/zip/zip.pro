TARGET = build/lzip
TEMPLATE = lib
DEFINES += LIB_LIBRARY QT_DEPRECATED_WARNINGS
MOC_DIR = moc
OBJECTS_DIR = obj

QT -= gui
QT *= core


INCLUDEPATH += .\
            $$PWD/../base \
            $$PWD

#path for libs files
DEPENDPATH += $$PWD/../base/build

#include libs
unix:!macx: LIBS += -L$$PWD/../base/build/ -llbase


# Input
HEADERS += $$PWD/zipobj.h \
	$$PWD/zip_global.h


SOURCES += $$PWD/zipobj.cpp
