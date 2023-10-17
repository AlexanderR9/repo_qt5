
TEMPLATE = app
TARGET = proc_cleaner
DEFINES += QT_DEPRECATED_WARNINGS
MOC_DIR=moc
OBJECTS_DIR=obj

QT -= gui
#QT *= network widgets xml testlib

INCLUDEPATH += . \
            $$PWD \
            $$PWD/../libs/base \
            $$PWD/../libs/process


#path for libs files
DEPENDPATH += $$PWD/../libs/base/build \
            $$PWD/../libs/process/build

#include libs
unix:!macx: LIBS += -L$$PWD/../libs/base/build/ -llbase
unix:!macx: LIBS += -L$$PWD/../libs/process/build/ -llprocess


# Input
HEADERS += $$PWD/pcleaner.h


SOURCES += $$PWD/main.cpp \
        $$PWD/pcleaner.cpp



