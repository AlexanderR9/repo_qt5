
TEMPLATE = app
TARGET = unigraph
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
            $$PWD/../../libs/web \
            $$PWD/../../libs/uniswap \

#path for libs files
DEPENDPATH += $$PWD/../../libs/base/build \
            $$PWD/../../libs/web/build \
            $$PWD/../../libs/uniswap/build \

#include libs
unix:!macx: LIBS += -L$$PWD/../../libs/base/build/ -llbase
unix:!macx: LIBS += -L$$PWD/../../libs/web/build/ -llweb
unix:!macx: LIBS += -L$$PWD/../../libs/uniswap/build/ -lluniswap


# Input
HEADERS += $$PWD/mainform.h \
        $$PWD/ug_centralwidget.h \
        $$PWD/ug_basepage.h \
        $$PWD/ug_poolpage.h \
        $$PWD/ug_jsonviewpage.h \
        $$PWD/ug_apistruct.h \
    	$$PWD/subcommonsettings.h \
        $$PWD/ug_tokenpage.h \
        $$PWD/ug_daysdatapage.h


SOURCES += $$PWD/main.cpp \
        $$PWD/mainform.cpp \
        $$PWD/ug_centralwidget.cpp \
        $$PWD/ug_basepage.cpp \
        $$PWD/ug_poolpage.cpp \
        $$PWD/ug_jsonviewpage.cpp \
        $$PWD/ug_apistruct.cpp \
    	$$PWD/subcommonsettings.cpp \
        $$PWD/ug_tokenpage.cpp \
        $$PWD/ug_daysdatapage.cpp

