
TEMPLATE = app
TARGET = dianaemulator
DEFINES += QT_DEPRECATED_WARNINGS
MOC_DIR=moc
OBJECTS_DIR=obj

QT -= gui
QT *= widgets xml

INCLUDEPATH += . \
            $$PWD \
            $$PWD/../libs/base \
            $$PWD/../libs/base/ui_h \
            $$PWD/../libs/mq \
            $$PWD/../libs/xmlpack


#path for libs files
DEPENDPATH += $$PWD/../libs/base/build
DEPENDPATH += $$PWD/../libs/mq/build
DEPENDPATH += $$PWD/../libs/xmlpack/build


#include libs
unix:!macx: LIBS += -L$$PWD/../libs/base/build/ -llbase
unix:!macx: LIBS += -L$$PWD/../libs/mq/build/ -llmq
unix:!macx: LIBS += -L$$PWD/../libs/xmlpack/build/ -llxmlpack


# Input
HEADERS += $$PWD/mainform.h \
	$$PWD/mqgeneralpage.h \
	$$PWD/dianaobj.h \
	$$PWD/dianaviewwidget.h

SOURCES += $$PWD/main.cpp \
	$$PWD/mqgeneralpage.cpp \
	$$PWD/mainform.cpp \
	$$PWD/dianaobj.cpp \
	$$PWD/dianaviewwidget.cpp



