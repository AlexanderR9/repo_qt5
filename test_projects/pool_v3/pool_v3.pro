
TEMPLATE = app
TARGET = poolsv3
DEFINES += QT_DEPRECATED_WARNINGS
MOC_DIR=moc
OBJECTS_DIR=obj
UI_DIR=ui

QT -= gui
QT *= network widgets xml
QT += webenginewidgets


INCLUDEPATH += . \
            $$PWD \
            $$PWD/tab \
            $$PWD/../../libs/base \
            $$PWD/../../libs/base/ui_h \
            $$PWD/../../libs/web \
            $$PWD/../../libs/process \
            $$PWD/../../libs/uniswap \

#path for libs files
DEPENDPATH += $$PWD/../../libs/base/build \
            $$PWD/../../libs/web/build \
            $$PWD/../../libs/process/build \
            $$PWD/../../libs/uniswap/build \

#include libs
unix:!macx: LIBS += -L$$PWD/../../libs/base/build/ -llbase
unix:!macx: LIBS += -L$$PWD/../../libs/web/build/ -llweb
unix:!macx: LIBS += -L$$PWD/../../libs/process/build/ -llprocess
unix:!macx: LIBS += -L$$PWD/../../libs/uniswap/build/ -lluniswap


# Input
HEADERS += $$PWD/mainform.h \
        $$PWD/centralwidget_v3.h \
        $$PWD/deficonfig.h \
        $$PWD/txlogger.h \
        $$PWD/txdialog.h \
        $$PWD/tab/basetab_v3.h \
        $$PWD/deficonfigloader.h \
	$$PWD/appcommonsettings.h \
	$$PWD/tab/basetabpage_v3.h \
        $$PWD/tab/wallettabpage.h \
        $$PWD/tab/approvepage.h \
        $$PWD/tab/txpage.h \
        $$PWD/tab/poolspage.h \
        $$PWD/nodejsbridge.h \
        $$PWD/txlogrecord.h \
        $$PWD/tokenpricelogger.h


SOURCES += $$PWD/main.cpp \
        $$PWD/mainform.cpp \
        $$PWD/centralwidget_v3.cpp \
        $$PWD/deficonfig.cpp \
        $$PWD/txlogger.cpp \
        $$PWD/txdialog.cpp \
        $$PWD/tab/basetab_v3.cpp \
        $$PWD/deficonfigloader.cpp \
	$$PWD/appcommonsettings.cpp \
	$$PWD/tab/basetabpage_v3.cpp \
	$$PWD/tab/wallettabpage.cpp \
        $$PWD/tab/approvepage.cpp \
        $$PWD/tab/txpage.cpp \
        $$PWD/tab/poolspage.cpp \
        $$PWD/nodejsbridge.cpp \
        $$PWD/txlogrecord.cpp \
        $$PWD/tokenpricelogger.cpp


