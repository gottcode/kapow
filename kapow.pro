TEMPLATE = app
greaterThan(QT_MAJOR_VERSION, 4) {
	QT += widgets
	QT += printsupport
}
CONFIG += warn_on
macx {
	CONFIG += x86_64
}
QT += network

MOC_DIR = build
OBJECTS_DIR = build
RCC_DIR = build

VERSION = $$system(git rev-parse --short HEAD)
isEmpty(VERSION) {
	VERSION = 0
}
DEFINES += VERSIONSTR=\\\"git.$${VERSION}\\\"

unix: !macx {
	TARGET = kapow
} else {
	TARGET = Kapow
}

HEADERS = src/contact.h \
	src/data_model.h \
	src/date_editor.h \
	src/filter_model.h \
	src/locale_dialog.h \
	src/project.h \
	src/rates.h \
	src/report.h \
	src/session.h \
	src/session_delegate.h \
	src/session_dialog.h \
	src/settings.h \
	src/time_editor.h \
	src/window.h

SOURCES = src/contact.cpp \
	src/data_model.cpp \
	src/date_editor.cpp \
	src/filter_model.cpp \
	src/locale_dialog.cpp \
	src/main.cpp \
	src/project.cpp \
	src/rates.cpp \
	src/report.cpp \
	src/session.cpp \
	src/session_delegate.cpp \
	src/session_dialog.cpp \
	src/settings.cpp \
	src/time_editor.cpp \
	src/window.cpp

TRANSLATIONS = translations/kapow_bg.ts \
	translations/kapow_cs.ts \
	translations/kapow_da.ts \
	translations/kapow_de.ts \
	translations/kapow_el.ts \
	translations/kapow_en.ts \
	translations/kapow_es.ts \
	translations/kapow_fr.ts \
	translations/kapow_it.ts \
	translations/kapow_nl.ts \
	translations/kapow_uk.ts

RESOURCES = icons/icons.qrc
macx {
	ICON = icons/kapow.icns
}
win32 {
	RC_FILE = icons/icon.rc
}

unix: !macx {
	isEmpty(PREFIX) {
		PREFIX = /usr/local
	}

	target.path = $$PREFIX/bin/

	icon.files = icons/hicolor/*
	icon.path = $$PREFIX/share/icons/hicolor/

	pixmap.files = icons/kapow_32.xpm
	pixmap.path = $$PREFIX/share/pixmaps/

	desktop.files = icons/kapow.desktop
	desktop.path = $$PREFIX/share/applications/

	qm.files = translations/*.qm
	qm.path = $$PREFIX/share/kapow/translations/

	INSTALLS += target icon pixmap desktop qm
}
