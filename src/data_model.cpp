/***********************************************************************
 *
 * Copyright (C) 2008, 2009, 2010 Graeme Gott <graeme@gottcode.org>
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

#include "data_model.h"

#include "session.h"

#include <QApplication>
#include <QMessageBox>
#include <QPalette>

/*****************************************************************************/

DataModel::DataModel(QObject* parent)
: QAbstractTableModel(parent), m_decimals(true) {
}

/*****************************************************************************/

bool DataModel::add(const QDateTime& start, const QDateTime& stop) {
	if (stop < start) {
		return false;
	}

	if (start.date() == stop.date()) {
		return add(Session(start.date(), start.time(), stop.time(), QString(), false));
	} else {
		return add(Session(start.date(), start.time(), QTime(23, 59, 59), QString(), false)) && add(Session(stop.date(), QTime(0, 0, 0), stop.time(), QString(), false));
	}
}

/*****************************************************************************/

bool DataModel::add(const Session& session) {
	if (!session.isValid()) {
		return false;
	}

	// Find session position
	int pos = 0;
	foreach (const Session& current, m_data) {
		if (session.date() < current.date() || (session.date() == current.date() && session.start() < current.start())) {
			break;
		}
		pos++;
	}

	// Prevent intersecting sessions
	if (pos > 0) {
		Session current = m_data.at(pos - 1);
		if (QDateTime(session.date(), session.start()) < QDateTime(current.date(), current.stop())) {
			QMessageBox::warning(0, tr("Error"), tr("Session conflicts with other sessions."));
			return false;
		}
	}
	if (pos < m_data.count()) {
		Session current = m_data.at(pos);
		if (QDateTime(session.date(), session.stop()) > QDateTime(current.date(), current.start())) {
			QMessageBox::warning(0, tr("Error"), tr("Session conflicts with other sessions."));
			return false;
		}
	}

	// Insert session
	beginInsertRows(QModelIndex(), pos, pos);
	m_data.insert(pos, session);
	endInsertRows();

	// Update successive billed sessions
	for (int i = 0; i < m_billed.count(); ++i) {
		int& session = m_billed[i];
		if (session >= pos) {
			session++;
		}
	}

	// Set billed status
	if (session.isBilled()) {
		setBilled(pos, true);
	}

	// Increase totals for sessions
	updateTotals();

	return true;
}

/*****************************************************************************/

bool DataModel::edit(int pos, const Session& session) {
	if (!session.isValid()) {
		return false;
	}

	Q_ASSERT(pos < m_data.count());
	Session current = m_data.at(pos);

	// Replace session
	remove(pos);
	if (add(Session(session.date(), session.start(), session.stop(), session.task(), current.isBilled())) == false) {
		add(current);
		return false;
	} else {
		return true;
	}
}

/*****************************************************************************/

bool DataModel::remove(int pos) {
	if (pos >= m_data.count()) {
		return false;
	}

	// Remove session
	beginRemoveRows(QModelIndex(), pos, pos);
	m_data.removeAt(pos);
	endRemoveRows();

	// Update successive billed sessions
	m_billed.removeAll(pos);
	for (int i = 0; i < m_billed.count(); ++i) {
		int& session = m_billed[i];
		if (session > pos) {
			session--;
		}
	}

	// Increase totals for sessions
	updateTotals();

	return true;
}

/*****************************************************************************/

void DataModel::setBilled(int pos, bool billed) {
	Q_ASSERT(pos < m_data.count());

	if (!billed) {
		m_billed.removeAll(pos);
	} else {
		Q_ASSERT(!m_billed.contains(pos));
		m_billed.append(pos);
		qSort(m_billed);
	}
	m_data[pos].setBilled(billed);

	updateTotals();
}

/*****************************************************************************/

void DataModel::setDecimalTotals(bool decimals) {
	m_decimals = decimals;
	emit dataChanged(index(0, 0), index(rowCount(), columnCount()));
}

/*****************************************************************************/

void DataModel::toXml(QXmlStreamWriter& xml) const {
	foreach (const Session& session, m_data) {
		session.toXml(xml);
	}
}

/*****************************************************************************/

int DataModel::rowCount(const QModelIndex& parent) const {
	if (!parent.isValid()) {
		return m_data.count() + 1;
	} else {
		return 0;
	}
}

/*****************************************************************************/

int DataModel::columnCount(const QModelIndex& parent) const {
	if (!parent.isValid()) {
		return 10;
	} else {
		return 0;
	}
}

/*****************************************************************************/

QVariant DataModel::data(const QModelIndex& index, int role) const {
	Q_ASSERT(index.isValid());
	QVariant result;

	int pos = index.row();
	if (pos == m_data.count()) {

		QFont font;
		font.setWeight(QFont::Bold);

		pos--;
		Session session = m_data.value(pos);
		if (session.isBilled()) {
			session = Session();
		}

		switch (role) {
		case Qt::DisplayRole:
			switch(index.column()) {
			case 0:
				return tr("Total");
			case 1:
			case 2:
			case 3:
				return QString();
			case 4:
				return session.total(Session::Total, m_decimals);
			default:
				break;
			}
			break;

		case Qt::ToolTipRole:
			if (index.column() == 4) {
				return session.total(Session::Total, !m_decimals, true);
			}
			break;

		case Qt::TextAlignmentRole:
			if (index.column() == 0) {
				return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
			}
			break;

		case Qt::FontRole:
			return font;

		case Qt::CheckStateRole:
			return QVariant();

		default:
			break;
		}
	}

	Session session = m_data.value(pos);
	switch (role) {
	case Qt::DisplayRole:
		switch (index.column()) {
		case 0:
			result = session.date().toString(Qt::SystemLocaleShortDate);
			break;
		case 1:
			result = session.start().toString(Qt::SystemLocaleShortDate);
			break;
		case 2:
			result = session.stop().toString(Qt::SystemLocaleShortDate);
			break;
		case 3:
			result = session.task();
			break;
		case 4:
			result = session.total(Session::Duration, m_decimals);
			break;
		case 5:
			result = session.total(Session::Daily, m_decimals);
			break;
		case 6:
			result = session.total(Session::Weekly, m_decimals);
			break;
		case 7:
			result = session.total(Session::Monthly, m_decimals);
			break;
		case 8:
			result = session.total(Session::Total, m_decimals);
			break;
		default:
			break;
		}
		break;

	case Qt::ToolTipRole:
		switch (index.column()) {
		case 4:
			result = session.total(Session::Duration, !m_decimals, true);
			break;
		case 5:
			result = session.total(Session::Daily, !m_decimals, true);
			break;
		case 6:
			result = session.total(Session::Weekly, !m_decimals, true);
			break;
		case 7:
			result = session.total(Session::Monthly, !m_decimals, true);
			break;
		case 8:
			result = session.total(Session::Total, !m_decimals, true);
			break;
		default:
			break;
		}
		break;

	case Qt::TextAlignmentRole:
		if (index.column() != 3) {
			result = static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
		}
		break;

	case Qt::ForegroundRole:
		if (isBilled(index.row())) {
			result = QApplication::palette().color(QPalette::Disabled, QPalette::Text);
		}
		break;

	case Qt::CheckStateRole:
		if (index.column() == 9) {
			result = m_data.at(index.row()).isBilled() ? Qt::Checked : Qt::Unchecked;
		}
		break;

	case Qt::EditRole:
		Q_ASSERT(!isBilled(index.row()));
		switch (index.column()) {
		case 0:
			result = session.date();
			break;
		case 1:
			result = session.start();
			break;
		case 2:
			result = session.stop();
			break;
		case 3:
			result = session.task();
			break;
		default:
			break;
		}
		break;

	default:
		break;
	}
	return result;
}

/*****************************************************************************/

Qt::ItemFlags DataModel::flags(const QModelIndex& index) const {
	if (index.column() == 9) {
		return QAbstractTableModel::flags(index) |  Qt::ItemIsUserCheckable;
	} else if (isBilled(index.row()) || (index.row() == m_data.count()) || (index.column() > 3 && index.column() < 9)) {
		return QAbstractTableModel::flags(index);
	} else {
		return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
	}
}

/*****************************************************************************/

QVariant DataModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (orientation != Qt::Horizontal) {
		return QVariant();
	}

	if (role == Qt::DisplayRole) {
		switch (section) {
		case 0:
			return tr("Date");
		case 1:
			return tr("Start");
		case 2:
			return tr("Stop");
		case 3:
			return tr("Task");
		case 4:
			return tr("Hours");
		case 5:
			return tr("Daily");
		case 6:
			return tr("Weekly");
		case 7:
			return tr("Monthly");
		case 8:
			return tr("Total");
		case 9:
			return tr("Billed");
		default:
			return QVariant();
		}
	} else {
		return QVariant();
	}
}

/*****************************************************************************/

bool DataModel::setData(const QModelIndex& index, const QVariant& value, int role) {
	if (!isBilled(index.row()) && role == Qt::EditRole) {
		Session session = m_data.at(index.row());
		QDate date = session.date();
		QTime start = session.start();
		QTime stop = session.stop();
		QString task = session.task();
		switch (index.column()) {
		case 0:
			date = value.toDate();
			break;
		case 1:
			start = value.toTime();
			break;
		case 2:
			stop = value.toTime();
			break;
		case 3:
			task = value.toString();
			break;
		default:
			break;
		}

		if (edit(index.row(), Session(date, start, stop, task, false))) {
			return true;
		} else {
			QMessageBox::warning(0, tr("Error"), tr("Session conflicts with other sessions."));
			return false;
		}
	} else if (index.column() == 9 && role == Qt::CheckStateRole) {
		setBilled(index.row(), value.toInt() == Qt::Checked);
		return true;
	} else {
		return QAbstractTableModel::setData(index, value, role);
	}
}

/*****************************************************************************/

void DataModel::updateTotals() {
	Session current, previous;
	for (int i = 0; i < m_data.count(); ++i) {
		current = m_data.at(i);
		current.updateTotals(previous);
		previous = current;
	}
	emit dataChanged(index(0, 0), index(rowCount(), columnCount()));
}

/*****************************************************************************/
