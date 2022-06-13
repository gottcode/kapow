/*
	SPDX-FileCopyrightText: 2009-2021 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KAPOW_REPORT_H
#define KAPOW_REPORT_H

class Contact;
class Rates;
class SessionModel;

#include <QDialog>
class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLineEdit;
class QPushButton;
class QTextEdit;
class QTreeView;

class Report : public QDialog
{
	Q_OBJECT

public:
	Report(SessionModel* data, int current, Contact* contact, Rates* rates, QWidget* parent = nullptr);

protected:
	void hideEvent(QHideEvent* event) override;

private Q_SLOTS:
	void currencyChanged();
	void generateText();
	void groupSelected(int group);
	void print();
	void reset();
	void save();
	void bill();
	void unbill();

private:
	void findGroups();
	QString generateHtml() const;
	void writeHtml(QString filename) const;
	void writeICalendar(QString filename) const;
	void writeOutlookCsv(QString filename) const;

private:
	QTextEdit* m_preview;

	QComboBox* m_groups;
	QTreeView* m_details;
	SessionModel* m_data;
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

	QPushButton* m_delete_button;
};

#endif // KAPOW_REPORT_H
