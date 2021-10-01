/*
	SPDX-FileCopyrightText: 2008 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef TIME_EDITOR_H
#define TIME_EDITOR_H

#include <QTimeEdit>

class TimeEditor : public QTimeEdit {
	Q_OBJECT
	Q_PROPERTY(QTime time READ time WRITE setTime USER true)
public:
	TimeEditor(QWidget* parent = 0);
};

#endif
