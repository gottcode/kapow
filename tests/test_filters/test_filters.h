/*
	SPDX-FileCopyrightText: 2013 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KAPOW_TEST_FILTERS_H
#define KAPOW_TEST_FILTERS_H

class SessionModel;

#include <QObject>

class TestFilters : public QObject
{
	Q_OBJECT

private slots:
	void checkFilters_data();
	void checkFilters();
	void addSessions_data();
	void addSessions();
	void editSessions_data();
	void editSessions();
	void removeSessions_data();
	void removeSessions();
	void unbilledFilter_data();
	void unbilledFilter();
	void mapUnbilled_data();
	void mapUnbilled();
};

#endif // KAPOW_TEST_FILTERS_H
