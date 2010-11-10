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

#include "session.h"

#include <QLocale>

#include <cmath>

/*****************************************************************************/

Session::Session() {
	d = new SessionData(QDate(), QTime(), QTime(), QString(), false);
}

/*****************************************************************************/

Session::Session(const QDate& date, const QTime& start, const QTime& stop, const QString& task, bool billed) {
	d = new SessionData(date, start, stop, task, billed);
	updateTotals();
}

/*****************************************************************************/

QString Session::total(Time time, bool decimals, bool unit) const {
	QString result;
	int seconds = d->m_totals[time];
	if (decimals) {
		float hours = qRound(d->m_totals[time] / 360.0f) * 0.1f;
		if (!unit) {
			result = QLocale().toString(hours, 'f', 1);
		} else {
			if (hours == std::floor(hours)) {
				result = tr("%n hour(s)", "", hours);
			} else {
				result = tr("%1 hours").arg(hours);
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

/*****************************************************************************/

void Session::updateTotals(const Session& previous) {
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

/*****************************************************************************/

void Session::toXml(QXmlStreamWriter& xml) const {
	xml.writeEmptyElement("session");
	xml.writeAttribute("date", d->m_date.toString(Qt::ISODate));
	xml.writeAttribute("start", d->m_start.toString(Qt::ISODate));
	xml.writeAttribute("stop", d->m_stop.toString(Qt::ISODate));
	xml.writeAttribute("billed", QString::number(d->m_billed));
	xml.writeAttribute("note", d->m_task);
}

/*****************************************************************************/
