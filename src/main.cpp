/***********************************************************************
 *
 * Copyright (C) 2008, 2010, 2011, 2012, 2013, 2014, 2015, 2018 Graeme Gott <graeme@gottcode.org>
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
#include "paths.h"
#include "settings.h"
#include "window.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QMessageBox>

int main(int argc, char** argv)
{
#if !defined(Q_OS_MAC)
	if (!qEnvironmentVariableIsSet("QT_DEVICE_PIXEL_RATIO")
			&& !qEnvironmentVariableIsSet("QT_AUTO_SCREEN_SCALE_FACTOR")
			&& !qEnvironmentVariableIsSet("QT_SCALE_FACTOR")
			&& !qEnvironmentVariableIsSet("QT_SCREEN_SCALE_FACTORS")) {
		QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	}
#endif
	QApplication app(argc, argv);
	app.setApplicationName("Kapow");
	app.setApplicationDisplayName(Window::tr("Kapow Punch Clock"));
	app.setApplicationVersion(VERSIONSTR);
	app.setOrganizationDomain("gottcode.org");
	app.setOrganizationName("GottCode");
#if !defined(Q_OS_WIN) && !defined(Q_OS_MAC)
	app.setWindowIcon(QIcon::fromTheme("kapow", QIcon(":/kapow.png")));
	app.setDesktopFileName("kapow");
#endif
	app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);

	QString path;
	bool backups_enabled = true;
	{
		// Handle portability
		QString appdir = app.applicationDirPath();
#if defined(Q_OS_MAC)
		QFileInfo portable(appdir + "/../../../Data");
#else
		QFileInfo portable(appdir + "/Data");
#endif
		if (portable.exists() && portable.isWritable()) {
			path = portable.absoluteFilePath();
			if (QFile::exists(path + "/Settings/GottCode/Kapow.ini") &&
					!QFile::rename(path + "/Settings/GottCode/Kapow.ini", path + "/Settings/Kapow.ini")) {
				Settings::setPath(path + "/Settings/GottCode/Kapow.ini");
			} else {
				Settings::setPath(path + "/Settings/Kapow.ini");
			}
		}

		// Handle command-line options
		QCommandLineParser parser;
		parser.setApplicationDescription(QCoreApplication::translate("main", "Punch clock program"));
		parser.addHelpOption();
		parser.addVersionOption();
		parser.addOption(QCommandLineOption("ini",
			QCoreApplication::translate("main", "Store settings as INI format in specified file."),
			QCoreApplication::translate("main", "file")));
		parser.addOption(QCommandLineOption("no-backups",
			QCoreApplication::translate("main", "Do not create automatic backups of time data.")));
		parser.addPositionalArgument("file", QCoreApplication::translate("main", "The time data file to use."), "[file]");
		parser.process(app);

		if (parser.isSet("ini")) {
			Settings::setPath(parser.value("ini"));
		}

		if (parser.isSet("no-backups")) {
			backups_enabled = false;
		}

		QStringList files = parser.positionalArguments();
		if (!files.isEmpty()) {
			QFileInfo override(files.back());
			if (override.suffix().toLower() == "xml") {
				path = override.absoluteFilePath();
			}
		}
	}

	// Load settings
	Settings settings;
	Q_UNUSED(settings);

	LocaleDialog::loadTranslator("kapow_");

	// Make sure data path exists
	if (path.isEmpty()) {
		path = Paths::dataPath();

		if (!QFile::exists(path)) {
			// Create data location
			QDir dir(path);
			if (!dir.mkpath(dir.absolutePath())) {
				QMessageBox::critical(0, Window::tr("Error"), Window::tr("Unable to create time data location."));
				return 1;
			}

			// Move time data
			QString oldpath = Paths::oldDataPath();
			if (!oldpath.isEmpty()) {
				QDir olddir(oldpath);
				QStringList files = olddir.entryList(QDir::Files);
				bool success = true;
				for (const QString& file : files) {
					success &= QFile::rename(olddir.absoluteFilePath(file), dir.absoluteFilePath(file));
				}
				dir.rmdir(oldpath);
				if (!success) {
					QMessageBox::warning(0, Window::tr("Error"), Window::tr("Unable to move time data location."));
				}
			}
		}
	// Make sure command-line data path exists
	} else if (!QFile::exists(path + "/../")) {
		QDir dir(path + "/../");
		if (!dir.mkpath(dir.absolutePath())) {
			QMessageBox::critical(0, Window::tr("Error"), Window::tr("Unable to create time data location."));
			return 1;
		}
	}

	// Find data file
	if (!path.endsWith(".xml")) {
		if (QFile::exists(path + "/kapow.xml")) {
			path += "/kapow.xml";
		} else if (QFile::exists(path + "/data.xml") && !QFile::rename(path + "/data.xml", path + "/kapow.xml")) {
			path += "/data.xml";
		} else {
			path += "/kapow.xml";
		}
	}

	Window window(path, backups_enabled);
	if (window.isValid()) {
		return app.exec();
	} else {
		return 1;
	}
}
