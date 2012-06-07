/***********************************************************************
 *
 * Copyright (C) 2009, 2012 Graeme Gott <graeme@gottcode.org>
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

#ifndef REPORT_H
#define REPORT_H

class Contact;
class DataModel;
class Rates;

#include <QDialog>
class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLineEdit;
class QTextEdit;
class QTreeView;

class Report : public QDialog {
	Q_OBJECT
public:
	Report(DataModel* data, int current, Contact* contact, Rates* rates, QWidget* parent = 0);

protected:
	virtual void hideEvent(QHideEvent* event);

private slots:
	void currencyChanged();
	void generateText();
	void groupSelected(int group);
	void print();
	void reset();
	void save();
	void bill();

private:
	void findGroups();
	QString generateHtml() const;
	void writeHtml(QString filename);
	void writeICalendar(QString filename);
	void writeOutlookCsv(QString filename);

private:
	QTextEdit* m_preview;

	QComboBox* m_groups;
	QTreeView* m_details;
	DataModel* m_data;
	int m_current_row;
	QDoubleSpinBox* m_hourly_rate;
	QDoubleSpinBox* m_tax_rate;
	QLineEdit* m_currency_symbol;
	QCheckBox* m_prepend_symbol;

	QLineEdit* m_name;
	QLineEdit* m_company;
	QTextEdit* m_address;
	QLineEdit* m_phone;
	QLineEdit* m_fax;
	QLineEdit* m_email;
	QLineEdit* m_website;

	Contact* m_contact;
	Rates* m_rates;
};

#endif
