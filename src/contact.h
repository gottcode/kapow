/*
	SPDX-FileCopyrightText: 2012 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KAPOW_CONTACT_H
#define KAPOW_CONTACT_H

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

class Contact
{
public:
	Contact();

	QString name() const
		{ return m_name; }
	QString company() const
		{ return m_company; }
	QString address() const
		{ return m_address; }
	QString phone() const
		{ return m_phone; }
	QString fax() const
		{ return m_fax; }
	QString email() const
		{ return m_email; }
	QString website() const
		{ return m_website; }

	void setName(const QString& name)
		{ m_name = name; }
	void setCompany(const QString& company)
		{ m_company = company; }
	void setAddress(const QString& address)
		{ m_address = address; }
	void setPhone(const QString& phone)
		{ m_phone = phone; }
	void setFax(const QString& fax)
		{ m_fax = fax; }
	void setEmail(const QString& email)
		{ m_email = email; }
	void setWebsite(const QString& website)
		{ m_website = website; }

	void toXml(QXmlStreamWriter& xml) const;
	void fromXml(QXmlStreamReader& xml);

private:
	QString m_name;
	QString m_company;
	QString m_address;
	QString m_phone;
	QString m_fax;
	QString m_email;
	QString m_website;
};

#endif // KAPOW_CONTACT_H
