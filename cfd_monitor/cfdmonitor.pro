
TEMPLATE = app
TARGET = cfdmonitor
DEFINES += QT_DEPRECATED_WARNINGS
MOC_DIR=moc
OBJECTS_DIR=obj
UI_DIR=ui
#CONFIG += warn_on
#QMAKE_CXXFLAGS_WARN_OFF += -Wno-deprecated -Wall -Wextra
#QMAKE_CFLAGS_WARN_ON -= -Wno-deprecated -Wmismatched-dealloc -Wmemset-transposed-args -Wpointer-sign -Warray-compare -Wunused-function -Wformat-overflow -Wmemset-elt-size
#QMAKE_CXXFLAGS_WARN_ON -= -Wno-deprecated -Wmismatched-dealloc -Wmemset-transposed-args -Wpointer-sign -Warray-compare -Wunused-function -Wformat-overflow -Wmemset-elt-size

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
	$$PWD/basepage.h \
	$$PWD/cfdpage.h \
	$$PWD/divpage.h \
	$$PWD/htmlpage.h \
	$$PWD/configpage.h \
        $$PWD/chartpage.h \
        $$PWD/cfdconfigobj.h \
	$$PWD/logpage.h


SOURCES += $$PWD/main.cpp \
	$$PWD/mainform.cpp \
	$$PWD/tgbot.cpp \
	$$PWD/cfdcalcobj.cpp \
	$$PWD/basepage.cpp \
	$$PWD/cfdpage.cpp \
	$$PWD/divpage.cpp \
	$$PWD/htmlpage.cpp \
        $$PWD/configpage.cpp \
        $$PWD/chartpage.cpp \
        $$PWD/cfdconfigobj.cpp \
	$$PWD/logpage.cpp


FORMS += $$PWD/ui/configpage.ui \
        $$PWD/ui/htmlpage.ui \
        $$PWD/ui/chartpage.ui \
        $$PWD/ui/logpage.ui

