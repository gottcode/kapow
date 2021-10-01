/*
	SPDX-FileCopyrightText: 2010-2013 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "filter_model.h"

#include "session.h"
#include "session_model.h"

#include <QDate>

//-----------------------------------------------------------------------------

FilterModel::FilterModel(SessionModel* model, QObject* parent)
	: QSortFilterProxyModel(parent)
	, m_model(model)
	, m_type(All)
{
	setSourceModel(model);
	connect(model, &SessionModel::dataChanged, this, &FilterModel::invalidate);
}

//-----------------------------------------------------------------------------

QModelIndex FilterModel::mapUnbilledToSource(const QModelIndex& proxy_index) const
{
	QModelIndex index = mapToSource(proxy_index);
	if (!m_model->isBilled(index.row()) && (index.row() + 1 < m_model->rowCount())) {
		return index;
	} else {
		return QModelIndex();
	}
}

//-----------------------------------------------------------------------------

void FilterModel::setType(int type)
{
	m_type = type;
	invalidateFilter();
}

//-----------------------------------------------------------------------------

bool FilterModel::filterAcceptsRow(int row, const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	if (!parent.isValid() && (row < sourceModel()->rowCount() - 1)) {
		QDate current = QDate::currentDate();
		QDate date = m_model->session(row).date();
		int week1, year1, week2, year2;

		switch (m_type) {
		case All:
			return true;

		case Unbilled:
			return !m_model->isBilled(row);

		case ThisMonth:
			return (current.month() == date.month()) && (current.year() == date.year());

		case ThisYear:
			return current.year() == date.year();

		case ThisWeek:
			week1 = current.weekNumber(&year1);
			week2 = date.weekNumber(&year2);
			return (week1 == week2) && (year1 == year2);

		default:
			return false;
		}
	} else {
		return true;
	}
}

//-----------------------------------------------------------------------------
