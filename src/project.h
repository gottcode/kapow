/***********************************************************************
 *
 * Copyright (C) 2009, 2010 Graeme Gott <graeme@gottcode.org>
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

#ifndef PROJECT_H
#define PROJECT_H

#include <QDateTime>
#include <QTreeWidgetItem>
#include <QXmlStreamWriter>
class DataModel;
class FilterModel;

class Project : public QObject, public QTreeWidgetItem {
	Q_OBJECT
public:
	Project(QTreeWidget* parent, const QString& project);
	Project(QTreeWidgetItem* parent, const QString& project);

	void start(const QDateTime& current);
	void stop(const QDateTime& current = QDateTime());
	void updateTime(const QDateTime& current);

	bool isActive() const {
		return m_active;
	}

	DataModel* model() const {
		return m_model;
	}

	FilterModel* filterModel() const {
		return m_filter_model;
	}

	int scrollValue() const {
		return m_scroll_value;
	}

	void setDecimalTotals(bool decimals);
	void setScrollValue(int value);
	QString time() const;

	void toXml(QXmlStreamWriter& xml) const;

private slots:
	void billedStatusChanged(bool billed);

private:
	void init();

private:
	DataModel* m_model;
	FilterModel* m_filter_model;
	QDateTime m_start_time;
	bool m_active;
	int m_scroll_value;
};

#endif
