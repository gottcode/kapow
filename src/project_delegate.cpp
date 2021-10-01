/*
	SPDX-FileCopyrightText: 2009-2013 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "project_delegate.h"

#include <QLineEdit>

//-----------------------------------------------------------------------------

ProjectDelegate::ProjectDelegate(QObject* parent) :
	QStyledItemDelegate(parent)
{
	m_height = QLineEdit().sizeHint().height();
}

//-----------------------------------------------------------------------------

QWidget* ProjectDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if (index.column() == 0) {
		return QStyledItemDelegate::createEditor(parent, option, index);
	} else {
		return 0;
	}
}

//-----------------------------------------------------------------------------

QSize ProjectDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QSize size = QStyledItemDelegate::sizeHint(option, index);
	size.setHeight(m_height);
	return size;
}

//-----------------------------------------------------------------------------
