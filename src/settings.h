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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>
#include <QSharedPointer>

class Settings
{
public:
	Settings();

	void remove(const QString& key)
	{
		m_settings->remove(key);
	}

	void setValue(const QString& key, const QVariant& value)
	{
		m_settings->setValue(key, value);
	}

	QVariant value(const QString& key, const QVariant& defaultValue = QVariant()) const
	{
		return m_settings->value(key, defaultValue);
	}

	static void setPath(const QString& path);

private:
	QSharedPointer<QSettings> m_settings;
};

#endif
