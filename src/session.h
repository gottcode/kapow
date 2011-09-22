/***********************************************************************
 *
 * Copyright (C) 2009 Graeme Gott <graeme@gottcode.org>
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

#ifndef SESSION_H
#define SESSION_H

#include <QCoreApplication>
#include <QDateTime>
#include <QExplicitlySharedDataPointer>
#include <QXmlStreamWriter>

class Session {
	Q_DECLARE_TR_FUNCTIONS(Session)

public:
	Session();
	Session(const QDate& date, const QTime& start, const QTime& stop, const QString& task, bool billed);

	bool isValid() const {
		return d->m_date.isValid() && (d->m_start <= d->m_stop);
	}

	QDate date() const {
		return d->m_date;
	}

	QTime start() const {
		return d->m_start;
	}

	QTime stop() const {
		return d->m_stop;
	}

	QString task() const {
		return d->m_task;
	}

	bool isBilled() const {
		return d->m_billed;
	}

	void setBilled(bool billed) {
		d->m_billed = billed;
	}

	enum Time {
		Duration = 0,
		Daily,
		Weekly,
		Monthly,
		Total
	};
	QString total(Time time, bool decimals, bool unit = false) const;
	void updateTotals(const Session& previous = Session());
	void toXml(QXmlStreamWriter& xml) const;

private:
	class SessionData: public QSharedData {
	public:
		SessionData(const QDate& date, const QTime& start, const QTime& stop, const QString& task, bool billed)
		: m_date(date), m_start(start.addMSecs(-start.msec())), m_stop(stop.addMSecs(-stop.msec())), m_task(task), m_billed(billed), m_totals(Total + 1) {
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

#endif
