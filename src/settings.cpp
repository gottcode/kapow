/*
	SPDX-FileCopyrightText: 2012 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

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
	QDir dir(f_path + "/../");
	if (!dir.exists()) {
		dir.mkpath(dir.absolutePath());
	}
}

//-----------------------------------------------------------------------------
