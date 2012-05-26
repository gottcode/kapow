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

#ifndef CONTACT_H
#define CONTACT_H

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

#endif
