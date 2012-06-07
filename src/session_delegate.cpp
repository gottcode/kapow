/***********************************************************************
 *
 * Copyright (C) 2012 Graeme Gott <graeme@gottcode.org>
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

#include "session_delegate.h"

#include <QDateEdit>
#include <QLineEdit>
#include <QPainter>

//-----------------------------------------------------------------------------

SessionDelegate::SessionDelegate(QObject* parent) :
	QStyledItemDelegate(parent)
{
	m_height = qMax(QDateEdit().sizeHint().height(), QLineEdit().sizeHint().height());
}

//-----------------------------------------------------------------------------

void SessionDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QStyleOptionViewItemV4 opt = option;
	initStyleOption(&opt, index);

	if (!index.parent().isValid()) {
		if ((index.model()->rowCount() - 1) > index.row()) {
			// Set up drawing of billed rows
			if (index.data(Qt::UserRole).toBool()) {
				opt.palette.setBrush(QPalette::Text, opt.palette.brush(QPalette::Disabled, QPalette::Text));
			}
		} else {
			// Set up drawing of unbilled totals row
			opt.features |= QStyleOptionViewItemV2::Alternate;
			opt.state &= ~QStyle::State_HasFocus;
		}

		// Draw text
		QStyledItemDelegate::paint(painter, opt, index);
	} else {
		// Set up drawing of totals row
		opt.features |= QStyleOptionViewItemV2::Alternate;
		opt.palette.setBrush(QPalette::Text, opt.palette.brush(QPalette::Disabled, QPalette::Text));
		opt.palette.setBrush(QPalette::AlternateBase, opt.palette.brush(QPalette::Disabled, QPalette::AlternateBase));
		opt.state &= ~QStyle::State_HasFocus;

		// Draw text
		QStyledItemDelegate::paint(painter, opt, index);

		// Draw drop shadow below totals row
		painter->save();
		int y = opt.rect.bottom();
		const int alphas[5] = { 99, 50, 20, 6, 1 };
		QColor color(0,0,0);
		for (int i = 0; i < 5; ++i) {
			color.setAlpha(alphas[i]);
			painter->setPen(color);
			painter->drawLine(opt.rect.left(), y, opt.rect.right(), y);
			--y;
		}
		painter->restore();
	}
}

//-----------------------------------------------------------------------------

QSize SessionDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QSize size = QStyledItemDelegate::sizeHint(option, index);
	if (index.parent().isValid() || (index.model()->rowCount() - 1) == index.row()) {
		size.setHeight(size.height() + 3);
	} else {
		size.setHeight(m_height);
	}
	return size;
}

//-----------------------------------------------------------------------------
