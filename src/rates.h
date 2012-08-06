/***********************************************************************
 *
 * Copyright (C) 2012 Graeme Gott <graeme@gottcode.org>
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

#ifndef RATES_H
#define RATES_H

#include <QCoreApplication>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

class Rates {
	Q_DECLARE_TR_FUNCTIONS(Rates)

public:
	Rates();

	double hourly() const
		{ return m_hourly; }
	double tax() const
		{ return m_tax; }
	QString currencySymbol() const
		{ return m_currency_symbol; }
	bool prependSymbol() const
		{ return m_prepend_symbol; }

	void setHourly(double hourly)
		{ m_hourly = hourly; }
	void setTax(double tax)
		{ m_tax = tax; }
	void setCurrencySymbol(const QString& symbol)
		{ m_currency_symbol = symbol; }
	void setPrependSymbol(bool prepend)
		{ m_prepend_symbol = prepend; }

	void toXml(QXmlStreamWriter& xml) const;
	void fromXml(QXmlStreamReader& xml);

private:
	double m_hourly;
	double m_tax;
	QString m_currency_symbol;
	bool m_prepend_symbol;
};

#endif
