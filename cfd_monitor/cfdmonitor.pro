
TEMPLATE = app
TARGET = cfdmonitor
DEFINES += QT_DEPRECATED_WARNINGS
MOC_DIR=moc
OBJECTS_DIR=obj
UI_DIR=ui

QT -= gui
QT *= network widgets xml
QT += webenginewidgets

INCLUDEPATH += . \
            $$PWD \
            $$PWD/ui \
            $$PWD/../libs/base \
            $$PWD/../libs/base/ui_h \
            $$PWD/../libs/web \
            $$PWD/../libs/tg


#path for libs files
DEPENDPATH += $$PWD/../libs/base/build \
            $$PWD/../libs/web/build \
            $$PWD/../libs/tg/build

#include libs
unix:!macx: LIBS += -L$$PWD/../libs/base/build/ -llbase
unix:!macx: LIBS += -L$$PWD/../libs/web/build/ -llweb
unix:!macx: LIBS += -L$$PWD/../libs/tg/build/ -lltg


# Input
HEADERS += $$PWD/mainform.h \
	$$PWD/tgbot.h \
	$$PWD/cfdcalcobj.h \
	$$PWD/statobj.h \
	$$PWD/basepage.h \
	$$PWD/cfdpage.h \
	$$PWD/htmlpage.h \
	$$PWD/configpage.h \
	$$PWD/cfdconfigobj.h \
	$$PWD/logpage.h


SOURCES += $$PWD/main.cpp \
	$$PWD/mainform.cpp \
	$$PWD/tgbot.cpp \
	$$PWD/cfdcalcobj.cpp \
	$$PWD/statobj.cpp \
	$$PWD/basepage.cpp \
	$$PWD/cfdpage.cpp \
	$$PWD/htmlpage.cpp \
	$$PWD/configpage.cpp \
	$$PWD/cfdconfigobj.cpp \
	$$PWD/logpage.cpp


FORMS += $$PWD/ui/configpage.ui \
        $$PWD/ui/htmlpage.ui

