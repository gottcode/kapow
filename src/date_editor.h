/*
	SPDX-FileCopyrightText: 2008 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KAPOW_DATE_EDITOR_H
#define KAPOW_DATE_EDITOR_H

#include <QDateEdit>

class DateEditor : public QDateEdit {
	Q_OBJECT
	Q_PROPERTY(QDate date READ date WRITE setDate USER true)
public:
	DateEditor(QWidget* parent = 0);
};

#endif // KAPOW_DATE_EDITOR_H
