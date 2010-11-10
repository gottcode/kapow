/***********************************************************************
 *
 * Copyright (C) 2008 Graeme Gott <graeme@gottcode.org>
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
