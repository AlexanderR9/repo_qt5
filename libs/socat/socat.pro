TARGET = build/lsocat
TEMPLATE = lib
DEFINES += LIB_LIBRARY QT_DEPRECATED_WARNINGS
MOC_DIR = moc
OBJECTS_DIR = obj

QT -= gui
QT += core


INCLUDEPATH += .\
            $$PWD/../base \
            $$PWD/../process \
            $$PWD

#path for libs files
DEPENDPATH += $$PWD/../base/build
DEPENDPATH += $$PWD/../process/build

#include libs
unix:!macx: LIBS += -L$$PWD/../base/build/ -llbase
unix:!macx: LIBS += -L$$PWD/../process/build/ -llprocess


# Input
HEADERS += $$PWD/socatobj.h \
	$$PWD/socat_global.h


SOURCES += $$PWD/socatobj.cpp
