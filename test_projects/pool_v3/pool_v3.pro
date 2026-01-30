
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

# include pri
include(tab/tab.pri)


# Input
HEADERS += $$PWD/mainform.h \
        $$PWD/centralwidget_v3.h \
        $$PWD/deficonfig.h \
        $$PWD/txlogger.h \
        $$PWD/txdialog.h \
        $$PWD/deficonfigloader.h \
	$$PWD/appcommonsettings.h \
        $$PWD/nodejsbridge.h \
        $$PWD/txlogrecord.h \
        $$PWD/tokenpricelogger.h \
        $$PWD/walletbalancelogger.h \
        $$PWD/defiposition.h \
        $$PWD/postxworker.h \
        $$PWD/defimintdialog.h \
        $$PWD/mintrangeview.h \
    strategydata.h


SOURCES += $$PWD/main.cpp \
        $$PWD/mainform.cpp \
        $$PWD/centralwidget_v3.cpp \
        $$PWD/deficonfig.cpp \
        $$PWD/txlogger.cpp \
        $$PWD/txdialog.cpp \
        $$PWD/deficonfigloader.cpp \
	$$PWD/appcommonsettings.cpp \
        $$PWD/nodejsbridge.cpp \
        $$PWD/txlogrecord.cpp \
        $$PWD/tokenpricelogger.cpp \
        $$PWD/walletbalancelogger.cpp \
        $$PWD/defiposition.cpp \
        $$PWD/postxworker.cpp \
        $$PWD/defimintdialog.cpp \
        $$PWD/mintrangeview.cpp \
    strategydata.cpp


FORMS += $$PWD/defimintdialog.ui

