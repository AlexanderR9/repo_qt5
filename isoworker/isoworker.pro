
TEMPLATE = app
TARGET = isoworker
DEFINES += QT_DEPRECATED_WARNINGS
MOC_DIR=moc
OBJECTS_DIR=obj
UI_DIR=ui

QT -= gui
QT *= network widgets xml

INCLUDEPATH += . \
            $$PWD \
            $$PWD/ui \
            $$PWD/../libs/base \
            $$PWD/../libs/base/ui_h \
            $$PWD/../libs/process


#path for libs files
DEPENDPATH += $$PWD/../libs/base/build \
            $$PWD/../libs/process/build


#include libs
unix:!macx: LIBS += -L$$PWD/../libs/base/build/ -llbase
unix:!macx: LIBS += -L$$PWD/../libs/process/build/ -llprocess


# Input
HEADERS += $$PWD/mainform.h \
        $$PWD/foldersstructdialog.h \
        $$PWD/paramspage.h



SOURCES += $$PWD/main.cpp \
        $$PWD/mainform.cpp \
        $$PWD/foldersstructdialog.cpp \
        $$PWD/paramspage.cpp


FORMS += $$PWD/ui/paramspage.ui \
        $$PWD/ui/foldersstructdialog.ui

