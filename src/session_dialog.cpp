/***********************************************************************
 *
 * Copyright (C) 2008, 2011, 2012 Graeme Gott <graeme@gottcode.org>
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
