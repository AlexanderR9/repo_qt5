TARGET = build/ltg
TEMPLATE = lib
DEFINES += LIB_LIBRARY QT_DEPRECATED_WARNINGS
MOC_DIR = moc
OBJECTS_DIR = obj

QT += network gui xml

INCLUDEPATH += .\
            $$PWD/../base \
            $$PWD

#path for libs files
DEPENDPATH += $$PWD/../base/build
#include libs
unix:!macx: LIBS += -L$$PWD/../base/build/ -llbase

# Input
HEADERS += $$PWD/tgconfigloaderbase.h \
        $$PWD/tgabstractbot.h \
        $$PWD/tgsender.h

SOURCES += $$PWD/tgconfigloaderbase.cpp \
        $$PWD/tgabstractbot.cpp \
        $$PWD/tgsender.cpp


