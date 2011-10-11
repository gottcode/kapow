/***********************************************************************
 *
 * Copyright (C) 2008, 2010, 2011 Graeme Gott <graeme@gottcode.org>
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

#include "locale_dialog.h"
#include "window.h"

#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <QSettings>

int main(int argc, char** argv) {
	QApplication app(argc, argv);
	app.setApplicationName("Kapow");
	app.setApplicationVersion("1.3.3");
	app.setOrganizationDomain("gottcode.org");
	app.setOrganizationName("GottCode");
	QString appdir = app.applicationDirPath();

#if defined(Q_OS_MAC)
	QFileInfo portable(appdir + "/../../../Data");
	QString path = QDir::homePath() + "/Library/Application Support/GottCode/Kapow/";
#elif defined(Q_OS_UNIX)
	QFileInfo portable(appdir + "/Data");
	QString path = getenv("$XDG_DATA_HOME");
	if (path.isEmpty()) {
		path = QDir::homePath() + "/.local/share/";
	}
	path += "/gottcode/kapow/";
#elif defined(Q_OS_WIN32)
	QFileInfo portable(appdir + "/Data");
	QString path = QDir::homePath() + "/Application Data/GottCode/Kapow/";
#else
	QFileInfo portable(appdir + "/Data");
	QString path = QDesktopServices::storageLocation(QDesktopSettings::Data) + "/Kapow/";
#endif

	// Handle portability
	if (portable.exists() && portable.isWritable()) {
		path = portable.absoluteFilePath();
		QSettings::setDefaultFormat(QSettings::IniFormat);
		QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, path + "/Settings");
	}

	// Create base data path
	if (!QFile::exists(path)) {
		QDir dir = QDir::home();
		dir.mkpath(path);
	}

	LocaleDialog::loadTranslator("kapow_");

	Window window(path + "/data.xml");
	return app.exec();
}
