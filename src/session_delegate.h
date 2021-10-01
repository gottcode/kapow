/*
	SPDX-FileCopyrightText: 2012-2015 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KAPOW_SESSION_DELEGATE_H
#define KAPOW_SESSION_DELEGATE_H

#include <QStyledItemDelegate>

class SessionDelegate : public QStyledItemDelegate
{
public:
	SessionDelegate(QObject* parent = 0);

	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
	void setDevicePixelRatio(int ratio);
	void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;
	QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;

private:
	int m_height;
	int m_ratio;
	QVector<int> m_alphas;
};

#endif // KAPOW_SESSION_DELEGATE_H
