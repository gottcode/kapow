/*
	SPDX-FileCopyrightText: 2010-2013 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KAPOW_FILTER_MODEL_H
#define KAPOW_FILTER_MODEL_H

class SessionModel;

#include <QSortFilterProxyModel>

class FilterModel : public QSortFilterProxyModel
{
public:
	explicit FilterModel(SessionModel* model, QObject* parent = nullptr);

	QModelIndex mapUnbilledToSource(const QModelIndex& proxy_index) const;

	enum FilterType
	{
		All,
		Unbilled,
		ThisYear,
		ThisMonth,
		ThisWeek
	};

	int type() const
	{
		return m_type;
	}

	void setType(int type);

protected:
	bool filterAcceptsRow(int row, const QModelIndex& parent) const override;

private:
	SessionModel* m_model;
	int m_type;
};

#endif // KAPOW_FILTER_MODEL_H
