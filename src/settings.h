/*
	SPDX-FileCopyrightText: 2012 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

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
