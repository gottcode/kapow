/*
	SPDX-FileCopyrightText: 2008 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KAPOW_TIME_EDITOR_H
#define KAPOW_TIME_EDITOR_H

#include <QTimeEdit>

class TimeEditor : public QTimeEdit
{
	Q_OBJECT

public:
	explicit TimeEditor(QWidget* parent = nullptr);
};

#endif // KAPOW_TIME_EDITOR_H
