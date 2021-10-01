/*
	SPDX-FileCopyrightText: 2012-2014 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "rates.h"

#include "settings.h"

#include <QLocale>

//-----------------------------------------------------------------------------

Rates::Rates()
{
	// Use currency symbol of locale
	QLocale locale;
	m_currency_symbol = locale.currencySymbol();
	m_prepend_symbol = locale.toCurrencyString(1, m_currency_symbol).at(0) != '1';

	// Default to old settings values if they exist
	Settings settings;
	m_hourly = settings.value("ReportDialog/HourlyRate").toDouble();
	m_tax = settings.value("ReportDialog/TaxRate").toDouble();
	m_currency_symbol = settings.value("ReportDialog/Currency", m_currency_symbol).toString();
	m_prepend_symbol = settings.value("ReportDialog/PrependCurrency", m_prepend_symbol).toBool();

	// Remove settings values
	settings.remove("ReportDialog/HourlyRate");
	settings.remove("ReportDialog/TaxRate");
	settings.remove("ReportDialog/Currency");
	settings.remove("ReportDialog/PrependCurrency");
}

//-----------------------------------------------------------------------------

void Rates::toXml(QXmlStreamWriter& xml) const
{
	xml.writeStartElement(QLatin1String("rates"));
	if (!qFuzzyIsNull(m_hourly)) {
		xml.writeTextElement(QLatin1String("hourly"), QString::number(m_hourly, 'f', 2));
	}
	if (!qFuzzyIsNull(m_tax)) {
		xml.writeTextElement(QLatin1String("tax"), QString::number(m_tax, 'f', 2));
	}
	if (!m_currency_symbol.isEmpty()) {
		xml.writeStartElement(QLatin1String("currency"));
		xml.writeAttribute(QLatin1String("prepend"), QString::number(m_prepend_symbol));
		xml.writeCharacters(m_currency_symbol);
		xml.writeEndElement();
	}
	xml.writeEndElement();
}

//-----------------------------------------------------------------------------

void Rates::fromXml(QXmlStreamReader& xml)
{
	while (xml.readNextStartElement()) {
		if (xml.name() == QLatin1String("hourly")) {
			m_hourly = xml.readElementText().toDouble();
		} else if (xml.name() == QLatin1String("tax")) {
			m_tax = xml.readElementText().toDouble();
		} else if (xml.name() == QLatin1String("currency")) {
			m_prepend_symbol = xml.attributes().value(QLatin1String("prepend")).toString().toInt();
			m_currency_symbol = xml.readElementText().simplified();
		} else {
			xml.skipCurrentElement();
		}
	}
}

//-----------------------------------------------------------------------------
