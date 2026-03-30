TARGET = build/lzip
TEMPLATE = lib
DEFINES += LIB_LIBRARY QT_DEPRECATED_WARNINGS
MOC_DIR = moc
OBJECTS_DIR = obj

QT -= gui
QT *= core

INCLUDEPATH += .\
            $$PWD \
            $$PWD/../base \
            $$PWD/../process \

#path for libs files
DEPENDPATH += $$PWD/../base/build
DEPENDPATH += $$PWD/../process/build

#include libs
unix:!macx: LIBS += -L$$PWD/../base/build/ -llbase
unix:!macx: LIBS += -L$$PWD/../process/build/ -llprocess



# Input
HEADERS += $$PWD/zipobj.h \
	$$PWD/zip_global.h \
        $$PWD/unzipobj.h \
        $$PWD/zipobj_base.h


SOURCES += $$PWD/zipobj.cpp \
        $$PWD/unzipobj.cpp \
        $$PWD/zipobj_base.cpp


