TEMPLATE = app
QT += network printsupport testlib widgets
macx {
	CONFIG -= app_bundle
}
CONFIG += c++17

MOC_DIR = ../build
OBJECTS_DIR = ../build
RCC_DIR = ../build

INCLUDEPATH += ../../src/

HEADERS += $$join(TARGET,,,.h)
HEADERS += ../../src/filter_model.h \
	../../src/session.h \
	../../src/session_model.h

SOURCES += $$join(TARGET,,,.cpp)
SOURCES += ../../src/filter_model.cpp \
	../../src/session.cpp \
	../../src/session_model.cpp

test.commands += ./$(QMAKE_TARGET)
QMAKE_EXTRA_TARGETS += test
