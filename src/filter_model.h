/*
	SPDX-FileCopyrightText: 2010-2013 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef FILTER_MODEL_H
#define FILTER_MODEL_H

#include <QSortFilterProxyModel>
class SessionModel;

class FilterModel : public QSortFilterProxyModel {
public:
	FilterModel(SessionModel* model, QObject* parent = 0);

	QModelIndex mapUnbilledToSource(const QModelIndex& proxy_index) const;

	enum FilterType {
		All,
		Unbilled,
		ThisYear,
		ThisMonth,
		ThisWeek
	};

	int type() const {
		return m_type;
	}

	void setType(int type);

protected:
	bool filterAcceptsRow(int row, const QModelIndex& parent) const;

private:
	SessionModel* m_model;
	int m_type;
};

#endif
