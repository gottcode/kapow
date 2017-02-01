lessThan(QT_VERSION, 5.2) {
	error("Kapow requires Qt 5.2 or greater")
}

TEMPLATE = app
QT += network widgets printsupport
CONFIG += warn_on c++11

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x050800

# Allow in-tree builds
!win32 {
	MOC_DIR = build
	OBJECTS_DIR = build
	RCC_DIR = build
}

# Set program version
VERSION = 1.5.2
DEFINES += VERSIONSTR=\\\"$${VERSION}\\\"

# Set program name
unix: !macx {
	TARGET = kapow
} else {
	TARGET = Kapow
}

# Specify program sources
HEADERS = src/contact.h \
	src/date_editor.h \
	src/filter_model.h \
	src/locale_dialog.h \
	src/paths.h \
	src/project.h \
	src/project_delegate.h \
	src/rates.h \
	src/report.h \
	src/session.h \
	src/session_delegate.h \
	src/session_dialog.h \
	src/session_model.h \
	src/settings.h \
	src/time_editor.h \
	src/window.h

SOURCES = src/contact.cpp \
	src/date_editor.cpp \
	src/filter_model.cpp \
	src/locale_dialog.cpp \
	src/main.cpp \
	src/paths.cpp \
	src/project.cpp \
	src/project_delegate.cpp \
	src/rates.cpp \
	src/report.cpp \
	src/session.cpp \
	src/session_delegate.cpp \
	src/session_dialog.cpp \
	src/session_model.cpp \
	src/settings.cpp \
	src/time_editor.cpp \
	src/window.cpp

# Generate translations
TRANSLATIONS = $$files(translations/kapow_*.ts)
TS_FILES = $$TRANSLATIONS
TS_FILES += translations/qtbase_nl.ts
qtPrepareTool(LRELEASE, lrelease)
updateqm.input = TS_FILES
updateqm.output = ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.qm
updateqm.commands = $$LRELEASE -silent ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_OUT}
updateqm.CONFIG += no_link target_predeps
QMAKE_EXTRA_COMPILERS += updateqm

# Install program data
RESOURCES = icons/icons.qrc

macx {
	ICON = icons/kapow.icns
} else:win32 {
	RC_FILE = icons/icon.rc
} else:unix {
	isEmpty(PREFIX) {
		PREFIX = /usr/local
	}

	target.path = $$PREFIX/bin/

	icon.files = icons/hicolor/*
	icon.path = $$PREFIX/share/icons/hicolor/

	pixmap.files = icons/kapow.xpm
	pixmap.path = $$PREFIX/share/pixmaps/

	desktop.files = icons/kapow.desktop
	desktop.path = $$PREFIX/share/applications/

	appdata.files = icons/kapow.appdata.xml
	appdata.path = $$PREFIX/share/metainfo/

	qm.files = translations/*.qm
	qm.path = $$PREFIX/share/kapow/translations/
	qm.CONFIG += no_check_exist

	man.files = doc/kapow.1
	man.path = $$PREFIX/share/man/man1

	INSTALLS += target icon pixmap desktop appdata qm man
}
