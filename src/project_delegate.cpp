/***********************************************************************
 *
 * Copyright (C) 2009, 2010, 2012, 2013 Graeme Gott <graeme@gottcode.org>
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
