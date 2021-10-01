/*
	SPDX-FileCopyrightText: 2008-2011 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "time_editor.h"

/*****************************************************************************/

TimeEditor::TimeEditor(QWidget* parent)
: QTimeEdit(parent) {
	setDisplayFormat(QLocale().timeFormat(QLocale::LongFormat).contains("AP", Qt::CaseInsensitive) ? "h:mm:ss AP" : "HH:mm:ss");
}

/*****************************************************************************/
