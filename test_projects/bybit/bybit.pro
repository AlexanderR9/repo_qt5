
TEMPLATE = app
TARGET = bybit
DEFINES += QT_DEPRECATED_WARNINGS
MOC_DIR=moc
OBJECTS_DIR=obj
UI_DIR=ui

QT -= gui
QT *= network widgets xml testlib

INCLUDEPATH += . \
            $$PWD \
            $$PWD/../../libs/base \
            $$PWD/../../libs/base/ui_h \


#path for libs files
DEPENDPATH += $$PWD/../../libs/base/build \

#include libs
unix:!macx: LIBS += -L$$PWD/../../libs/base/build/ -llbase


# Input
HEADERS += $$PWD/mainform.h \
        $$PWD/bb_centralwidget.h \
    bb_chartpage.h \
    apiconfig.h


SOURCES += $$PWD/main.cpp \
        $$PWD/mainform.cpp \
        $$PWD/bb_centralwidget.cpp \
    bb_chartpage.cpp \
    apiconfig.cpp


