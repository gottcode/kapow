/*
	SPDX-FileCopyrightText: 2008-2025 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KAPOW_SESSION_MODEL_H
#define KAPOW_SESSION_MODEL_H

#include "session.h"

#include <QAbstractItemModel>
#include <QXmlStreamWriter>

class SessionModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	explicit SessionModel(QObject* parent = nullptr);

	QList<int> billedRows() const
	{
		return m_billed;
	}

	bool canBill() const
	{
		return !m_data.isEmpty() && !isBilled(m_data.size() - 1);
	}

	bool isBilled(int pos) const
	{
		return (!m_billed.isEmpty() && pos <= m_billed.last());
	}

	bool isLastBilled(int pos) const;

	void fixConflict(const QDateTime& current_start, QDateTime& current_stop) const;

	bool hasConflict(const QDateTime& current) const;

	Session session(int pos) const
	{
		return m_data.value(pos);
	}

	void beginLoad();
	void endLoad();

	bool add(const QDateTime& start, const QDateTime& stop, const QString& task);
	bool add(const Session& session);
	bool edit(int row, const Session& session);
	bool remove(int row);
	int remove(QList<int> rows);
	bool take(SessionModel* model, int row);
	bool take(SessionModel* model, QList<int> rows);
	void setBilled(int row, bool billed);
	void setDecimalTotals(bool decimals);
	void setMaximumDateTime(const QDateTime& max);
	void toXml(QXmlStreamWriter& xml) const;

	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	Qt::ItemFlags flags(const QModelIndex& index) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex& child) const override;
	bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

Q_SIGNALS:
	void billedStatusChanged(bool billed);

private:
	bool canRemove(int pos) const
	{
		return (pos < m_data.count()) && !isBilled(pos);
	}

	int findPosition(const Session& session) const;
	void updateTotals();

private:
	QList<Session> m_data;
	QList<int> m_billed;
	QDateTime m_max_datetime;
	bool m_decimals;
	bool m_loaded;
};

#endif // KAPOW_SESSION_MODEL_H
