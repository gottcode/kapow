/***********************************************************************
 *
 * Copyright (C) 2013-2020 Graeme Gott <graeme@gottcode.org>
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

#ifndef KAPOW_TEST_SESSIONS_H
#define KAPOW_TEST_SESSIONS_H

#include <QObject>

class TestSessions : public QObject
{
	Q_OBJECT

private slots:
	void addSessions_data();
	void addSessions();
	void addConflictingSessions_data();
	void addConflictingSessions();
	void addMultipleSessions_data();
	void addMultipleSessions();
	void addSessionsFixConflict_data();
	void addSessionsFixConflict();
	void addSessionsHasConflict_data();
	void addSessionsHasConflict();
	void addSessionsMaximumDateTime_data();
	void addSessionsMaximumDateTime();
	void billSessions_data();
	void billSessions();
	void editSessions_data();
	void editSessions();
	void editSessionsMaximumDateTime_data();
	void editSessionsMaximumDateTime();
	void removeSessions_data();
	void removeSessions();
};

#endif
