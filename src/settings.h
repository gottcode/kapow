/*
	SPDX-FileCopyrightText: 2012 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KAPOW_SETTINGS_H
#define KAPOW_SETTINGS_H

#include <QSettings>
#include <QSharedPointer>

class Settings
{
public:
	explicit Settings();

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

#endif // KAPOW_SETTINGS_H
