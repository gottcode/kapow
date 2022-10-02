/*
	SPDX-FileCopyrightText: 2009-2021 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "report.h"

#include "contact.h"
#include "rates.h"
#include "settings.h"
#include "session_model.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QHostInfo>
#include <QLineEdit>
#include <QMessageBox>
#include <QPrintDialog>
#include <QPrinter>
#include <QPushButton>
#include <QStandardPaths>
#include <QTabWidget>
#include <QTextEdit>
#include <QTextStream>
#include <QTreeView>
#include <QVBoxLayout>

//-----------------------------------------------------------------------------

Report::Report(SessionModel* data, int current, Contact* contact, Rates* rates, QWidget* parent)
	: QDialog(parent)
	, m_data(data)
	, m_current_row(current)
	, m_contact(contact)
	, m_rates(rates)
	, m_delete_button(nullptr)
{
	// Create contact information widgets
	QWidget* contact_info_tab = new QWidget(this);
	m_name = new QLineEdit(contact_info_tab);
	connect(m_name, &QLineEdit::textChanged, this, &Report::generateText);
	m_company = new QLineEdit(contact_info_tab);
	connect(m_company, &QLineEdit::textChanged, this, &Report::generateText);
	m_address = new QTextEdit(contact_info_tab);
	connect(m_address, &QTextEdit::textChanged, this, &Report::generateText);
	m_phone = new QLineEdit(contact_info_tab);
	connect(m_phone, &QLineEdit::textChanged, this, &Report::generateText);
	m_fax = new QLineEdit(contact_info_tab);
	connect(m_fax, &QLineEdit::textChanged, this, &Report::generateText);
	m_email = new QLineEdit(contact_info_tab);
	connect(m_email, &QLineEdit::textChanged, this, &Report::generateText);
	m_website = new QLineEdit(contact_info_tab);
	connect(m_website, &QLineEdit::textChanged, this, &Report::generateText);

	QFormLayout* contact_info_layout = new QFormLayout(contact_info_tab);
	contact_info_layout->addRow(tr("Name:"), m_name);
	contact_info_layout->addRow(tr("Company:"), m_company);
	contact_info_layout->addRow(tr("Address:"), m_address);
	contact_info_layout->addRow(tr("Phone:"), m_phone);
	contact_info_layout->addRow(tr("Fax:"), m_fax);
	contact_info_layout->addRow(tr("Email:"), m_email);
	contact_info_layout->addRow(tr("Website:"), m_website);

	// Create data widgets
	QWidget* data_tab = new QWidget(this);

	m_groups = new QComboBox(data_tab);
	connect(m_groups, &QComboBox::currentIndexChanged, this, &Report::groupSelected);

	m_details = new QTreeView(data_tab);
	m_details->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_details->setRootIsDecorated(false);
	m_details->setItemsExpandable(false);
	m_details->setSelectionMode(QAbstractItemView::SingleSelection);
	m_details->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_details->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	m_details->header()->setSectionsClickable(false);
	m_details->header()->setSectionsMovable(false);
	m_details->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

	m_details->setModel(m_data);
	m_details->header()->setSectionResizeMode(3, QHeaderView::Stretch);
	m_details->header()->setStretchLastSection(false);
	for (int i = 5; i < 10; ++i) {
		m_details->setColumnHidden(i, true);
	}

	m_hourly_rate = new QDoubleSpinBox(data_tab);
	m_hourly_rate->setRange(0.0, 10000000.0);
	m_hourly_rate->setSpecialValueText(tr("N/A"));
	connect(m_hourly_rate, &QDoubleSpinBox::valueChanged, this, &Report::generateText);
	m_tax_rate = new QDoubleSpinBox(data_tab);
	m_tax_rate->setRange(0.0, 100.0);
	m_tax_rate->setSuffix(QLocale().percent());
	m_tax_rate->setSpecialValueText(tr("N/A"));
	connect(m_tax_rate, &QDoubleSpinBox::valueChanged, this, &Report::generateText);
	m_currency_symbol = new QLineEdit(data_tab);
	connect(m_currency_symbol, &QLineEdit::textChanged, this, &Report::currencyChanged);
	m_prepend_symbol = new QCheckBox(tr("Prepend currency symbol"), data_tab);
	connect(m_prepend_symbol, &QCheckBox::toggled, this, &Report::currencyChanged);

	QFormLayout* data_rates_layout = new QFormLayout;
	data_rates_layout->setFormAlignment(Qt::AlignCenter | Qt::AlignTop);
	data_rates_layout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
	data_rates_layout->setContentsMargins(0, 0, 0, 0);
	data_rates_layout->addRow(tr("Hourly rate:"), m_hourly_rate);
	data_rates_layout->addRow(tr("Tax rate:"), m_tax_rate);
	data_rates_layout->addRow(tr("Currency symbol:"), m_currency_symbol);
	data_rates_layout->addRow("", m_prepend_symbol);

	QVBoxLayout* data_layout = new QVBoxLayout(data_tab);
	data_layout->addWidget(m_groups);
	data_layout->addWidget(m_details);
	data_layout->addLayout(data_rates_layout);

	// Create preview widget
	m_preview = new QTextEdit(this);
	m_preview->setReadOnly(true);
	m_preview->document()->setDefaultStyleSheet("td { padding: 0 5px; } th { padding: 0 5px; }");

	// Create tabs
	QTabWidget* tabs = new QTabWidget(this);
	tabs->addTab(contact_info_tab, tr("Contact Information"));
	tabs->addTab(data_tab, tr("Data"));
	tabs->addTab(m_preview, tr("Preview"));
	tabs->setCurrentIndex(1);

	// Create dialog actions
	QDialogButtonBox* buttons = new QDialogButtonBox(this);
	QPushButton* reset_button = buttons->addButton(QDialogButtonBox::Reset);
	QPushButton* export_button = buttons->addButton(tr("Export"), QDialogButtonBox::ActionRole);
	QPushButton* print_button = buttons->addButton(tr("Print"), QDialogButtonBox::ActionRole);
	if (m_data->isBilled(m_current_row)) {
		m_delete_button = buttons->addButton(tr("Remove"), QDialogButtonBox::ActionRole);
		connect(m_delete_button, &QPushButton::clicked, this, &Report::unbill);
		buttons->addButton(QDialogButtonBox::Close);
	} else {
		buttons->addButton(QDialogButtonBox::Cancel);
		QPushButton* ok_button = buttons->addButton(QDialogButtonBox::Ok);
		connect(ok_button, &QPushButton::clicked, this, &Report::bill);
	}
	if (style()->styleHint(QStyle::SH_DialogButtonBox_ButtonsHaveIcons)) {
		export_button->setIcon(QIcon::fromTheme("document-export"));
		print_button->setIcon(QIcon::fromTheme("document-print"));
		if (m_delete_button) {
			m_delete_button->setIcon(QIcon::fromTheme("user-trash"));
		}
	}
	connect(buttons, &QDialogButtonBox::rejected, this, &Report::reject);
	connect(reset_button, &QPushButton::clicked, this, &Report::reset);
	connect(export_button, &QPushButton::clicked, this, &Report::save);
	connect(print_button, &QPushButton::clicked, this, &Report::print);

	// Lay out dialog
	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(tabs, 1);
	layout->addWidget(buttons);

	resize(Settings().value("ReportDialog/Size", QSize(700, 500)).toSize());

	// Load data
	findGroups();
	reset();
}

//-----------------------------------------------------------------------------

void Report::hideEvent(QHideEvent* event)
{
	m_contact->setName(m_name->text());
	m_contact->setCompany(m_company->text());
	m_contact->setAddress(m_address->toPlainText());
	m_contact->setPhone(m_phone->text());
	m_contact->setFax(m_fax->text());
	m_contact->setEmail(m_email->text());
	m_contact->setWebsite(m_website->text());

	m_rates->setHourly(m_hourly_rate->value());
	m_rates->setTax(m_tax_rate->value());
	m_rates->setCurrencySymbol(m_currency_symbol->text().simplified());
	m_rates->setPrependSymbol(m_prepend_symbol->isChecked());

	Settings().setValue("ReportDialog/Size", size());

	QDialog::hideEvent(event);
}

//-----------------------------------------------------------------------------

void Report::currencyChanged()
{
	QString symbol = m_currency_symbol->text().simplified();
	if (m_prepend_symbol->isChecked()) {
		m_hourly_rate->setSuffix("");
		m_hourly_rate->setPrefix(symbol);
	} else {
		m_hourly_rate->setPrefix("");
		m_hourly_rate->setSuffix(symbol);
	}
	generateText();
}

//-----------------------------------------------------------------------------

void Report::generateText()
{

	m_preview->setHtml(generateHtml());
}

//-----------------------------------------------------------------------------

void Report::groupSelected(int group)
{
	if (group == -1) {
		return;
	}

	if (m_delete_button) {
		m_delete_button->setEnabled(group == 0);
	}

	for (int i = 0; i < m_data->rowCount(); ++i) {
		m_details->setRowHidden(i, QModelIndex(), true);
	}

	QList<QVariant> rows = m_groups->itemData(group).toList();
	for (const QVariant& row : rows) {
		m_details->setRowHidden(row.toInt(), QModelIndex(), false);
	}

	generateText();
}

//-----------------------------------------------------------------------------

void Report::print()
{
	QPrinter printer;
	QPrintDialog dialog(&printer, this);
	if (dialog.exec() != QDialog::Accepted) {
		return;
	}
	m_preview->print(&printer);
}

//-----------------------------------------------------------------------------

void Report::reset()
{
	m_name->setText(m_contact->name());
	m_company->setText(m_contact->company());
	m_address->setPlainText(m_contact->address());
	m_phone->setText(m_contact->phone());
	m_fax->setText(m_contact->fax());
	m_email->setText(m_contact->email());
	m_website->setText(m_contact->website());

	m_hourly_rate->setValue(m_rates->hourly());
	m_tax_rate->setValue(m_rates->tax());
	m_currency_symbol->setText(m_rates->currencySymbol());
	m_prepend_symbol->setChecked(m_rates->prependSymbol());
}

//-----------------------------------------------------------------------------

void Report::save()
{
	Settings settings;
	QString filter = settings.value("ReportDialog/Filter").toString();
	QString html_filter = tr("Web Page (*.html *.htm)");
	QString ical_filter = tr("iCalendar (*.ics)");
	QString outlook_filter = tr("Outlook CSV (*.csv)");
	if ("iCalendar" == filter) {
		filter = ical_filter + ";;" + html_filter + ";;" + outlook_filter;
	} else if ("Outlook" == filter) {
		filter = outlook_filter + ";;" + html_filter + ";;" + ical_filter;
	} else {
		filter = html_filter + ";;" + ical_filter + ";;" + outlook_filter;
	}

	QString selected_filter;
	QString filename = QFileDialog::getSaveFileName(this,
			tr("Export Report"),
			QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
			filter,
			&selected_filter);
	if (!filename.isEmpty()) {
		if (selected_filter == html_filter) {
			settings.remove("ReportDialog/Filter");
			writeHtml(filename);
		} else if (selected_filter == ical_filter) {
			settings.setValue("ReportDialog/Filter", "iCalendar");
			writeICalendar(filename);
		} else if (selected_filter == outlook_filter) {
			settings.setValue("ReportDialog/Filter", "Outlook");
			writeOutlookCsv(filename);
		}
	}
}

//-----------------------------------------------------------------------------

void Report::bill()
{
	m_data->setBilled(m_current_row, true);
	QDialog::accept();
}

//-----------------------------------------------------------------------------

void Report::unbill()
{
	if (QMessageBox::question(this, tr("Question"), tr("Remove newest report?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No) {
		return;
	}

	// Remove report
	m_data->setBilled(m_groups->currentData().toList().last().toInt(), false);
	m_groups->removeItem(m_groups->currentIndex());

	// Hide dialog if last report removed
	if (m_groups->count() == 0) {
		reject();
	}
}

//-----------------------------------------------------------------------------

void Report::findGroups()
{
	int current_group = -1;

	int count = m_data->rowCount();
	QList<int> billed = m_data->billedRows();

	QList<QVariant> rows;
	if (m_data->isBilled(m_current_row)) {
		setWindowTitle(tr("View Reports"));

		// Find groups of billed sessions
		for (int i = 0; i < count; ++i) {
			rows.append(i);
			if (billed.contains(i)) {
				QString from = m_data->data(m_data->index(rows.first().toInt(), 0)).toString();
				QString to = m_data->data(m_data->index(rows.last().toInt(), 0)).toString();
				m_groups->insertItem(0, QString("%1 - %2").arg(from).arg(to), rows);
				if ((i >= m_current_row) && (m_current_row >= rows.first().toInt())) {
					current_group = m_groups->count();
				}
				rows.clear();
			}
		}
	} else {
		setWindowTitle(tr("Create Report"));

		// Find unbilled data through current row
		for (int i = !billed.isEmpty() ? (billed.last() + 1) : 0; i <= m_current_row; ++i) {
			rows.append(i);
		}
		m_groups->addItem(QString(), rows);
		m_groups->hide();
		rows.clear();
	}

	// Select group of sessions with current row
	if (current_group != -1) {
		current_group = m_groups->count() - current_group;
	} else {
		current_group = 0;
	}
	m_groups->setCurrentIndex(current_group);
}

//-----------------------------------------------------------------------------

QString Report::generateHtml() const
{
	if (!m_data->rowCount()) {
		m_preview->clear();
		return QString();
	}

	QString title = tr("Time Sheet Report");
	QString html = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\"\n   \"http://www.w3.org/TR/html4/loose.dtd\">\n"
			"<html>\n"
			"<head>\n"
			"<title>" + title + "</title>\n"
			"<meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\">\n"
			"<style type=\"text/css\">td, th {white-space:pre}</style>\n"
			"</head>\n"
			"<body>\n"
			"<p align=\"center\"><b><big>" + title + "</big><br>\n" + m_groups->currentText() + "</b>\n<small>";

	// Add contact information
	QString info = m_name->text().simplified();
	if (!info.isEmpty()) {
		html += "<br>\n" + info;
	}
	info = m_company->text().simplified();
	if (!info.isEmpty()) {
		html += "<br>\n" + info;
	}
	info = m_address->toPlainText().simplified();
	if (!info.isEmpty()) {
		info.replace("\n", "<br>\n");
		html += "<br>\n" + info;
	}
	info = m_phone->text().simplified();
	if (!info.isEmpty()) {
		html += "<br>\n" + tr("Phone: %1").arg(info);
	}
	info = m_fax->text().simplified();
	if (!info.isEmpty()) {
		html += "<br>\n" + tr("Fax: %1").arg(info);
	}
	info = m_email->text().simplified();
	if (!info.isEmpty()) {
		html += "<br>\n" + info;
	}
	info = m_website->text().simplified();
	if (!info.isEmpty()) {
		html += "<br>\n" + info;
	}

	// Finish header
	html += "<br>\n</small></p>\n";

	// Add data
	html += "<table width=\"100%\">\n";

	QString header("<tr>"
			"<th align=\"left\">%1</th>"
			"<th align=\"left\">%2</th>"
			"<th align=\"left\">%3</th>"
			"<th align=\"center\" colspan=\"2\">%4</th>"
			"<th align=\"right\">%5</th>"
			"</tr>\n");
	for (int column = 0; column < 5; ++column) {
		header = header.arg(m_data->headerData(column, Qt::Horizontal).toString());
	}
	html += header;

	int last = 0;
	int rows = m_data->rowCount();
	for (int row = 0; row < rows; ++row) {
		if (m_details->isRowHidden(row, QModelIndex())) {
			continue;
		}
		last = row;

		QStringList columns;
		for (int column = 0; column < 5; ++column) {
			QModelIndex index = m_data->index(row, column);
			QString data = m_data->data(index).toString().simplified();
			columns.append(data);
		}
		html += QString("<tr>"
				"<td width=\"0%\" align=\"right\">%1</td>"
				"<td width=\"0%\" align=\"right\">%2</td>"
				"<td width=\"0%\" align=\"right\">%3</td>"
				"<td width=\"100%\" colspan=\"2\" style=\"white-space: normal;\">%4</td>"
				"<td width=\"0%\" align=\"right\">%5</td>"
				"</tr>\n").arg(columns[0], columns[1], columns[2], columns[3], columns[4]);
	}

	// Add billing information
	QString hours = m_data->session(last).total(Session::Total, true);
	double value = m_data->session(last).total();
	html += "<tr><td colspan=\"4\"></td><td colspan=\"2\"><hr></td>\n";
	html += "<tr><td colspan=\"4\"></td><td>" + tr("Hours") + "</td><td align=\"right\">" + hours + "</td></tr>\n";
	double hourly_rate = m_hourly_rate->value();
	QString currency(m_hourly_rate->prefix() + "%L1" + m_hourly_rate->suffix());
	if (hourly_rate) {
		value *= hourly_rate;
		double tax_rate = m_tax_rate->value() * 0.01;
		if (tax_rate) {
			html += "<tr><td colspan=\"4\"</td><td>" + tr("Subtotal") + "</td><td align=\"right\">" + QString(currency).arg(value, 0, 'f', 2) + "</td></tr>\n";
			double tax = tax_rate * value;
			html += "<tr><td colspan=\"4\"></td><td>" + tr("Taxes") + "</td><td align=\"right\">" + QString(currency).arg(tax, 0, 'f', 2) + "</td></tr>\n";
			value += tax;
		}
		html += "<tr><td colspan=\"4\"></td><td><b>" + tr("Total") + "</b></td><td align=\"right\"><b>" + QString(currency).arg(value, 0, 'f', 2) + "</b></td></tr>\n";
	}

	html += "</table>\n</body>\n</html>\n";

	return html;
}

//-----------------------------------------------------------------------------

void Report::writeHtml(QString filename) const
{
	if (!filename.endsWith(".html") && !filename.endsWith(".htm")) {
		filename.append(".html");
	}
	QFile file(filename);
	if (file.open(QFile::WriteOnly | QFile::Text)) {
		QTextStream stream(&file);
		stream << generateHtml();
		file.close();
	}
}

//-----------------------------------------------------------------------------

static void writeWrappedLine(const QByteArray& line, QIODevice* device)
{
	int end = line.length();
	int start = 0;
	int pos = 0;
	Q_FOREVER {
		pos = start + 74;
		if (pos >= end) {
			// Write unwrapped line or end of wrapped line
			device->write(&line.constData()[start], end - start);
			device->write("\r\n");
			break;
		} else {
			// Don't split UTF-8 characters
			while ((line[pos] & 0x80) && !(line[pos] & 0x40)) {
				pos--;
			}

			// Write wrapped text
			device->write(&line.constData()[start], pos - start);
			device->write("\r\n ");
			start = pos;
		}
	}
}

void Report::writeICalendar(QString filename) const
{
	if (!filename.endsWith(".ics")) {
		filename.append(".ics");
	}
	QFile file(filename);
	if (file.open(QFile::WriteOnly)) {
		file.write("BEGIN:VCALENDAR\r\nVERSION:2.0\r\nPRODID:-//GottCode//Kapow//EN\r\n");

		QByteArray uid_suffix;
		uid_suffix += '-';
		uid_suffix += QByteArray::number(QCoreApplication::applicationPid());
		uid_suffix += '@';
		uid_suffix += QHostInfo::localHostName().toUtf8();

		QString current;
		int dup = 0;
		int rows = m_data->rowCount();
		for (int row = 0; row < rows; ++row) {
			if (m_details->isRowHidden(row, QModelIndex())) {
				continue;
			}

			QString now = QDateTime::currentDateTimeUtc().toString("yyyyMMddThhmmssZ");
			if (current == now) {
				dup++;
			} else {
				current = now;
				dup = 0;
			}

			Session session = m_data->session(row);

			file.write("BEGIN:VEVENT\r\n");

			QByteArray uid("UID:Kapow-");
			uid += current.toUtf8();
			uid += QByteArray::number(dup);
			uid += uid_suffix;
			writeWrappedLine(uid, &file);

			QDateTime start(session.date(), session.start());
			file.write(QByteArray("DTSTART:") + start.toUTC().toString("yyyyMMddThhmmssZ").toUtf8() + QByteArray("\r\n"));
			QDateTime stop(session.date(), session.stop());
			file.write(QByteArray("DTEND:") + stop.toUTC().toString("yyyyMMddThhmmssZ").toUtf8() + QByteArray("\r\n"));

			if (!session.task().isEmpty()) {
				writeWrappedLine(QByteArray("SUMMARY:") + session.task().toUtf8(), &file);
			}

			file.write("END:VEVENT\r\n");
		}

		file.write("END:VCALENDAR\r\n");
		file.close();
	}
}

//-----------------------------------------------------------------------------

void Report::writeOutlookCsv(QString filename) const
{
	if (!filename.endsWith(".csv")) {
		filename.append(".csv");
	}
	QFile file(filename);
	if (file.open(QFile::WriteOnly)) {
		QTextStream stream(&file);
		stream << QLatin1String("\"Title\",\"Start Date\",\"Start Time\",\"End Date\",\"End Time\"\r\n");
		int rows = m_data->rowCount();
		for (int row = 0; row < rows; ++row) {
			if (m_details->isRowHidden(row, QModelIndex())) {
				continue;
			}

			Session session = m_data->session(row);
			stream << '"' << session.task() << QLatin1String("\",\"")
					<< session.date().toString("MM/dd/yy") << QLatin1String("\",\"")
					<< session.start().toString("hh:mm:ss AP") << QLatin1String("\",\"")
					<< session.date().toString("MM/dd/yy") << QLatin1String("\",\"")
					<< session.stop().toString("hh:mm:ss AP") << QLatin1String("\"\r\n");
		}
		file.close();
	}
}

//-----------------------------------------------------------------------------
