
QT -= gui
QT += webenginewidgets


TARGET = build/lweb
TEMPLATE = lib
DEFINES += LIB_LIBRARY
MOC_DIR = moc
OBJECTS_DIR = obj

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += .\
	    $$PWD

# Input
HEADERS += $$PWD/lhtmlrequesterbase.h \
        $$PWD/lhtmlrequester.h \
	$$PWD/lhtmlpagerequester.h \
	$$PWD/web_global.h

SOURCES += $$PWD/lhtmlrequesterbase.cpp \
        $$PWD/lhtmlrequester.cpp \
        $$PWD/lhtmlpagerequester.cpp


