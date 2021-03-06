/***********************************************************************
 *
 * Copyright (C) 2015-2020 Graeme Gott <graeme@gottcode.org>
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

#include "paths.h"

#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QStringList>

//-----------------------------------------------------------------------------

QString Paths::dataPath()
{
	static QString path = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
	return path;
}

//-----------------------------------------------------------------------------

QString Paths::oldDataPath()
{
	QStringList oldpaths;
	QString oldpath;

#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
	// Data path from Qt 4 version of 1.4
	oldpaths.append(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/data/GottCode/Kapow");
#endif

	// Data path from 1.0
#if defined(Q_OS_MAC)
	oldpath = QDir::homePath() + "/Library/Application Support/GottCode/Kapow/";
#elif defined(Q_OS_UNIX)
	oldpath = QString::fromLocal8Bit(qgetenv("XDG_DATA_HOME"));
	if (oldpath.isEmpty()) {
		oldpath = QDir::homePath() + "/.local/share";
	}
	oldpath += "/gottcode/kapow/";
#elif defined(Q_OS_WIN)
	oldpath = QDir::homePath() + "/Application Data/GottCode/Kapow/";
#endif
	if (!oldpaths.contains(oldpath)) {
		oldpaths.append(oldpath);
	}
	oldpaths.removeAll(dataPath() + "/");

	// Check if an old data location exists
	oldpath.clear();
	for (const QString& testpath : oldpaths) {
		if (QFile::exists(testpath)) {
			oldpath = testpath;
			break;
		}
	}

	return oldpath;
}

//-----------------------------------------------------------------------------
