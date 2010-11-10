/***********************************************************************
 *
 * Copyright (C) 2010 Graeme Gott <graeme@gottcode.org>
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

#ifndef FILTER_MODEL_H
#define FILTER_MODEL_H

#include <QSortFilterProxyModel>
class DataModel;

class FilterModel : public QSortFilterProxyModel {
public:
	FilterModel(DataModel* model, QObject* parent = 0);

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
	DataModel* m_model;
	int m_type;
};

#endif
