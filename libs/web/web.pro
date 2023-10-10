TARGET = build/lweb
TEMPLATE = lib
DEFINES += LIB_LIBRARY QT_DEPRECATED_WARNINGS
MOC_DIR = moc
OBJECTS_DIR = obj
INCLUDEPATH += .\
            $$PWD/../base \
            $$PWD

QT -= gui
QT += webenginewidgets

#path for libs files (include libs)
DEPENDPATH += $$PWD/../base/build
unix:!macx: LIBS += -L$$PWD/../base/build/ -llbase

# Input
HEADERS += $$PWD/lhtmlrequesterbase.h \
        $$PWD/lhtmlrequester.h \
	$$PWD/lhtmlpagerequester.h \
	$$PWD/lhttpapirequester.h \
	$$PWD/lhttp_types.h \
	$$PWD/web_global.h

SOURCES += $$PWD/lhtmlrequesterbase.cpp \
        $$PWD/lhtmlrequester.cpp \
	$$PWD/lhttpapirequester.cpp \
        $$PWD/lhtmlpagerequester.cpp


