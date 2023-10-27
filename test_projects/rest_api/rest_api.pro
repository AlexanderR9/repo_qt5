
TEMPLATE = app
TARGET = rest_api_test
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
	$$PWD/apipages.h \
	$$PWD/instrument.h \
        $$PWD/apicommonsettings.h \
        $$PWD/bagstate.h \
        $$PWD/apireqpage.h \
        $$PWD/reqpreparer.h \
    cycleworker.h

SOURCES += $$PWD/main.cpp \
	$$PWD/apipages.cpp \
	$$PWD/mainform.cpp \
        $$PWD/apicommonsettings.cpp \
        $$PWD/instrument.cpp \
        $$PWD/bagstate.cpp \
        $$PWD/apireqpage.cpp \
        $$PWD/reqpreparer.cpp \
    cycleworker.cpp

