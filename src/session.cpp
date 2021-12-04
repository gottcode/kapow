/*
	SPDX-FileCopyrightText: 2009-2013 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "session.h"

#include <QLocale>

#include <cmath>

//-----------------------------------------------------------------------------

Session::Session()
{
	d = new SessionData(QDate(), QTime(), QTime(), QString(), false);
}

//-----------------------------------------------------------------------------

Session::Session(const QDate& date, const QTime& start, const QTime& stop, const QString& task, bool billed)
{
	d = new SessionData(date, start, stop, task, billed);
	updateTotals();
}

//-----------------------------------------------------------------------------

double Session::total() const
{
	return d->m_totals[Total] / 3600.0;
}

//-----------------------------------------------------------------------------

QString Session::total(Time time, bool decimals, bool unit) const
{
	QString result;
	int seconds = d->m_totals[time];
	if (decimals) {
		double hours = d->m_totals[time] / 3600.0;
		if (!unit) {
			result = QLocale().toString(hours, 'f', 1);
		} else {
			if (hours == std::floor(hours)) {
				result = tr("%n hour(s)", "", hours);
			} else {
				result = tr("%L1 hours").arg(hours, 0, 'f', 1);
			}
		}
	} else {
		int hours = std::floor(seconds / 3600.f);
		seconds -= (hours * 3600);
		int minutes = std::floor(seconds / 60.f);
		seconds -= (minutes * 60);
		result = QString("%1:%2:%3").arg(hours).arg(minutes, 2, 10, QLatin1Char('0')).arg(seconds, 2, 10, QLatin1Char('0'));
	}
	return result;
}

//-----------------------------------------------------------------------------

void Session::updateTotals(const Session& previous)
{
	int duration = d->m_start.secsTo(d->m_stop);
	for (int i = Duration; i <= Total; ++i) {
		d->m_totals[i] = duration;
	}

	if (!previous.isValid() || previous.d->m_billed) {
		return;
	}

	if (previous.date() == date()) {
		d->m_totals[Daily] += previous.d->m_totals[Daily];
	}

	if (previous.date().weekNumber() == date().weekNumber()) {
		d->m_totals[Weekly] += previous.d->m_totals[Weekly];
	}

	if (previous.date().month() == date().month()) {
		d->m_totals[Monthly] += previous.d->m_totals[Monthly];
	}

	d->m_totals[Total] += previous.d->m_totals[Total];
}

//-----------------------------------------------------------------------------

void Session::toXml(QXmlStreamWriter& xml) const
{
	xml.writeEmptyElement("session");
	xml.writeAttribute("date", d->m_date.toString(Qt::ISODate));
	xml.writeAttribute("start", d->m_start.toString(Qt::ISODate));
	xml.writeAttribute("stop", d->m_stop.toString(Qt::ISODate));
	xml.writeAttribute("billed", QString::number(d->m_billed));
	xml.writeAttribute("note", d->m_task);
}

//-----------------------------------------------------------------------------

bool operator==(const Session& lhs, const Session &rhs)
{
	return (lhs.date() == rhs.date())
			&& (lhs.start() == rhs.start())
			&& (lhs.stop() == rhs.stop())
			&& (lhs.task() == rhs.task())
			&& (lhs.isBilled() == rhs.isBilled());
}

//-----------------------------------------------------------------------------
