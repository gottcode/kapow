/*
	SPDX-FileCopyrightText: 2009-2020 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "project.h"

#include "filter_model.h"
#include "session_model.h"

#include <QApplication>
#include <QMessageBox>
#include <QXmlStreamWriter>

//-----------------------------------------------------------------------------

Project::Project(QTreeWidget* parent, const QString& project)
	: QTreeWidgetItem(parent, QStringList(project))
{
	init();
}

//-----------------------------------------------------------------------------

Project::Project(QTreeWidgetItem* parent, const QString& project)
	: QTreeWidgetItem(parent, QStringList(project))
{
	init();
}

//-----------------------------------------------------------------------------

bool Project::isActive() const
{
	if (m_active) {
		return true;
	}

	int count = childCount();
	for (int i = 0; i < count; ++i) {
		Project* project = dynamic_cast<Project*>(child(i));
		if (project && project->isActive()) {
			return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------

void Project::setDecimalTotals(bool decimals)
{
	m_model->setDecimalTotals(decimals);
	int count = childCount();
	for (int i = 0; i < count; ++i) {
		Project* project = dynamic_cast<Project*>(child(i));
		if (project) {
			project->setDecimalTotals(decimals);
		}
	}
}

//-----------------------------------------------------------------------------

void Project::setScrollValue(int value)
{
	m_scroll_value = value;
}

//-----------------------------------------------------------------------------

QString Project::time() const
{
	return text(1);
}

//-----------------------------------------------------------------------------

void Project::toXml(QXmlStreamWriter& xml) const
{
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

//-----------------------------------------------------------------------------

bool Project::start(const QDateTime& current)
{
	// Prevent starting timer inside of existing session
	if (m_model->hasConflict(current)) {
		return false;
	}

	m_start_time = current;
	m_active = true;
	m_model->setMaximumDateTime(m_start_time);
	setText(1, "00:00:00");
	billedStatusChanged(false);

	return true;
}

//-----------------------------------------------------------------------------

bool Project::stop(QDateTime current)
{
	// Prevent ending timer inside of or after existing session
	if (current.isValid()) {
		m_model->fixConflict(m_start_time, current);
	}

	bool success = true;
	m_active = false;
	m_model->setMaximumDateTime(QDateTime());
	if (current.isValid()) {
		success = m_model->add(m_start_time, current, m_task);
	} else {
		billedStatusChanged(m_model->isBilled(m_model->rowCount() - 2));
	}
	setText(1, "");

	return success;
}

//-----------------------------------------------------------------------------

void Project::setTask(const QString& task)
{
	m_task = task;
}

//-----------------------------------------------------------------------------

void Project::updateTime(const QDateTime& current)
{
	if (m_active) {
		QTime convert(0, 0, 0);
		convert = convert.addSecs(m_start_time.secsTo(current));
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

//-----------------------------------------------------------------------------

void Project::billedStatusChanged(bool billed)
{
	setForeground(0, QApplication::palette().color((billed && !m_active) ? QPalette::Disabled : QPalette::Normal, QPalette::Text));
}

//-----------------------------------------------------------------------------

void Project::init()
{
	setFlags(flags() | Qt::ItemIsEditable);
	setTextAlignment(1, Qt::AlignRight | Qt::AlignVCenter);

	m_model = new SessionModel(this);
	connect(m_model, SIGNAL(billedStatusChanged(bool)), this, SLOT(billedStatusChanged(bool)));
	m_filter_model = new FilterModel(m_model, treeWidget());
	m_active = false;
	m_scroll_value = -1;
}

//-----------------------------------------------------------------------------
