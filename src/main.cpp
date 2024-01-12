/*
	SPDX-FileCopyrightText: 2008-2022 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "locale_dialog.h"
#include "paths.h"
#include "settings.h"
#include "window.h"

#include <QCommandLineParser>
#include <QDir>
#include <QMessageBox>
#include <QtSingleApplication>

int main(int argc, char** argv)
{
	QtSingleApplication app(argc, argv);
	app.setApplicationName("Kapow");
	app.setApplicationDisplayName(Window::tr("Kapow Punch Clock"));
	app.setApplicationVersion(VERSIONSTR);
	app.setOrganizationDomain("gottcode.org");
	app.setOrganizationName("GottCode");
#if !defined(Q_OS_WIN) && !defined(Q_OS_MAC)
	app.setWindowIcon(QIcon::fromTheme("kapow", QIcon(":/kapow.png")));
	app.setDesktopFileName("kapow");
#endif

	if (app.isRunning()) {
		return !app.sendMessage("show");
	}

	const QString appdir = app.applicationDirPath();
	const QString datadir = QDir::cleanPath(appdir + "/" + KAPOW_DATADIR);

	QString path;
	bool backups_enabled = true;
	{
		// Handle portability
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

	LocaleDialog::loadTranslator("kapow_", datadir);

	// Make sure data path exists
	if (path.isEmpty()) {
		path = Paths::dataPath();

		if (!QFile::exists(path)) {
			// Create data location
			QDir dir(path);
			if (!dir.mkpath(dir.absolutePath())) {
				QMessageBox::critical(nullptr, Window::tr("Error"), Window::tr("Unable to create time data location."));
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
					QMessageBox::warning(nullptr, Window::tr("Error"), Window::tr("Unable to move time data location."));
				}
			}
		}
	// Make sure command-line data path exists
	} else if (!QFile::exists(path + "/../")) {
		QDir dir(path + "/../");
		if (!dir.mkpath(dir.absolutePath())) {
			QMessageBox::critical(nullptr, Window::tr("Error"), Window::tr("Unable to create time data location."));
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
		Window::connect(&app, &QtSingleApplication::messageReceived, &window, &QMainWindow::show);
		return app.exec();
	} else {
		return 1;
	}
}
