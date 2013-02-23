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

#include "test_filters.h"

#include "filter_model.h"
#include "session.h"
#include "session_model.h"
Q_DECLARE_METATYPE(Session)

#include <QTest>

//-----------------------------------------------------------------------------

void TestFilters::checkFilters_data()
{
	QTest::addColumn<QVariantList>("sessions");
	QTest::addColumn<int>("type");
	QTest::addColumn<QVariantList>("result");

	QDate today = QDate::currentDate();
	QTest::newRow("Filter for billed")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true))
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		)
		<< int(FilterModel::Unbilled)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		);

	QDate before = today.addYears(-1);
	QTest::newRow("Filter for this year")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(before, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(before, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		)
		<< int(FilterModel::ThisYear)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		);

	before = today.addMonths(-1);
	QTest::newRow("Filter for this month")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(before, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(before, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		)
		<< int(FilterModel::ThisMonth)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		);

	before = today.addDays(-8);
	QTest::newRow("Filter for this week")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(before, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(before, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		)
		<< int(FilterModel::ThisWeek)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		);
}

void TestFilters::checkFilters()
{
	SessionModel session_model;

	QFETCH(QVariantList, sessions);
	for (int i = 0; i < sessions.count(); ++i) {
		session_model.add(sessions.at(i).value<Session>());
	}

	FilterModel model(&session_model);
	QFETCH(int, type);
	model.setType(type);

	QFETCH(QVariantList, result);
	QCOMPARE(model.rowCount() - 1, result.count());
	for (int i = 0; i < result.count(); ++i) {
		int row = model.mapToSource(model.index(i, 0)).row();
		QCOMPARE(session_model.session(row), result.at(i).value<Session>());
	}
}

//-----------------------------------------------------------------------------

void TestFilters::addSessions_data()
{
	QTest::addColumn<QVariantList>("start_sessions");
	QTest::addColumn<int>("type");
	QTest::addColumn<QVariantList>("add_sessions");
	QTest::addColumn<QVariantList>("result");

	QDate today = QDate::currentDate();
	QTest::newRow("Filtered by billed")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		)
		<< int(FilterModel::Unbilled)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true))
		)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		);
	QTest::newRow("Unfiltered by billed")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true))
		)
		<< int(FilterModel::Unbilled)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		);


	QDate before = today.addYears(-1);
	QTest::newRow("Filtered by this year")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		)
		<< int(FilterModel::ThisYear)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(before, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(before, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
		)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		);
	QTest::newRow("Unfiltered by this year")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		)
		<< int(FilterModel::ThisYear)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
		)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		);

	before = today.addMonths(-1);
	QTest::newRow("Filtered by this month")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		)
		<< int(FilterModel::ThisMonth)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(before, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(before, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
		)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		);
	QTest::newRow("Unfiltered by this month")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		)
		<< int(FilterModel::ThisMonth)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
		)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		);

	before = today.addDays(-8);
	QTest::newRow("Filtered by this week")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		)
		<< int(FilterModel::ThisWeek)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(before, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(before, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
		)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		);
	QTest::newRow("Unfiltered by this week")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		)
		<< int(FilterModel::ThisWeek)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
		)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		);
}

//-----------------------------------------------------------------------------

void TestFilters::addSessions()
{
	SessionModel session_model;

	QFETCH(QVariantList, start_sessions);
	for (int i = 0; i < start_sessions.count(); ++i) {
		session_model.add(start_sessions.at(i).value<Session>());
	}

	FilterModel model(&session_model);
	QFETCH(int, type);
	model.setType(type);

	QFETCH(QVariantList, add_sessions);
	for (int i = 0; i < add_sessions.count(); ++i) {
		session_model.add(add_sessions.at(i).value<Session>());
	}

	QFETCH(QVariantList, result);
	QCOMPARE(model.rowCount() - 1, result.count());
	for (int i = 0; i < result.count(); ++i) {
		int row = model.mapToSource(model.index(i, 0)).row();
		QCOMPARE(session_model.session(row), result.at(i).value<Session>());
	}
}

//-----------------------------------------------------------------------------

void TestFilters::editSessions_data()
{
	QTest::addColumn<QVariantList>("sessions");
	QTest::addColumn<int>("type");
	QTest::addColumn<int>("position");
	QTest::addColumn<Session>("replacement");
	QTest::addColumn<QVariantList>("result");

	QDate today = QDate::currentDate();
	QTest::newRow("Unfiltered by billed")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true))
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
		)
		<< int(FilterModel::Unbilled)
		<< 2
		<< Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		);

	QDate before = today.addYears(-1);
	QTest::newRow("Filtered by this year")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		)
		<< int(FilterModel::ThisYear)
		<< 1
		<< Session(before, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
		);
	QTest::newRow("Unfiltered by this year")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(before, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
		)
		<< int(FilterModel::ThisYear)
		<< 0
		<< Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		);

	before = today.addMonths(-1);
	QTest::newRow("Filtered by this month")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		)
		<< int(FilterModel::ThisMonth)
		<< 1
		<< Session(before, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
		);
	QTest::newRow("Unfiltered by this month")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(before, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
		)
		<< int(FilterModel::ThisMonth)
		<< 0
		<< Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		);

	before = today.addDays(-8);
	QTest::newRow("Filtered by this week")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		)
		<< int(FilterModel::ThisWeek)
		<< 1
		<< Session(before, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
		);
	QTest::newRow("Unfiltered by this week")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(before, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
		)
		<< int(FilterModel::ThisWeek)
		<< 0
		<< Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		);
}

//-----------------------------------------------------------------------------

void TestFilters::editSessions()
{
	SessionModel session_model;

	QFETCH(QVariantList, sessions);
	for (int i = 0; i < sessions.count(); ++i) {
		session_model.add(sessions.at(i).value<Session>());
	}

	FilterModel model(&session_model);
	QFETCH(int, type);
	model.setType(type);

	QFETCH(int, position);
	QFETCH(Session, replacement);
	session_model.edit(position, replacement);

	QFETCH(QVariantList, result);
	QCOMPARE(model.rowCount() - 1, result.count());
	for (int i = 0; i < result.count(); ++i) {
		int row = model.mapToSource(model.index(i, 0)).row();
		QCOMPARE(session_model.session(row), result.at(i).value<Session>());
	}
}

//-----------------------------------------------------------------------------

void TestFilters::removeSessions_data()
{
	QTest::addColumn<QVariantList>("sessions");
	QTest::addColumn<int>("type");
	QTest::addColumn<int>("position");
	QTest::addColumn<QVariantList>("result");

	QDate today = QDate::currentDate();
	QTest::newRow("Unfiltered by billed")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true))
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		)
		<< int(FilterModel::Unbilled)
		<< 2
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		);

	QDate before = today.addYears(-1);
	QTest::newRow("Filtered by this year")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(before, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(before, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		)
		<< int(FilterModel::ThisYear)
		<< 1
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		);
	QTest::newRow("Unfiltered by this year")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(before, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(before, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		)
		<< int(FilterModel::ThisYear)
		<< 2
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		);

	before = today.addMonths(-1);
	QTest::newRow("Filtered by this month")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(before, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(before, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		)
		<< int(FilterModel::ThisMonth)
		<< 1
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		);
	QTest::newRow("Unfiltered by this month")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(before, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(before, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		)
		<< int(FilterModel::ThisMonth)
		<< 2
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		);

	before = today.addDays(-8);
	QTest::newRow("Filtered by this week")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(before, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(before, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		)
		<< int(FilterModel::ThisWeek)
		<< 1
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		);
	QTest::newRow("Unfiltered by this week")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(before, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(before, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		)
		<< int(FilterModel::ThisWeek)
		<< 2
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		);
}

//-----------------------------------------------------------------------------

void TestFilters::removeSessions()
{
	SessionModel session_model;

	QFETCH(QVariantList, sessions);
	for (int i = 0; i < sessions.count(); ++i) {
		session_model.add(sessions.at(i).value<Session>());
	}

	FilterModel model(&session_model);
	QFETCH(int, type);
	model.setType(type);

	QFETCH(int, position);
	session_model.remove(position);

	QFETCH(QVariantList, result);
	QCOMPARE(model.rowCount() - 1, result.count());
	for (int i = 0; i < result.count(); ++i) {
		int row = model.mapToSource(model.index(i, 0)).row();
		QCOMPARE(session_model.session(row), result.at(i).value<Session>());
	}
}

//-----------------------------------------------------------------------------

void TestFilters::unbilledFilter_data()
{
	QTest::addColumn<QVariantList>("sessions");
	QTest::addColumn<int>("position");
	QTest::addColumn<bool>("billed");
	QTest::addColumn<QVariantList>("result");

	QDate today = QDate::currentDate();
	QTest::newRow("Bill sessions")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		)
		<< 1
		<< true
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		);
	QTest::newRow("Unbill sessions")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true))
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		)
		<< 1
		<< false
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		);
}

void TestFilters::unbilledFilter()
{
	SessionModel session_model;

	QFETCH(QVariantList, sessions);
	for (int i = 0; i < sessions.count(); ++i) {
		session_model.add(sessions.at(i).value<Session>());
	}

	FilterModel model(&session_model);
	model.setType(FilterModel::Unbilled);

	QFETCH(int, position);
	QFETCH(bool, billed);
	session_model.setBilled(position, billed);

	QFETCH(QVariantList, result);
	QCOMPARE(model.rowCount() - 1, result.count());
	for (int i = 0; i < result.count(); ++i) {
		int row = model.mapToSource(model.index(i, 0)).row();
		QCOMPARE(session_model.session(row), result.at(i).value<Session>());
	}
}

//-----------------------------------------------------------------------------

void TestFilters::mapUnbilled_data()
{
	QTest::addColumn<QVariantList>("sessions");
	QTest::addColumn<int>("type");
	QTest::addColumn<int>("first_unbilled_row");
	QTest::addColumn<QVariantList>("mapped_rows");

	QDate today = QDate::currentDate();
	QTest::newRow("Filter for billed")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(today, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true))
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		)
		<< int(FilterModel::Unbilled)
		<< 0
		<< (QVariantList()
			<< 2
			<< 3
		);

	QDate before = today.addYears(-1);
	QTest::newRow("Filter for this year")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(before, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(before, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(15, 0, 0), QTime(16, 0, 0), "Test session.", true))
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		)
		<< int(FilterModel::ThisYear)
		<< 1
		<< (QVariantList()
			<< 3
			<< 4
		);

	before = today.addMonths(-1);
	QTest::newRow("Filter for this month")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(before, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(before, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(15, 0, 0), QTime(16, 0, 0), "Test session.", true))
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		)
		<< int(FilterModel::ThisMonth)
		<< 1
		<< (QVariantList()
			<< 3
			<< 4
		);

	before = today.addDays(-8);
	QTest::newRow("Filter for this week")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(before, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(before, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(15, 0, 0), QTime(16, 0, 0), "Test session.", true))
			<< QVariant::fromValue<Session>(Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		)
		<< int(FilterModel::ThisWeek)
		<< 1
		<< (QVariantList()
			<< 3
			<< 4
		);
}

void TestFilters::mapUnbilled()
{
	SessionModel session_model;

	QFETCH(QVariantList, sessions);
	for (int i = 0; i < sessions.count(); ++i) {
		session_model.add(sessions.at(i).value<Session>());
	}

	FilterModel model(&session_model);
	QFETCH(int, type);
	model.setType(type);

	int unbilled_row = -1;
	QList<int> unbilled;
	for (int i = 0; i < model.rowCount() - 1; ++i) {
		int row = model.mapToSource(model.index(i, 0)).row();
		if (!session_model.isBilled(row)) {
			unbilled_row = i;
			break;
		}
	}

	QFETCH(int, first_unbilled_row);
	QCOMPARE(unbilled_row, first_unbilled_row);

	QFETCH(QVariantList, mapped_rows);
	QCOMPARE(model.rowCount() - 1, first_unbilled_row + mapped_rows.count());
	for (int i = 0; i < mapped_rows.count(); ++i) {
		int proxy_row = i + first_unbilled_row;
		int row = model.mapUnbilledToSource(model.index(proxy_row, 0)).row();
		QCOMPARE(row, mapped_rows.at(i).toInt());
	}
}

//-----------------------------------------------------------------------------

QTEST_MAIN(TestFilters)
