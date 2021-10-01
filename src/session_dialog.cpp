/*
	SPDX-FileCopyrightText: 2008-2012 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "session_dialog.h"

#include "session.h"
#include "settings.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QVBoxLayout>

/*****************************************************************************/

SessionDialog::SessionDialog(QWidget* parent)
: QDialog(parent) {
	setWindowTitle(tr("Add Session"));

	m_date = new QDateEdit(QDate::currentDate(), this);
	m_date->setCalendarPopup(true);

	QString format = QLocale().timeFormat(QLocale::LongFormat).contains("AP", Qt::CaseInsensitive) ? "h:mm:ss AP" : "HH:mm:ss";

	m_start = new QTimeEdit(QTime::currentTime(), this);
	m_start->setDisplayFormat(format);

	m_stop = new QTimeEdit(QTime::currentTime(), this);
	m_stop->setDisplayFormat(format);

	m_task = new QLineEdit(this);

	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

	QFormLayout* item_layout = new QFormLayout;
	item_layout->setContentsMargins(0, 0, 0, 0);
	item_layout->addRow(tr("Date:"), m_date);
	item_layout->addRow(tr("Start:"), m_start);
	item_layout->addRow(tr("Stop:"), m_stop);
	item_layout->addRow(tr("Task:"), m_task);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addLayout(item_layout);
	layout->addStretch();
	layout->addWidget(buttons);

	resize(Settings().value("SessionDialog/Size").toSize());
}

/*****************************************************************************/

void SessionDialog::setSession(const Session& session) {
	setWindowTitle(tr("Edit Session"));
	m_date->setDate(session.date());
	m_start->setTime(session.start());
	m_stop->setTime(session.stop());
	m_task->setText(session.task());
}

/*****************************************************************************/

void SessionDialog::accept() {
	m_session = Session(m_date->date(), m_start->time(), m_stop->time(), m_task->text(), false);
	QDialog::accept();
}

/*****************************************************************************/

void SessionDialog::hideEvent(QHideEvent* event) {
	Settings().setValue("SessionDialog/Size", size());
	QDialog::hideEvent(event);
}

/*****************************************************************************/
