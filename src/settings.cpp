/***********************************************************************
 *
 * Copyright (C) 2012 Graeme Gott <graeme@gottcode.org>
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

#include "settings.h"

#include <QDir>

//-----------------------------------------------------------------------------

static QString f_path;
static QWeakPointer<QSettings> f_settings;

//-----------------------------------------------------------------------------

Settings::Settings()
{
	if (f_settings) {
		m_settings = f_settings;
	} else if (f_path.isEmpty()) {
		f_settings = m_settings = QSharedPointer<QSettings>(new QSettings);
	} else {
		f_settings = m_settings = QSharedPointer<QSettings>(new QSettings(f_path, QSettings::IniFormat));
	}
}

//-----------------------------------------------------------------------------

void Settings::setPath(const QString& path)
{
	// Set path
	f_path = path;
	if (f_path.isEmpty()) {
		return;
	}

	// Make sure location of INI file exists
	QDir dir(f_path);
	if (!dir.exists()) {
		dir.mkpath(dir.absolutePath());
	}
}

//-----------------------------------------------------------------------------
