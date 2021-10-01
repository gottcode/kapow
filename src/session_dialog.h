/*
	SPDX-FileCopyrightText: 2008 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef SESSION_DIALOG_H
#define SESSION_DIALOG_H

#include "session.h"

#include <QDialog>
#include <QDateEdit>
#include <QLineEdit>
#include <QTimeEdit>

class SessionDialog : public QDialog {
	Q_OBJECT
public:
	SessionDialog(QWidget* parent = 0);

	Session session() const {
		return m_session;
	}

	void setSession(const Session& session);

public:
	virtual void accept();

protected:
	virtual void hideEvent(QHideEvent* event);

private:
	Session m_session;
	QDateEdit* m_date;
	QTimeEdit* m_start;
	QTimeEdit* m_stop;
	QLineEdit* m_task;
};

#endif
