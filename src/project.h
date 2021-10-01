/*
	SPDX-FileCopyrightText: 2009-2020 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef PROJECT_H
#define PROJECT_H

#include <QDateTime>
#include <QTreeWidgetItem>
#include <QXmlStreamWriter>
class SessionModel;
class FilterModel;

class Project : public QObject, public QTreeWidgetItem {
	Q_OBJECT
public:
	Project(QTreeWidget* parent, const QString& project);
	Project(QTreeWidgetItem* parent, const QString& project);

	bool start(const QDateTime& current);
	bool stop(QDateTime current = QDateTime());
	void updateTime(const QDateTime& current);
	void setTask(const QString& task);

	bool isActive() const;

	SessionModel* model() const {
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
	SessionModel* m_model;
	FilterModel* m_filter_model;
	QDateTime m_start_time;
	QString m_task;
	bool m_active;
	int m_scroll_value;
};

#endif
