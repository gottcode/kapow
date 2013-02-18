TEMPLATE = app
QT += network testlib
greaterThan(QT_MAJOR_VERSION, 4) {
	QT += widgets printsupport
}
macx {
	CONFIG -= app_bundle
}

DEPENDPATH += ../../src/
INCLUDEPATH += ../../src/

SOURCES += $$join(TARGET,,,.cpp)
HEADERS += $$join(TARGET,,,.h)

test.commands += ./$(QMAKE_TARGET)
QMAKE_EXTRA_TARGETS += test
