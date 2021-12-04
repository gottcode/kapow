/*
	SPDX-FileCopyrightText: 2009-2013 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KAPOW_SESSION_H
#define KAPOW_SESSION_H

#include <QCoreApplication>
#include <QDateTime>
#include <QExplicitlySharedDataPointer>
#include <QXmlStreamWriter>

class Session
{
	Q_DECLARE_TR_FUNCTIONS(Session)

public:
	explicit Session();
	Session(const QDate& date, const QTime& start, const QTime& stop, const QString& task, bool billed);

	bool isValid() const
	{
		return d->m_date.isValid() && (d->m_start <= d->m_stop);
	}

	QDate date() const
	{
		return d->m_date;
	}

	QTime start() const
	{
		return d->m_start;
	}

	QTime stop() const
	{
		return d->m_stop;
	}

	QString task() const
	{
		return d->m_task;
	}

	bool isBilled() const
	{
		return d->m_billed;
	}

	void setBilled(bool billed)
	{
		d->m_billed = billed;
	}

	enum Time
	{
		Duration = 0,
		Daily,
		Weekly,
		Monthly,
		Total
	};
	double total() const;
	QString total(Time time, bool decimals, bool unit = false) const;
	void updateTotals(const Session& previous = Session());
	void toXml(QXmlStreamWriter& xml) const;

private:
	class SessionData: public QSharedData
	{
	public:
		SessionData(const QDate& date, const QTime& start, const QTime& stop, const QString& task, bool billed)
			: m_date(date)
			, m_start(start.addMSecs(-start.msec()))
			, m_stop(stop.addMSecs(-stop.msec()))
			, m_task(task)
			, m_billed(billed)
			, m_totals(Total + 1)
		{
		}

		QDate m_date;
		QTime m_start;
		QTime m_stop;
		QString m_task;
		bool m_billed;
		QVector<int> m_totals;
	};
	QExplicitlySharedDataPointer<SessionData> d;
};

bool operator==(const Session& lhs, const Session &rhs);

#endif // KAPOW_SESSION_H
