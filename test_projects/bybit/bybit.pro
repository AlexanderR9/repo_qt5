
TEMPLATE = app
TARGET = bybit
DEFINES += QT_DEPRECATED_WARNINGS
MOC_DIR=moc
OBJECTS_DIR=obj
UI_DIR=ui

QT -= gui
QT *= network widgets xml
QT += webenginewidgets


INCLUDEPATH += . \
            $$PWD \
            $$PWD/../../libs/base \
            $$PWD/../../libs/base/ui_h \
            $$PWD/../../libs/web

#path for libs files
DEPENDPATH += $$PWD/../../libs/base/build \
            $$PWD/../../libs/web/build

#include libs
unix:!macx: LIBS += -L$$PWD/../../libs/base/build/ -llbase
unix:!macx: LIBS += -L$$PWD/../../libs/web/build/ -llweb


# Input
HEADERS += $$PWD/mainform.h \
        $$PWD/bb_centralwidget.h \
        $$PWD/bb_chartpage.h \
        $$PWD/jsonviewpage.h \
        $$PWD/apiconfig.h \
        $$PWD/bb_apistruct.h


SOURCES += $$PWD/main.cpp \
        $$PWD/mainform.cpp \
        $$PWD/bb_centralwidget.cpp \
        $$PWD/bb_chartpage.cpp \
        $$PWD/jsonviewpage.cpp \
        $$PWD/apiconfig.cpp \
        $$PWD/bb_apistruct.cpp


