/*
	SPDX-FileCopyrightText: 2012-2017 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "session_delegate.h"

#include <QDateEdit>
#include <QFont>
#include <QLineEdit>
#include <QMessageBox>
#include <QMetaProperty>
#include <QPainter>

#include <algorithm>
#include <cmath>

//-----------------------------------------------------------------------------

SessionDelegate::SessionDelegate(QObject* parent) :
	QStyledItemDelegate(parent),
	m_ratio(1)
{
	m_height = std::max(QDateEdit().sizeHint().height(), QLineEdit().sizeHint().height());
}

//-----------------------------------------------------------------------------

void SessionDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QStyleOptionViewItem opt = option;
	initStyleOption(&opt, index);

	if (!index.parent().isValid()) {
		if ((index.model()->rowCount() - 1) > index.row()) {
			// Set up drawing of billed rows
			if (index.data(Qt::UserRole).toBool()) {
				opt.palette.setBrush(QPalette::Text, opt.palette.brush(QPalette::Disabled, QPalette::Text));
			}
		} else {
			// Set up drawing of unbilled totals row
			opt.features |= QStyleOptionViewItem::Alternate;
			opt.font.setWeight(QFont::Bold);
			opt.state &= ~QStyle::State_HasFocus;
		}

		// Draw text
		QStyledItemDelegate::paint(painter, opt, index);
	} else {
		// Set up drawing of totals row
		opt.features |= QStyleOptionViewItem::Alternate;
		opt.font.setWeight(QFont::Bold);
		opt.palette.setBrush(QPalette::Text, opt.palette.brush(QPalette::Disabled, QPalette::Text));
		opt.palette.setBrush(QPalette::AlternateBase, opt.palette.brush(QPalette::Disabled, QPalette::AlternateBase));
		opt.state &= ~QStyle::State_HasFocus;

		// Draw text
		QStyledItemDelegate::paint(painter, opt, index);

		// Draw drop shadow below totals row
		painter->save();
		painter->translate(0, opt.rect.bottom() + 1);
		painter->scale(1.0, 1.0 / m_ratio);
		int y = 0;
		QColor color(0,0,0);
		for (int i = 0, end = m_alphas.size(); i < end; ++i) {
			color.setAlpha(m_alphas[i]);
			painter->setPen(color);
			painter->drawLine(opt.rect.left(), y, opt.rect.right(), y);
			--y;
		}
		painter->restore();
	}
}

//-----------------------------------------------------------------------------

void SessionDelegate::setDevicePixelRatio(int ratio)
{
	m_ratio = ratio;

	const int count = m_ratio * 5;
	m_alphas.resize(count);

	const qreal delta = 1.0 / m_ratio;
	for (int i = 0; i < count; ++i)
	{
		const qreal x = i * delta;
		qreal y = (-0.75 * x * x * x) + (11.875 * x * x) + (-60.0 * x) + 99;
		m_alphas[i] = std::lround(y);
	}
}

//-----------------------------------------------------------------------------

void SessionDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
	QByteArray prop = editor->metaObject()->userProperty().name();
	if (!model->setData(index, editor->property(prop), Qt::EditRole)) {
		QMessageBox::warning(0, tr("Error"), tr("Session conflicts with other sessions."));
	}
}

//-----------------------------------------------------------------------------

QSize SessionDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QSize size;
	if (index.parent().isValid() || (index.model()->rowCount() - 1) == index.row()) {
		QStyleOptionViewItem opt = option;
		opt.font.setWeight(QFont::Bold);
		size = QStyledItemDelegate::sizeHint(opt, index);
		size.setHeight(size.height() + (3 * m_ratio));
	} else {
		size.setWidth(QStyledItemDelegate::sizeHint(option, index).width());
		size.setHeight(m_height);
	}
	return size;
}

//-----------------------------------------------------------------------------
