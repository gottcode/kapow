/***********************************************************************
 *
 * Copyright (C) 2008, 2010, 2011, 2012 Graeme Gott <graeme@gottcode.org>
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
#include <QMessageBox>
#include <QSettings>

int main(int argc, char** argv) {
	QApplication app(argc, argv);
	app.setApplicationName("Kapow");
	app.setApplicationVersion("1.3.4");
	app.setOrganizationDomain("gottcode.org");
	app.setOrganizationName("GottCode");
	{
		QIcon fallback(":/hicolor/256x256/apps/kapow.png");
		fallback.addFile(":/hicolor/128x128/apps/kapow.png");
		fallback.addFile(":/hicolor/64x64/apps/kapow.png");
		fallback.addFile(":/hicolor/48x48/apps/kapow.png");
		fallback.addFile(":/hicolor/32x32/apps/kapow.png");
		fallback.addFile(":/hicolor/24x24/apps/kapow.png");
		fallback.addFile(":/hicolor/22x22/apps/kapow.png");
		fallback.addFile(":/hicolor/16x16/apps/kapow.png");
		app.setWindowIcon(QIcon::fromTheme("kapow", fallback));
	}
	QString appdir = app.applicationDirPath();

	LocaleDialog::loadTranslator("kapow_");

	// Handle portability
	QString path;
	{
#if defined(Q_OS_MAC)
		QFileInfo portable(appdir + "/../../../Data");
#else
		QFileInfo portable(appdir + "/Data");
#endif
		if (portable.exists() && portable.isWritable()) {
			path = portable.absoluteFilePath();
			QSettings::setDefaultFormat(QSettings::IniFormat);
			QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, path + "/Settings");
		}
	}

	// Check for command-line data location
	{
		QFileInfo override(app.arguments().back());
		if (override.exists() && override.isDir() && override.isWritable()) {
			path = override.absoluteFilePath();
		}
	}

	// Make sure data location exists
	if (path.isEmpty()) {
		path = QDesktopServices::storageLocation(QDesktopServices::DataLocation);

		// Update data location
		if (!QFile::exists(path)) {
#if defined(Q_OS_MAC)
			QString oldpath = QDir::homePath() + "/Library/Application Support/GottCode/Kapow/";
#elif defined(Q_OS_UNIX)
			QString oldpath = getenv("$XDG_DATA_HOME");
			if (oldpath.isEmpty()) {
				oldpath = QDir::homePath() + "/.local/share/";
			}
			oldpath += "/gottcode/kapow/";
#elif defined(Q_OS_WIN32)
			QString oldpath = QDir::homePath() + "/Application Data/GottCode/Kapow/";
#endif

			// Create data location if old data location doesn't exist
			if (!QFile::exists(oldpath)) {
				QDir dir(path);
				if (!dir.mkpath(dir.absolutePath())) {
					QMessageBox::critical(0, Window::tr("Error"), Window::tr("Unable to create time data location."));
					return 1;
				}
			// Otherwise, move old data location
			} else if (!QFile::rename(oldpath, path)) {
				QMessageBox::critical(0, Window::tr("Error"), Window::tr("Unable to move time data location."));
				return 1;
			}
		}
	}

	Window window(path + "/data.xml");
	if (window.isValid()) {
		return app.exec();
	} else {
		return 1;
	}
}
