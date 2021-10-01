/*
	SPDX-FileCopyrightText: 2008 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KAPOW_SESSION_DIALOG_H
#define KAPOW_SESSION_DIALOG_H

#include "session.h"

#include <QDialog>
#include <QDateEdit>
#include <QLineEdit>
#include <QTimeEdit>

class SessionDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SessionDialog(QWidget* parent = nullptr);

	Session session() const
	{
		return m_session;
	}

	void setSession(const Session& session);

public:
	void accept() override;

protected:
	void hideEvent(QHideEvent* event) override;

private:
	Session m_session;
	QDateEdit* m_date;
	QTimeEdit* m_start;
	QTimeEdit* m_stop;
	QLineEdit* m_task;
};

#endif // KAPOW_SESSION_DIALOG_H
