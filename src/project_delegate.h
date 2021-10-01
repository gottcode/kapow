/*
	SPDX-FileCopyrightText: 2009-2013 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KAPOW_PROJECT_DELEGATE_H
#define KAPOW_PROJECT_DELEGATE_H

#include <QStyledItemDelegate>

class ProjectDelegate : public QStyledItemDelegate
{
public:
	explicit ProjectDelegate(QObject* parent = nullptr);

	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
	QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
	int m_height;
};

#endif // KAPOW_PROJECT_DELEGATE_H
