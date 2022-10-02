/*
	SPDX-FileCopyrightText: 2013-2020 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KAPOW_TEST_SESSIONS_H
#define KAPOW_TEST_SESSIONS_H

#include <QObject>

class TestSessions : public QObject
{
	Q_OBJECT

private Q_SLOTS:
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

#endif // KAPOW_TEST_SESSIONS_H
