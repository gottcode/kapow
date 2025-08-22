/*
	SPDX-FileCopyrightText: 2008-2025 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "session_model.h"

#include "session.h"

#include <QLocale>

#include <algorithm>
#include <climits>

//-----------------------------------------------------------------------------

static void sortNewestToOldest(QList<int>& rows)
{
	std::sort(rows.begin(), rows.end(), std::greater<int>());
}

//-----------------------------------------------------------------------------

SessionModel::SessionModel(QObject* parent)
	: QAbstractItemModel(parent)
	, m_decimals(true)
	, m_loaded(true)
{
}

//-----------------------------------------------------------------------------

bool SessionModel::isLastBilled(int pos) const
{
	if (m_billed.isEmpty() || pos > m_billed.last()) {
		return false;
	}

	const int size = m_billed.size();
	if ((size > 1) && (pos <= m_billed[size - 2])) {
		return false;
	} else {
		return true;
	}
}

//-----------------------------------------------------------------------------

void SessionModel::fixConflict(const QDateTime& current_start, QDateTime& current_stop) const
{
	for (int pos = m_data.count(); pos > 0; --pos) {
		const Session& session = m_data.at(pos - 1);
		const QDateTime start(session.date(), session.start());
		if ((current_start < start) && (current_stop >= start)) {
			current_stop = start.addSecs(-1);
		} else {
			break;
		}
	}
}

//-----------------------------------------------------------------------------

bool SessionModel::hasConflict(const QDateTime& current) const
{
	for (int pos = m_data.count(); pos > 0; --pos) {
		const Session& session = m_data.at(pos - 1);
		if (session.date() == current.date()) {
			if ((session.stop() >= current.time()) && (session.start() <= current.time())) {
				return true;
			} else if (session.stop() < current.time()) {
				break;
			}
		} else if (session.date() < current.date()) {
			break;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------

void SessionModel::beginLoad()
{
	m_loaded = false;
}

//-----------------------------------------------------------------------------

void SessionModel::endLoad()
{
	m_loaded = true;

	// Store billed status
	for (int pos = 0, count = m_data.count(); pos < count; ++pos) {
		const Session& session = m_data.at(pos);
		if (session.isBilled()) {
			m_billed.append(pos);
		}
	}

	// Find totals for sessions
	updateTotals();
}

//-----------------------------------------------------------------------------

bool SessionModel::add(const QDateTime& start, const QDateTime& stop, const QString& task)
{
	// Prevent adding invalid sessions
	if (stop < start) {
		return false;
	}

	// Add single session if it does not cross midnight
	if (start.date() == stop.date()) {
		return add(Session(start.date(), start.time(), stop.time(), task, false));
	}

	QList<Session> sessions;

	// Split session at first midnight
	sessions.append(Session(start.date(), start.time(), QTime(23, 59, 59), task, false));
	if (findPosition(sessions.last()) == -1) {
		return false;
	}

	// Split full days between start date and stop date
	QDate date = start.date();
	while ((date = date.addDays(1)) != stop.date()) {
		sessions.append(Session(date, QTime(0, 0, 0), QTime(23, 59, 59), task, false));
		if (findPosition(sessions.last()) == -1) {
			return false;
		}
	}

	// Split session from last midnight
	sessions.append(Session(stop.date(), QTime(0, 0, 0), stop.time(), task, false));
	if (findPosition(sessions.last()) == -1) {
		return false;
	}

	// Add the split sessions
	for (const Session& session : std::as_const(sessions)) {
		add(session);
	}

	return true;
}

//-----------------------------------------------------------------------------

bool SessionModel::add(const Session& session)
{
	// Find position of session
	const int pos = findPosition(session);
	if (pos == -1) {
		return false;
	}

	// Insert session
	if (!m_loaded) {
		m_data.insert(pos, session);
		return true;
	}
	beginInsertRows(QModelIndex(), pos, pos);
	m_data.insert(pos, session);
	endInsertRows();

	// Set billed status
	if (session.isBilled()) {
		setBilled(pos, true);
	}

	// Increase totals for sessions
	updateTotals();

	return true;
}

//-----------------------------------------------------------------------------

bool SessionModel::edit(int pos, const Session& session)
{
	if (!session.isValid() || isBilled(pos)) {
		return false;
	}

	Q_ASSERT(pos < m_data.count());
	Session current = m_data.at(pos);

	// Replace session
	remove(pos);
	if (add(Session(session.date(), session.start(), session.stop(), session.task(), current.isBilled())) == false) {
		const QDateTime temp = m_max_datetime;
		m_max_datetime = QDateTime();
		add(current);
		m_max_datetime = temp;
		return false;
	} else {
		return true;
	}
}

//-----------------------------------------------------------------------------

bool SessionModel::remove(int pos)
{
	if (!canRemove(pos)) {
		return false;
	}

	// Remove session
	beginRemoveRows(QModelIndex(), pos, pos);
	m_data.removeAt(pos);
	endRemoveRows();

	// Increase totals for sessions
	updateTotals();

	return true;
}

//-----------------------------------------------------------------------------

int SessionModel::remove(QList<int> rows)
{
	sortNewestToOldest(rows);

	int result = 0;
	for (int row : std::as_const(rows)) {
		result += remove(row);
	}

	return result;
}

//-----------------------------------------------------------------------------

bool SessionModel::take(SessionModel* model, int pos)
{
	if (!model || !model->canRemove(pos)) {
		return false;
	}

	// Attempt to add session
	if (!add(model->session(pos))) {
		return false;
	}

	// Remove session from source
	model->remove(pos);

	return true;
}

//-----------------------------------------------------------------------------

bool SessionModel::take(SessionModel* model, QList<int> rows)
{
	if (!model) {
		return false;
	}

	sortNewestToOldest(rows);

	// Fetch sessions from source if they can be moved
	QList<Session> sessions;
	for (int pos : std::as_const(rows)) {
		if (!model->canRemove(pos)) {
			return false;
		}

		sessions.append(model->session(pos));
		if (findPosition(sessions.last()) == -1) {
			return false;
		}
	}

	// Add sessions
	for (const Session& session : std::as_const(sessions)) {
		add(session);
	}

	// Remove sessions from source
	for (int pos : std::as_const(rows)) {
		model->remove(pos);
	}

	return true;
}

//-----------------------------------------------------------------------------

void SessionModel::setBilled(int pos, bool billed)
{
	Q_ASSERT(pos < m_data.count());

	if (!billed) {
		m_billed.removeAll(pos);
	} else {
		Q_ASSERT(!m_billed.contains(pos));
		m_billed.append(pos);
		std::sort(m_billed.begin(), m_billed.end());
	}
	m_data[pos].setBilled(billed);

	updateTotals();
}

//-----------------------------------------------------------------------------

void SessionModel::setDecimalTotals(bool decimals)
{
	m_decimals = decimals;
	Q_EMIT dataChanged(index(0, 0), index(rowCount(), columnCount()));
}

//-----------------------------------------------------------------------------

void SessionModel::setMaximumDateTime(const QDateTime& max)
{
	m_max_datetime = max;
}

//-----------------------------------------------------------------------------

void SessionModel::toXml(QXmlStreamWriter& xml) const
{
	for (const Session& session : m_data) {
		session.toXml(xml);
	}
}

//-----------------------------------------------------------------------------

int SessionModel::rowCount(const QModelIndex& parent) const
{
	if (!parent.isValid()) {
		return m_data.count() + 1;
	} else if ((parent.internalId() == UINT_MAX) && m_billed.contains(parent.row())) {
		return 1;
	} else {
		return 0;
	}
}

//-----------------------------------------------------------------------------

int SessionModel::columnCount(const QModelIndex&) const
{
	return 10;
}

//-----------------------------------------------------------------------------

QVariant SessionModel::data(const QModelIndex& index, int role) const
{
	Q_ASSERT(index.isValid());
	QVariant result;

	int pos = index.row();
	if (index.parent().isValid() || (pos == m_data.count())) {

		Session session;
		if (!index.parent().isValid()) {
			--pos;
			session = m_data.value(pos);
			if (session.isBilled()) {
				session = Session();
			}
		} else {
			session = m_data.value(index.parent().row());
		}

		switch (role) {
		case Qt::DisplayRole:
			switch(index.column()) {
			case 0:
				result = tr("Total");
				break;
			case 1:
			case 2:
			case 3:
				result = QString();
				break;
			case 4:
				result = session.total(Session::Total, m_decimals);
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
			switch(index.column()) {
			case 4:
				result = session.total(Session::Total, !m_decimals, true);
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
			if (index.column() == 0) {
				result = static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
			} else if (index.column() != 3) {
				result = static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
			}
			break;

		default:
			break;
		}
		return result;
	}

	QLocale locale;
	Session session = m_data.value(pos);
	switch (role) {
	case Qt::DisplayRole:
		switch (index.column()) {
		case 0:
			result = locale.toString(session.date(), QLocale::ShortFormat);
			break;
		case 1:
			result = locale.toString(session.start(), QLocale::ShortFormat);
			break;
		case 2:
			result = locale.toString(session.stop(), QLocale::ShortFormat);
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
		case 3:
			result = session.task();
			break;
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
		} else {
			result = static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
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

	case Qt::UserRole:
		result = isBilled(index.row());
		break;

	default:
		break;
	}
	return result;
}

//-----------------------------------------------------------------------------

Qt::ItemFlags SessionModel::flags(const QModelIndex& index) const
{
	Qt::ItemFlags result = QAbstractItemModel::flags(index);
	if (index.parent().isValid() || (index.row() == m_data.count())) {
		result = Qt::ItemIsEnabled;
	} else if (index.column() == 9) {
		if (!isBilled(index.row()) || (index.row() == m_billed.last())) {
			result |= Qt::ItemIsUserCheckable;
		}
	} else if (!isBilled(index.row()) && (index.column() <= 3)) {
		result |= Qt::ItemIsEditable;
	}
	return result;
}

//-----------------------------------------------------------------------------

QVariant SessionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
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
			return tr("Report");
		default:
			return QVariant();
		}
	} else if (role == Qt::TextAlignmentRole) {
		return Qt::AlignCenter;
	} else {
		return QVariant();
	}
}

//-----------------------------------------------------------------------------

QModelIndex SessionModel::index(int row, int column, const QModelIndex& parent) const
{
	if (!hasIndex(row, column, parent)) {
		return QModelIndex();
	}

	if (!parent.isValid()) {
		return createIndex(row, column, UINT_MAX);
	} else {
		return createIndex(row, column, parent.row());
	}
}

//-----------------------------------------------------------------------------

QModelIndex SessionModel::parent(const QModelIndex& child) const
{
	if (!child.isValid()) {
		return QModelIndex();
	}

	quintptr row = child.internalId();
	if (row == UINT_MAX) {
		return QModelIndex();
	} else {
		return createIndex(row, 0, UINT_MAX);
	}
}

//-----------------------------------------------------------------------------

bool SessionModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
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
			return false;
		}
	} else if (index.column() == 9 && role == Qt::CheckStateRole) {
		setBilled(index.row(), value.toInt() == Qt::Checked);
		return true;
	} else {
		return QAbstractItemModel::setData(index, value, role);
	}
}

//-----------------------------------------------------------------------------

int SessionModel::findPosition(const Session& session) const
{
	if (!session.isValid()) {
		return -1;
	}

	// Prevent intersecting current running timer
	if (m_max_datetime.isValid()) {
		if ((QDateTime(session.date(), session.start()) >= m_max_datetime) || (QDateTime(session.date(), session.stop()) >= m_max_datetime)) {
			return -1;
		}
	}

	// Find session position
	int pos = 0;
	for (pos = m_data.count(); pos > 0; --pos) {
		const Session& current = m_data.at(pos - 1);
		if (session.date() > current.date() || (session.date() == current.date() && session.stop() > current.stop())) {
			break;
		}
	}
	pos = std::max(pos, 0);

	// Prevent intersecting sessions
	if (pos > 0) {
		const Session& current = m_data.at(pos - 1);
		if (QDateTime(session.date(), session.start()) < QDateTime(current.date(), current.stop())) {
			return -1;
		}
	}
	if (pos < m_data.count()) {
		const Session& current = m_data.at(pos);
		if (QDateTime(session.date(), session.stop()) > QDateTime(current.date(), current.start())) {
			return -1;
		}
	}

	// Prevent adding to billed
	if (isBilled(pos)) {
		return -1;
	}

	return pos;
}

//-----------------------------------------------------------------------------

void SessionModel::updateTotals()
{
	Session current, previous;
	for (int i = 0; i < m_data.count(); ++i) {
		current = m_data.at(i);
		current.updateTotals(previous);
		previous = current;
	}
	Q_EMIT dataChanged(index(0, 0), index(rowCount(), columnCount()));
	Q_EMIT billedStatusChanged(current.isBilled());
}

//-----------------------------------------------------------------------------
