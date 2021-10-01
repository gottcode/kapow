/*
	SPDX-FileCopyrightText: 2012 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KAPOW_RATES_H
#define KAPOW_RATES_H

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

#endif // KAPOW_RATES_H
