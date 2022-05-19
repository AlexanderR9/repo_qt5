
TEMPLATE = app
TARGET = cfdmonitor
DEFINES += QT_DEPRECATED_WARNINGS
MOC_DIR=moc
OBJECTS_DIR=obj

QT -= gui
QT *= network widgets
QT += webenginewidgets

INCLUDEPATH += . \
            $$PWD \
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
	$$PWD/cfdrequester.h \
	$$PWD/statobj.h \
	$$PWD/basepage.h \
	$$PWD/cfdpage.h \
	$$PWD/logpage.h


SOURCES += $$PWD/main.cpp \
	$$PWD/mainform.cpp \
	$$PWD/tgbot.cpp \
	$$PWD/cfdrequester.cpp \
	$$PWD/statobj.cpp \
	$$PWD/basepage.cpp \
	$$PWD/cfdpage.cpp \
	$$PWD/logpage.cpp


