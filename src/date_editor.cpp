/*
	SPDX-FileCopyrightText: 2008 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "date_editor.h"

/*****************************************************************************/

DateEditor::DateEditor(QWidget* parent)
: QDateEdit(parent) {
	setCalendarPopup(true);
}

/*****************************************************************************/
