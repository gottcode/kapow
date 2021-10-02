/*
	SPDX-FileCopyrightText: 2008 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KAPOW_DATE_EDITOR_H
#define KAPOW_DATE_EDITOR_H

#include <QDateEdit>

class DateEditor : public QDateEdit
{
	Q_OBJECT

public:
	explicit DateEditor(QWidget* parent = nullptr);
};

#endif // KAPOW_DATE_EDITOR_H
