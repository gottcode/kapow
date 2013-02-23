TEMPLATE = app
QT += network testlib
greaterThan(QT_MAJOR_VERSION, 4) {
	QT += widgets printsupport
}
macx {
	CONFIG -= app_bundle
}

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
