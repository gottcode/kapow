/***********************************************************************
 *
 * Copyright (C) 2008, 2010 Graeme Gott <graeme@gottcode.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ***********************************************************************/

#include "window.h"

#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>

int main(int argc, char** argv) {
	QApplication app(argc, argv);
	app.setApplicationName("Kapow");
	app.setApplicationVersion("1.3.2");
	app.setOrganizationDomain("gottcode.org");
	app.setOrganizationName("GottCode");

	QString language = QLocale::system().name();

	QTranslator qt_translator;
	qt_translator.load("qt_" + language, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
	app.installTranslator(&qt_translator);

	QTranslator translator_en;
	if (!language.startsWith("en")) {
		translator_en.load(":/kapow_en");
		app.installTranslator(&translator_en);
	}

	QTranslator translator;
	translator.load(":/kapow_" + language);
	app.installTranslator(&translator);

	QDir dir = QDir::home();
#if defined(Q_OS_MAC)
	QString path = QDir::homePath() + "/Library/Application Support/GottCode/Kapow/";
#elif defined(Q_OS_UNIX)
	QString path = getenv("$XDG_DATA_HOME");
	if (path.isEmpty()) {
		path = QDir::homePath() + "/.local/share/";
	}
	path += "/gottcode/kapow/";
#elif defined(Q_OS_WIN32)
	QString path = QDir::homePath() + "/Application Data/GottCode/Kapow/";
#else
	QString path = QDesktopServices::storageLocation(QDesktopSettings::Data) + "/Kapow/";
#endif
	dir.mkpath(path);

	Window window(path + "data.xml");
	return app.exec();
}
