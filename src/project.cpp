/***********************************************************************
 *
 * Copyright (C) 2009, 2010, 2011, 2012, 2013 Graeme Gott <graeme@gottcode.org>
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

#include "project.h"

#include "filter_model.h"
#include "session_model.h"

#include <QApplication>
#include <QMessageBox>
#include <QXmlStreamWriter>

/*****************************************************************************/

Project::Project(QTreeWidget* parent, const QString& project)
: QTreeWidgetItem(parent, QStringList(project)) {
	init();
}

/*****************************************************************************/

Project::Project(QTreeWidgetItem* parent, const QString& project)
: QTreeWidgetItem(parent, QStringList(project)) {
	init();
}

/*****************************************************************************/

void Project::setDecimalTotals(bool decimals) {
	m_model->setDecimalTotals(decimals);
	int count = childCount();
	for (int i = 0; i < count; ++i) {
		Project* project = dynamic_cast<Project*>(child(i));
		if (project) {
			project->setDecimalTotals(decimals);
		}
	}
}

/*****************************************************************************/

void Project::setScrollValue(int value) {
	m_scroll_value = value;
}

/*****************************************************************************/

QString Project::time() const {
	return text(1);
}

/*****************************************************************************/

void Project::toXml(QXmlStreamWriter& xml) const {
	xml.writeStartElement("project");
	xml.writeAttribute("name", text(0));
	if (isExpanded()) {
		xml.writeAttribute("expanded", "1");
	}
	if (this == treeWidget()->currentItem()) {
		xml.writeAttribute("current", "1");
	}
	xml.writeAttribute("filter", QString::number(m_filter_model->type()));
	if (m_active) {
		xml.writeEmptyElement("autosave");
		xml.writeAttribute("start", m_start_time.toString(Qt::ISODate));
		xml.writeAttribute("stop", QDateTime::currentDateTime().toString(Qt::ISODate));
		xml.writeAttribute("note", m_task);
	}

	m_model->toXml(xml);

	int count = childCount();
	for (int i = 0; i < count; ++i) {
		Project* project = dynamic_cast<Project*>(child(i));
		if (project) {
			project->toXml(xml);
		}
	}

	xml.writeEndElement();
}

/*****************************************************************************/

void Project::start(const QDateTime& current) {
	m_start_time = current;
	m_active = true;
	setText(1, "00:00:00");
	billedStatusChanged(false);
}

/*****************************************************************************/

void Project::stop(const QDateTime& current, bool* ok) {
	bool success = true;
	m_active = false;
	if (current.isValid()) {
		success = m_model->add(m_start_time, current, m_task);
	} else {
		billedStatusChanged(m_model->isBilled(m_model->rowCount() - 2));
	}
	setText(1, "");
	if (ok) {
		*ok = success;
	}
}

/*****************************************************************************/

void Project::setTask(const QString& task) {
	m_task = task;
}

/*****************************************************************************/

void Project::updateTime(const QDateTime& current) {
	if (m_active) {
		int elapsed = current.toTime_t() - m_start_time.toTime_t();
		QTime convert(0, 0, 0);
		convert = convert.addSecs(elapsed);
		setText(1, convert.toString("hh:mm:ss"));
	}

	int count = childCount();
	for (int i = 0; i < count; ++i) {
		Project* project = dynamic_cast<Project*>(child(i));
		if (project) {
			project->updateTime(current);
		}
	}
}

/*****************************************************************************/

void Project::billedStatusChanged(bool billed) {
	setForeground(0, QApplication::palette().color((billed && !m_active) ? QPalette::Disabled : QPalette::Normal, QPalette::Text));
}

/*****************************************************************************/

void Project::init() {
	setFlags(flags() | Qt::ItemIsEditable);
	setTextAlignment(1, Qt::AlignRight | Qt::AlignVCenter);

	m_model = new SessionModel(this);
	connect(m_model, SIGNAL(billedStatusChanged(bool)), this, SLOT(billedStatusChanged(bool)));
	m_filter_model = new FilterModel(m_model, treeWidget());
	m_active = false;
	m_scroll_value = -1;
}

/*****************************************************************************/
