TARGET = build/lxmlpack
TEMPLATE = lib
DEFINES += LIB_LIBRARY QT_DEPRECATED_WARNINGS
MOC_DIR = moc
OBJECTS_DIR = obj

QT -= gui
QT += core xml widgets


INCLUDEPATH += .\
            $$PWD/../base \
            $$PWD

#path for libs files
DEPENDPATH += $$PWD/../base/build
#include libs
unix:!macx: LIBS += -L$$PWD/../base/build/ -llbase


# Input
HEADERS += $$PWD/xmlpack.h \
	$$PWD/xmlpack_global.h \
	$$PWD/xmlpackview.h \
	$$PWD/xmlpacktype.h


SOURCES += $$PWD/xmlpack.cpp \
	$$PWD/xmlpackview.cpp \
	$$PWD/xmlpacktype.cpp



