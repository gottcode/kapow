/*
	SPDX-FileCopyrightText: 2012 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "contact.h"

#include "settings.h"

//-----------------------------------------------------------------------------

Contact::Contact()
{
	// Default to old settings values if they exist
	Settings settings;
	m_name = settings.value("Contact/Name").toString();
	m_company = settings.value("Contact/Company").toString();
	m_address = settings.value("Contact/Address").toString();
	m_phone = settings.value("Contact/Phone").toString();
	m_fax = settings.value("Contact/Fax").toString();
	m_email = settings.value("Contact/Email").toString();
	m_website = settings.value("Contact/Website").toString();

	// Remove settings values
	settings.remove("Contact/Name");
	settings.remove("Contact/Company");
	settings.remove("Contact/Address");
	settings.remove("Contact/Phone");
	settings.remove("Contact/Fax");
	settings.remove("Contact/Email");
	settings.remove("Contact/Website");
}

//-----------------------------------------------------------------------------

void Contact::toXml(QXmlStreamWriter& xml) const
{
	xml.writeStartElement(QLatin1String("contact"));
	if (!m_name.isEmpty()) {
		xml.writeTextElement(QLatin1String("name"), m_name);
	}
	if (!m_company.isEmpty()) {
		xml.writeTextElement(QLatin1String("company"), m_company);
	}
	if (!m_address.isEmpty()) {
		QString address = m_address;
		address.replace("\n", "\\n");
		xml.writeTextElement(QLatin1String("address"), address);
	}
	if (!m_phone.isEmpty()) {
		xml.writeTextElement(QLatin1String("phone"), m_phone);
	}
	if (!m_fax.isEmpty()) {
		xml.writeTextElement(QLatin1String("fax"), m_fax);
	}
	if (!m_email.isEmpty()) {
		xml.writeTextElement(QLatin1String("email"), m_email);
	}
	if (!m_website.isEmpty()) {
		xml.writeTextElement(QLatin1String("website"), m_website);
	}
	xml.writeEndElement();
}

//-----------------------------------------------------------------------------

void Contact::fromXml(QXmlStreamReader& xml)
{
	while (xml.readNextStartElement()) {
		if (xml.name() == QLatin1String("name")) {
			m_name = xml.readElementText().simplified();
		} else if (xml.name() == QLatin1String("company")) {
			m_company = xml.readElementText().simplified();
		} else if (xml.name() == QLatin1String("address")) {
			m_address = xml.readElementText().simplified();
			m_address.replace("\\n", "\n");
		} else if (xml.name() == QLatin1String("phone")) {
			m_phone = xml.readElementText().simplified();
		} else if (xml.name() == QLatin1String("fax")) {
			m_fax = xml.readElementText().simplified();
		} else if (xml.name() == QLatin1String("email")) {
			m_email = xml.readElementText().simplified();
		} else if (xml.name() == QLatin1String("website")) {
			m_website = xml.readElementText().simplified();
		} else {
			xml.skipCurrentElement();
		}
	}
}

//-----------------------------------------------------------------------------
