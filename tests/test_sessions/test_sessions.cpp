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

#include "test_sessions.h"

#include "data_model.h"
#include "session.h"
Q_DECLARE_METATYPE(Session)

#include <QTest>

//-----------------------------------------------------------------------------

void TestSessions::addSessions_data()
{
	QTest::addColumn<QDateTime>("start");
	QTest::addColumn<QDateTime>("stop");
	QTest::addColumn<QString>("task");
	QTest::addColumn<int>("row_count");

	QTest::newRow("Simple session")
			<< QDateTime(QDate(2013, 2, 18), QTime(12, 30, 0))
			<< QDateTime(QDate(2013, 2, 18), QTime(13, 45, 0))
			<< QString("Test session.")
			<< 2;
	QTest::newRow("Session across midnight")
			<< QDateTime(QDate(2013, 2, 18), QTime(22, 30, 0))
			<< QDateTime(QDate(2013, 2, 19), QTime(2, 45, 0))
			<< QString("Test session.")
			<< 3;
	QTest::newRow("Two day session")
			<< QDateTime(QDate(2013, 2, 18), QTime(22, 30, 0))
			<< QDateTime(QDate(2013, 2, 20), QTime(2, 45, 0))
			<< QString("Test session.")
			<< 4;
}

void TestSessions::addSessions()
{
	DataModel model;

	QFETCH(QDateTime, start);
	QFETCH(QDateTime, stop);
	QFETCH(QString, task);
	model.add(start, stop, task);

	QFETCH(int, row_count);
	QCOMPARE(model.rowCount(), row_count);

	Session session = model.session(0);
	QCOMPARE(session.date(), start.date());
	QCOMPARE(session.start(), start.time());
	if (row_count > 2) {
		QCOMPARE(session.stop(), QTime(23, 59, 59));
		session = model.session(row_count - 2);
		QCOMPARE(session.date(), stop.date());
		QCOMPARE(session.start(), QTime(0, 0, 0));
	}
	QCOMPARE(session.stop(), stop.time());
	QCOMPARE(session.task(), task);
	QCOMPARE(session.isBilled(), false);
}

//-----------------------------------------------------------------------------

void TestSessions::addConflictingSessions_data()
{
	QTest::addColumn<QDateTime>("start");
	QTest::addColumn<QDateTime>("stop");
	QTest::addColumn<QString>("task");
	QTest::addColumn<QDateTime>("conflict_start");
	QTest::addColumn<QDateTime>("conflict_stop");
	QTest::addColumn<QString>("conflict_task");

	QTest::newRow("Prevent duplicate sessions")
			<< QDateTime(QDate(2013, 2, 18), QTime(12, 30, 0))
			<< QDateTime(QDate(2013, 2, 18), QTime(13, 45, 0))
			<< QString("Test session.")
			<< QDateTime(QDate(2013, 2, 18), QTime(12, 30, 0))
			<< QDateTime(QDate(2013, 2, 18), QTime(13, 45, 0))
			<< QString("Conflict session.");
	QTest::newRow("Prevent overlap session start")
			<< QDateTime(QDate(2013, 2, 18), QTime(12, 30, 0))
			<< QDateTime(QDate(2013, 2, 18), QTime(13, 45, 0))
			<< QString("Test session.")
			<< QDateTime(QDate(2013, 2, 18), QTime(11, 0, 0))
			<< QDateTime(QDate(2013, 2, 18), QTime(13, 0, 0))
			<< QString("Conflict session.");
	QTest::newRow("Prevent overlap session end")
			<< QDateTime(QDate(2013, 2, 18), QTime(12, 30, 0))
			<< QDateTime(QDate(2013, 2, 18), QTime(13, 45, 0))
			<< QString("Test session.")
			<< QDateTime(QDate(2013, 2, 18), QTime(13, 0, 0))
			<< QDateTime(QDate(2013, 2, 18), QTime(14, 0, 0))
			<< QString("Conflict session.");
	QTest::newRow("Prevent overlap entire session")
			<< QDateTime(QDate(2013, 2, 18), QTime(12, 30, 0))
			<< QDateTime(QDate(2013, 2, 18), QTime(13, 45, 0))
			<< QString("Test session.")
			<< QDateTime(QDate(2013, 2, 18), QTime(11, 0, 0))
			<< QDateTime(QDate(2013, 2, 18), QTime(14, 0, 0))
			<< QString("Conflict session.");
	QTest::newRow("Prevent overlap inside session")
			<< QDateTime(QDate(2013, 2, 18), QTime(12, 30, 0))
			<< QDateTime(QDate(2013, 2, 18), QTime(13, 45, 0))
			<< QString("Test session.")
			<< QDateTime(QDate(2013, 2, 18), QTime(13, 0, 0))
			<< QDateTime(QDate(2013, 2, 18), QTime(13, 30, 0))
			<< QString("Conflict session.");
}

void TestSessions::addConflictingSessions()
{
	DataModel model;

	QFETCH(QDateTime, start);
	QFETCH(QDateTime, stop);
	QFETCH(QString, task);
	model.add(start, stop, task);

	QFETCH(QDateTime, conflict_start);
	QFETCH(QDateTime, conflict_stop);
	QFETCH(QString, conflict_task);
	model.add(conflict_start, conflict_stop, conflict_task);

	QCOMPARE(model.rowCount(), 2);
}

//-----------------------------------------------------------------------------

void TestSessions::addMultipleSessions_data()
{
	QTest::addColumn<QVariantList>("sessions");
	QTest::addColumn<QVariantList>("result");

	QTest::newRow("Sorted")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
		)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
		);
	QTest::newRow("Sorted reverse")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
		)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
		);
	QTest::newRow("Unsorted")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
		)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
		);
}

void TestSessions::addMultipleSessions()
{
	DataModel model;

	QFETCH(QVariantList, sessions);
	for (int i = 0; i < sessions.count(); ++i) {
		model.add(sessions.at(i).value<Session>());
	}

	QFETCH(QVariantList, result);
	for (int i = 0; i < result.count(); ++i) {
		QCOMPARE(model.session(i), result.at(i).value<Session>());
	}
}

//-----------------------------------------------------------------------------

void TestSessions::editSessions_data()
{
	QTest::addColumn<QVariantList>("sessions");
	QTest::addColumn<int>("position");
	QTest::addColumn<Session>("replacement");
	QTest::addColumn<QVariantList>("result");

	QTest::newRow("Simple edit")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
		)
		<< 1
		<< Session(QDate(2013, 2, 18), QTime(15, 0, 0), QTime(15, 45, 0), "Test session.", false)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(15, 0, 0), QTime(15, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
		);
	QTest::newRow("Prevent editing into conflict")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
		)
		<< 1
		<< Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
		);
}

void TestSessions::editSessions()
{
	DataModel model;

	QFETCH(QVariantList, sessions);
	for (int i = 0; i < sessions.count(); ++i) {
		model.add(sessions.at(i).value<Session>());
	}

	QFETCH(int, position);
	QFETCH(Session, replacement);
	model.edit(position, replacement);

	QFETCH(QVariantList, result);
	for (int i = 0; i < result.count(); ++i) {
		QCOMPARE(model.session(i), result.at(i).value<Session>());
	}
}

//-----------------------------------------------------------------------------

void TestSessions::removeSessions_data()
{
	QTest::addColumn<QVariantList>("sessions");
	QTest::addColumn<QVariantList>("remove");
	QTest::addColumn<QVariantList>("result");

	QTest::newRow("Remove one session")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
		)
		<< (QVariantList()
			<< 1
		)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
		);
	QTest::newRow("Remove two sessions in a row")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		)
		<< (QVariantList()
			<< 1
			<< 1
		)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		);
}

void TestSessions::removeSessions()
{
	DataModel model;

	QFETCH(QVariantList, sessions);
	for (int i = 0; i < sessions.count(); ++i) {
		model.add(sessions.at(i).value<Session>());
	}

	QFETCH(QVariantList, remove);
	for (int i = 0; i < remove.count(); ++i) {
		model.remove(remove.at(i).toInt());
	}

	QFETCH(QVariantList, result);
	for (int i = 0; i < result.count(); ++i) {
		QCOMPARE(model.session(i), result.at(i).value<Session>());
	}
}

//-----------------------------------------------------------------------------

QTEST_MAIN(TestSessions)
