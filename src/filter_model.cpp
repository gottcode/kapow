/***********************************************************************
 *
 * Copyright (C) 2010, 2011, 2012, 2013 Graeme Gott <graeme@gottcode.org>
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

#include "filter_model.h"

#include "session.h"
#include "session_model.h"

#include <QDate>

/*****************************************************************************/

FilterModel::FilterModel(SessionModel* model, QObject* parent)
: QSortFilterProxyModel(parent), m_model(model), m_type(All) {
	setSourceModel(model);
	connect(model, SIGNAL(dataChanged(QModelIndex, QModelIndex)), this, SLOT(invalidate()));
}

/*****************************************************************************/

QModelIndex FilterModel::mapUnbilledToSource(const QModelIndex& proxy_index) const {
	QModelIndex index = mapToSource(proxy_index);
	if (!m_model->isBilled(index.row()) && (index.row() + 1 < m_model->rowCount())) {
		return index;
	} else {
		return QModelIndex();
	}
}

/*****************************************************************************/

void FilterModel::setType(int type) {
	m_type = type;
	invalidateFilter();
}

/*****************************************************************************/

bool FilterModel::filterAcceptsRow(int row, const QModelIndex& parent) const {
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
			if (current.month() != date.month()) {
				return false;
			}
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

/*****************************************************************************/
