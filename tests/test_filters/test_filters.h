/***********************************************************************
 *
 * Copyright (C) 2013 Graeme Gott <graeme@gottcode.org>
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

#endif
