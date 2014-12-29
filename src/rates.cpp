/***********************************************************************
 *
 * Copyright (C) 2012, 2014 Graeme Gott <graeme@gottcode.org>
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
