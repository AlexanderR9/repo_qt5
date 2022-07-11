TARGET = build/lprocess
TEMPLATE = lib
DEFINES += LIB_LIBRARY QT_DEPRECATED_WARNINGS
MOC_DIR = moc
OBJECTS_DIR = obj

QT -= gui
QT += core


INCLUDEPATH += .\
            $$PWD/../base \
            $$PWD

#path for libs files
DEPENDPATH += $$PWD/../base/build


#include libs
unix:!macx: LIBS += -L$$PWD/../base/build/ -llbase


# Input
HEADERS += $$PWD/processobj.h \
	$$PWD/process_global.h


SOURCES += $$PWD/processobj.cpp



