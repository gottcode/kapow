/*
	SPDX-FileCopyrightText: 2013-2025 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "test_sessions.h"

#include "session.h"
#include "session_model.h"
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
		<< 1;
	QTest::newRow("Session across midnight")
		<< QDateTime(QDate(2013, 2, 18), QTime(22, 30, 0))
		<< QDateTime(QDate(2013, 2, 19), QTime(2, 45, 0))
		<< QString("Test session.")
		<< 2;
	QTest::newRow("Two day session")
		<< QDateTime(QDate(2013, 2, 18), QTime(22, 30, 0))
		<< QDateTime(QDate(2013, 2, 20), QTime(2, 45, 0))
		<< QString("Test session.")
		<< 3;
}

void TestSessions::addSessions()
{
	SessionModel model;

	QFETCH(QDateTime, start);
	QFETCH(QDateTime, stop);
	QFETCH(QString, task);
	model.add(start, stop, task);

	QFETCH(int, row_count);
	QCOMPARE(model.rowCount() - 1, row_count);

	Session session = model.session(0);
	QCOMPARE(session.date(), start.date());
	QCOMPARE(session.start(), start.time());
	if (row_count > 1) {
		QCOMPARE(session.stop(), QTime(23, 59, 59));
		session = model.session(row_count - 1);
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
	QTest::newRow("Prevent overlap session start from previous day")
		<< QDateTime(QDate(2013, 2, 18), QTime(12, 30, 0))
		<< QDateTime(QDate(2013, 2, 18), QTime(13, 45, 0))
		<< QString("Test session.")
		<< QDateTime(QDate(2013, 2, 17), QTime(11, 0, 0))
		<< QDateTime(QDate(2013, 2, 18), QTime(13, 0, 0))
		<< QString("Conflict session.");
	QTest::newRow("Prevent overlap session end into next day")
		<< QDateTime(QDate(2013, 2, 18), QTime(12, 30, 0))
		<< QDateTime(QDate(2013, 2, 18), QTime(13, 45, 0))
		<< QString("Test session.")
		<< QDateTime(QDate(2013, 2, 18), QTime(13, 0, 0))
		<< QDateTime(QDate(2013, 2, 19), QTime(14, 0, 0))
		<< QString("Conflict session.");
	QTest::newRow("Prevent overlap entire session from previous day")
		<< QDateTime(QDate(2013, 2, 18), QTime(12, 30, 0))
		<< QDateTime(QDate(2013, 2, 18), QTime(13, 45, 0))
		<< QString("Test session.")
		<< QDateTime(QDate(2013, 2, 17), QTime(11, 0, 0))
		<< QDateTime(QDate(2013, 2, 18), QTime(14, 0, 0))
		<< QString("Conflict session.");
	QTest::newRow("Prevent overlap entire session into next day")
		<< QDateTime(QDate(2013, 2, 18), QTime(12, 30, 0))
		<< QDateTime(QDate(2013, 2, 18), QTime(13, 45, 0))
		<< QString("Test session.")
		<< QDateTime(QDate(2013, 2, 18), QTime(11, 0, 0))
		<< QDateTime(QDate(2013, 2, 19), QTime(14, 0, 0))
		<< QString("Conflict session.");
	QTest::newRow("Prevent overlap entire session across multiple days")
		<< QDateTime(QDate(2013, 2, 18), QTime(12, 30, 0))
		<< QDateTime(QDate(2013, 2, 18), QTime(13, 45, 0))
		<< QString("Test session.")
		<< QDateTime(QDate(2013, 2, 17), QTime(11, 0, 0))
		<< QDateTime(QDate(2013, 2, 19), QTime(14, 0, 0))
		<< QString("Conflict session.");
}

void TestSessions::addConflictingSessions()
{
	SessionModel model;

	QFETCH(QDateTime, start);
	QFETCH(QDateTime, stop);
	QFETCH(QString, task);
	model.add(start, stop, task);

	QFETCH(QDateTime, conflict_start);
	QFETCH(QDateTime, conflict_stop);
	QFETCH(QString, conflict_task);
	model.add(conflict_start, conflict_stop, conflict_task);

	QCOMPARE(model.rowCount() - 1, 1);
}

//-----------------------------------------------------------------------------

void TestSessions::addMultipleSessions_data()
{
	QTest::addColumn<QList<Session>>("sessions");
	QTest::addColumn<QList<Session>>("result");

	QTest::newRow("Sorted")
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		}
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		};
	QTest::newRow("Sorted reverse")
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false)
		}
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		};
	QTest::newRow("Unsorted")
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		}
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		};
	QTest::newRow("Sessions with some billed")
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		}
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		};
	QTest::newRow("Prevent adding to billed")
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(10, 0, 0), QTime(11, 0, 0), "Test session.", false)
		}
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		};
}

void TestSessions::addMultipleSessions()
{
	SessionModel model;

	QFETCH(QList<Session>, sessions);
	for (const Session& session : sessions) {
		model.add(session);
	}

	QFETCH(QList<Session>, result);
	QCOMPARE(model.rowCount() - 1, result.count());
	for (int i = 0; i < result.count(); ++i) {
		QCOMPARE(model.session(i), result.at(i));
	}
}

//-----------------------------------------------------------------------------

void TestSessions::addSessionsFixConflict_data()
{
	QTest::addColumn<QList<Session>>("sessions");
	QTest::addColumn<QDateTime>("start");
	QTest::addColumn<QDateTime>("stop");
	QTest::addColumn<QString>("task");
	QTest::addColumn<QList<Session>>("result");

	QTest::newRow("No sessions")
		<< QList<Session>()
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< QDateTime(QDate(2020, 12, 18), QTime(15, 45, 0))
		<< QString("New session.")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(14, 30, 0), QTime(15, 45, 0), "New session.", false)
		};
	QTest::newRow("Session before")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false)
		}
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< QDateTime(QDate(2020, 12, 18), QTime(15, 45, 0))
		<< QString("New session.")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2020, 12, 18), QTime(14, 30, 0), QTime(15, 45, 0), "New session.", false)
		};
	QTest::newRow("Session after")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		}
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< QDateTime(QDate(2020, 12, 18), QTime(15, 45, 0))
		<< QString("New session.")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(14, 30, 0), QTime(15, 45, 0), "New session.", false),
			Session(QDate(2020, 12, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		};
	QTest::newRow("Intersect end")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(15, 30, 0), QTime(17, 45, 0), "Test session.", false)
		}
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< QDateTime(QDate(2020, 12, 18), QTime(15, 45, 0))
		<< QString("New session.")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(14, 30, 0), QTime(15, 29, 59), "New session.", false),
			Session(QDate(2020, 12, 18), QTime(15, 30, 0), QTime(17, 45, 0), "Test session.", false)
		};
	QTest::newRow("Intersect end next day")
		<< QList<Session>{
			Session(QDate(2020, 12, 19), QTime(15, 30, 0), QTime(17, 45, 0), "Test session.", false)
		}
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< QDateTime(QDate(2020, 12, 19), QTime(15, 45, 0))
		<< QString("New session.")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(14, 30, 0), QTime(23, 59, 59), "New session.", false),
			Session(QDate(2020, 12, 19), QTime(0, 0, 0), QTime(15, 29, 59), "New session.", false),
			Session(QDate(2020, 12, 19), QTime(15, 30, 0), QTime(17, 45, 0), "Test session.", false)
		};
	QTest::newRow("Intersect start")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(12, 30, 0), QTime(15, 45, 0), "Test session.", false)
		}
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< QDateTime(QDate(2020, 12, 18), QTime(15, 45, 0))
		<< QString("New session.")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(12, 30, 0), QTime(15, 45, 0), "Test session.", false)
		};
	QTest::newRow("Intersect start previous day")
		<< QList<Session>{
			Session(QDate(2020, 12, 17), QTime(12, 30, 0), QTime(15, 45, 0), "Test session.", false)
		}
		<< QDateTime(QDate(2020, 12, 17), QTime(14, 30, 0))
		<< QDateTime(QDate(2020, 12, 18), QTime(15, 45, 0))
		<< QString("New session.")
		<< QList<Session>{
			Session(QDate(2020, 12, 17), QTime(12, 30, 0), QTime(15, 45, 0), "Test session.", false)
		};
	QTest::newRow("Overlap single")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(15, 0, 0), QTime(15, 15, 0), "Test session.", false)
		}
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< QDateTime(QDate(2020, 12, 18), QTime(17, 45, 0))
		<< QString("New session.")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(14, 30, 0), QTime(14, 59, 59), "New session.", false),
			Session(QDate(2020, 12, 18), QTime(15, 0, 0), QTime(15, 15, 0), "Test session.", false)
		};
	QTest::newRow("Overlap single next day")
		<< QList<Session>{
			Session(QDate(2020, 12, 19), QTime(15, 0, 0), QTime(15, 15, 0), "Test session.", false)
		}
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< QDateTime(QDate(2020, 12, 19), QTime(17, 45, 0))
		<< QString("New session.")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(14, 30, 0), QTime(23, 59, 59), "New session.", false),
			Session(QDate(2020, 12, 19), QTime(0, 0, 0), QTime(14, 59, 59), "New session.", false),
			Session(QDate(2020, 12, 19), QTime(15, 0, 0), QTime(15, 15, 0), "Test session.", false)
		};
	QTest::newRow("Overlap single previous day")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(15, 0, 0), QTime(15, 15, 0), "Test session.", false)
		}
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< QDateTime(QDate(2020, 12, 19), QTime(17, 45, 0))
		<< QString("New session.")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(14, 30, 0), QTime(14, 59, 59), "New session.", false),
			Session(QDate(2020, 12, 18), QTime(15, 0, 0), QTime(15, 15, 0), "Test session.", false)
		};
	QTest::newRow("Overlap multiple")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(15, 0, 0), QTime(15, 15, 0), "First session.", false),
			Session(QDate(2020, 12, 18), QTime(16, 0, 0), QTime(16, 15, 0), "Second session.", false)
		}
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< QDateTime(QDate(2020, 12, 18), QTime(17, 45, 0))
		<< QString("New session.")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(14, 30, 0), QTime(14, 59, 59), "New session.", false),
			Session(QDate(2020, 12, 18), QTime(15, 0, 0), QTime(15, 15, 0), "First session.", false),
			Session(QDate(2020, 12, 18), QTime(16, 0, 0), QTime(16, 15, 0), "Second session.", false)
		};
	QTest::newRow("Overlap multiple next day")
		<< QList<Session>{
			Session(QDate(2020, 12, 19), QTime(15, 0, 0), QTime(15, 15, 0), "First session.", false),
			Session(QDate(2020, 12, 19), QTime(16, 0, 0), QTime(16, 15, 0), "Second session.", false)
		}
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< QDateTime(QDate(2020, 12, 19), QTime(17, 45, 0))
		<< QString("New session.")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(14, 30, 0), QTime(23, 59, 59), "New session.", false),
			Session(QDate(2020, 12, 19), QTime(0, 0, 0), QTime(14, 59, 59), "New session.", false),
			Session(QDate(2020, 12, 19), QTime(15, 0, 0), QTime(15, 15, 0), "First session.", false),
			Session(QDate(2020, 12, 19), QTime(16, 0, 0), QTime(16, 15, 0), "Second session.", false)
		};
	QTest::newRow("Overlap multiple previous day")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(15, 0, 0), QTime(15, 15, 0), "First session.", false),
			Session(QDate(2020, 12, 18), QTime(16, 0, 0), QTime(16, 15, 0), "Second session.", false)
		}
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< QDateTime(QDate(2020, 12, 19), QTime(17, 45, 0))
		<< QString("New session.")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(14, 30, 0), QTime(14, 59, 59), "New session.", false),
			Session(QDate(2020, 12, 18), QTime(15, 0, 0), QTime(15, 15, 0), "First session.", false),
			Session(QDate(2020, 12, 18), QTime(16, 0, 0), QTime(16, 15, 0), "Second session.", false)
		};
}

void TestSessions::addSessionsFixConflict()
{
	SessionModel model;

	QFETCH(QList<Session>, sessions);
	for (const Session& session : sessions) {
		model.add(session);
	}

	QFETCH(QDateTime, start);
	QFETCH(QDateTime, stop);
	QFETCH(QString, task);
	model.fixConflict(start, stop);
	model.add(start, stop, task);

	QFETCH(QList<Session>, result);
	QCOMPARE(model.rowCount() - 1, result.count());
	for (int i = 0; i < result.count(); ++i) {
		QCOMPARE(model.session(i), result.at(i));
	}
}

//-----------------------------------------------------------------------------

void TestSessions::addSessionsHasConflict_data()
{
	QTest::addColumn<QList<Session>>("sessions");
	QTest::addColumn<QDateTime>("test");
	QTest::addColumn<bool>("result");

	QTest::newRow("No sessions")
		<< QList<Session>()
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< false;
	QTest::newRow("Session before")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false)
		}
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< false;
	QTest::newRow("Session after")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		}
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< false;
	QTest::newRow("Between sessions")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2020, 12, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		}
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< false;
	QTest::newRow("Inside session")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false)
		}
		<< QDateTime(QDate(2020, 12, 18), QTime(13, 0, 0))
		<< true;
}

void TestSessions::addSessionsHasConflict()
{
	SessionModel model;

	QFETCH(QList<Session>, sessions);
	for (const Session& session : sessions) {
		model.add(session);
	}

	QFETCH(QDateTime, test);
	QFETCH(bool, result);
	QCOMPARE(model.hasConflict(test), result);
}

//-----------------------------------------------------------------------------

void TestSessions::addSessionsMaximumDateTime_data()
{
	QTest::addColumn<QDateTime>("maximum");
	QTest::addColumn<QDateTime>("start");
	QTest::addColumn<QDateTime>("stop");
	QTest::addColumn<QString>("task");
	QTest::addColumn<QList<Session>>("result");

	QTest::newRow("Add before max")
		<< QDateTime(QDate(2020, 12, 18), QTime(16, 0, 0))
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< QDateTime(QDate(2020, 12, 18), QTime(15, 45, 0))
		<< QString("New session.")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(14, 30, 0), QTime(15, 45, 0), "New session.", false)
		};
	QTest::newRow("Prevent add across max")
		<< QDateTime(QDate(2020, 12, 18), QTime(15, 0, 0))
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< QDateTime(QDate(2020, 12, 18), QTime(15, 45, 0))
		<< QString("New session.")
		<< QList<Session>();
	QTest::newRow("Prevent add after max")
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 0, 0))
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< QDateTime(QDate(2020, 12, 18), QTime(15, 45, 0))
		<< QString("New session.")
		<< QList<Session>();
}

void TestSessions::addSessionsMaximumDateTime()
{
	SessionModel model;

	QFETCH(QDateTime, maximum);
	model.setMaximumDateTime(maximum);

	QFETCH(QDateTime, start);
	QFETCH(QDateTime, stop);
	QFETCH(QString, task);
	model.add(start, stop, task);

	QFETCH(QList<Session>, result);
	QCOMPARE(model.rowCount() - 1, result.count());
	for (int i = 0; i < result.count(); ++i) {
		QCOMPARE(model.session(i), result.at(i));
	}
}

//-----------------------------------------------------------------------------

void TestSessions::billSessions_data()
{
	QTest::addColumn<QList<Session>>("sessions");
	QTest::addColumn<QList<int>>("toggle");
	QTest::addColumn<QList<bool>>("result");

	QTest::newRow("Check unbilled status")
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		}
		<< QList<int>()
		<< QList<bool>{
			false,
			false,
			false,
			false
		};
	QTest::newRow("Check billed status")
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", true),
			Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", true),
			Session(QDate(2013, 2, 18), QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		}
		<< QList<int>()
		<< QList<bool>{
			true,
			true,
			true,
			false
		};
	QTest::newRow("Bill session")
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		}
		<< QList<int>{
			1
		}
		<< QList<bool>{
			true,
			true,
			false,
			false
		};
	QTest::newRow("Bill second session")
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", true),
			Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		}
		<< QList<int>{
			2
		}
		<< QList<bool>{
			true,
			true,
			true,
			false
		};
	QTest::newRow("Unbill session")
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", true),
			Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", true),
			Session(QDate(2013, 2, 18), QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		}
		<< QList<int>{
			2
		}
		<< QList<bool>{
			true,
			false,
			false,
			false
		};
	QTest::newRow("Unbill all sessions")
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", true),
			Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", true),
			Session(QDate(2013, 2, 18), QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		}
		<< QList<int>{
			0,
			2
		}
		<< QList<bool>{
			false,
			false,
			false,
			false,
		};
	QTest::newRow("Toggle billed session")
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", true),
			Session(QDate(2013, 2, 18), QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		}
		<< QList<int>{
			2,
			2
		}
		<< QList<bool>{
			true,
			true,
			true,
			false
		};
	QTest::newRow("Toggle unbilled session")
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		}
		<< QList<int>{
			2,
			2
		}
		<< QList<bool>{
			false,
			false,
			false,
			false
		};
}

void TestSessions::billSessions()
{
	SessionModel model;

	QFETCH(QList<Session>, sessions);
	for (const Session& session : sessions) {
		model.add(session);
	}

	QFETCH(QList<int>, toggle);
	for (int pos : toggle) {
		model.setBilled(pos, !model.session(pos).isBilled());
	}

	QFETCH(QList<bool>, result);
	QCOMPARE(model.rowCount() - 1, result.count());
	for (int i = 0; i < result.count(); ++i) {
		QCOMPARE(model.isBilled(i), result.at(i));
	}
}

//-----------------------------------------------------------------------------

void TestSessions::editSessions_data()
{
	QTest::addColumn<QList<Session>>("sessions");
	QTest::addColumn<int>("position");
	QTest::addColumn<Session>("replacement");
	QTest::addColumn<QList<Session>>("result");

	QTest::newRow("Simple edit")
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		}
		<< 1
		<< Session(QDate(2013, 2, 18), QTime(15, 0, 0), QTime(15, 45, 0), "New session.", false)
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(15, 0, 0), QTime(15, 45, 0), "New session.", false),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		};
	QTest::newRow("Prevent editing into conflict")
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		}
		<< 1
		<< Session(QDate(2013, 2, 18), QTime(13, 0, 0), QTime(14, 45, 0), "New session.", false)
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		};
	QTest::newRow("Prevent editing into overlap")
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		}
		<< 1
		<< Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "New session.", false)
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		};
	QTest::newRow("Prevent editing in billed")
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		}
		<< 0
		<< Session(QDate(2013, 2, 18), QTime(15, 0, 0), QTime(15, 45, 0), "New session.", false)
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		};
	QTest::newRow("Prevent editing into billed")
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		}
		<< 2
		<< Session(QDate(2013, 2, 18), QTime(10, 0, 0), QTime(11, 0, 0), "New session.", false)
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		};
	QTest::newRow("Prevent editing billed session")
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		}
		<< 1
		<< Session(QDate(2013, 2, 18), QTime(15, 0, 0), QTime(15, 45, 0), "New session.", false)
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		};
}

void TestSessions::editSessions()
{
	SessionModel model;

	QFETCH(QList<Session>, sessions);
	for (const Session& session : sessions) {
		model.add(session);
	}

	QFETCH(int, position);
	QFETCH(Session, replacement);
	model.edit(position, replacement);

	QFETCH(QList<Session>, result);
	QCOMPARE(model.rowCount() - 1, result.count());
	for (int i = 0; i < result.count(); ++i) {
		QCOMPARE(model.session(i), result.at(i));
	}
}

//-----------------------------------------------------------------------------

void TestSessions::editSessionsMaximumDateTime_data()
{
	QTest::addColumn<Session>("session");
	QTest::addColumn<QDateTime>("maximum");
	QTest::addColumn<Session>("replacement");
	QTest::addColumn<Session>("result");

	QTest::newRow("Before to before max")
		<< Session(QDate(2020, 12, 18), QTime(12, 0, 0), QTime(13, 45, 0), "Test session.", false)
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< Session(QDate(2020, 12, 18), QTime(11, 0, 0), QTime(12, 45, 0), "New session.", false)
		<< Session(QDate(2020, 12, 18), QTime(11, 0, 0), QTime(12, 45, 0), "New session.", false);
	QTest::newRow("After to before max")
		<< Session(QDate(2020, 12, 18), QTime(15, 0, 0), QTime(16, 45, 0), "Test session.", false)
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< Session(QDate(2020, 12, 18), QTime(11, 0, 0), QTime(12, 45, 0), "New session.", false)
		<< Session(QDate(2020, 12, 18), QTime(11, 0, 0), QTime(12, 45, 0), "New session.", false);
	QTest::newRow("Across to before max")
		<< Session(QDate(2020, 12, 18), QTime(12, 0, 0), QTime(17, 45, 0), "Test session.", false)
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< Session(QDate(2020, 12, 18), QTime(11, 0, 0), QTime(12, 45, 0), "New session.", false)
		<< Session(QDate(2020, 12, 18), QTime(11, 0, 0), QTime(12, 45, 0), "New session.", false);
	QTest::newRow("Prevent before to after max")
		<< Session(QDate(2020, 12, 18), QTime(12, 0, 0), QTime(13, 45, 0), "Test session.", false)
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< Session(QDate(2020, 12, 18), QTime(15, 0, 0), QTime(16, 45, 0), "New session.", false)
		<< Session(QDate(2020, 12, 18), QTime(12, 0, 0), QTime(13, 45, 0), "Test session.", false);
	QTest::newRow("Prevent before to across max")
		<< Session(QDate(2020, 12, 18), QTime(12, 0, 0), QTime(13, 45, 0), "Test session.", false)
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< Session(QDate(2020, 12, 18), QTime(12, 0, 0), QTime(16, 45, 0), "New session.", false)
		<< Session(QDate(2020, 12, 18), QTime(12, 0, 0), QTime(13, 45, 0), "Test session.", false);
	QTest::newRow("Prevent after to after max")
		<< Session(QDate(2020, 12, 18), QTime(15, 0, 0), QTime(16, 45, 0), "Test session.", false)
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< Session(QDate(2020, 12, 18), QTime(17, 0, 0), QTime(18, 45, 0), "New session.", false)
		<< Session(QDate(2020, 12, 18), QTime(15, 0, 0), QTime(16, 45, 0), "Test session.", false);
	QTest::newRow("Prevent after to across max")
		<< Session(QDate(2020, 12, 18), QTime(15, 0, 0), QTime(16, 45, 0), "Test session.", false)
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< Session(QDate(2020, 12, 18), QTime(12, 0, 0), QTime(16, 45, 0), "New session.", false)
		<< Session(QDate(2020, 12, 18), QTime(15, 0, 0), QTime(16, 45, 0), "Test session.", false);
}

void TestSessions::editSessionsMaximumDateTime()
{
	SessionModel model;

	QFETCH(Session, session);
	model.add(session);

	QFETCH(QDateTime, maximum);
	model.setMaximumDateTime(maximum);

	QFETCH(Session, replacement);
	model.edit(0, replacement);

	QFETCH(Session, result);
	QCOMPARE(model.session(0), result);
}

//-----------------------------------------------------------------------------

void TestSessions::removeSessions_data()
{
	QTest::addColumn<QList<Session>>("sessions");
	QTest::addColumn<QList<int>>("remove");
	QTest::addColumn<QList<Session>>("result");

	QTest::newRow("Remove one session")
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		}
		<< QList<int>{
			1
		}
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		};
	QTest::newRow("Remove two sessions in a row")
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		}
		<< QList<int>{
			1,
			1
		}
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		};
	QTest::newRow("Prevent removing from billed")
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		}
		<< QList<int>{
			0
		}
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		};
	QTest::newRow("Prevent removing billed session")
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		}
		<< QList<int>{
			1
		}
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		};
}

void TestSessions::removeSessions()
{
	SessionModel model;

	QFETCH(QList<Session>, sessions);
	for (const Session& session : sessions) {
		model.add(session);
	}

	QFETCH(QList<int>, remove);
	for (int row : remove) {
		model.remove(row);
	}

	QFETCH(QList<Session>, result);
	QCOMPARE(model.rowCount() - 1, result.count());
	for (int i = 0; i < result.count(); ++i) {
		QCOMPARE(model.session(i), result.at(i));
	}
}

//-----------------------------------------------------------------------------

void TestSessions::removeMultipleSessions_data()
{
	QTest::addColumn<QList<Session>>("sessions");
	QTest::addColumn<QList<int>>("rows");
	QTest::addColumn<QList<Session>>("result");

	QTest::newRow("Remove one session")
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		}
		<< QList<int>{
			1
		}
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		};
	QTest::newRow("Remove two sessions in a row")
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		}
		<< QList<int>{
			1,
			2
		}
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		};
	QTest::newRow("Remove two sessions")
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		}
		<< QList<int>{
			1,
			3
		}
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		};
	QTest::newRow("Remove three sessions")
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(21, 0, 0), QTime(22, 15, 0), "Test session.", false)
		}
		<< QList<int>{
			3,
			1,
			4
		}
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		};
	QTest::newRow("Prevent removing from billed")
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(11, 30, 0), QTime(11, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		}
		<< QList<int>{
			0,
			1
		}
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(11, 30, 0), QTime(11, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		};
	QTest::newRow("Prevent removing billed session")
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		}
		<< QList<int>{
			0,
			1
		}
		<< QList<Session>{
			Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true),
			Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		};
}

void TestSessions::removeMultipleSessions()
{
	SessionModel model;

	QFETCH(QList<Session>, sessions);
	for (const Session& session : sessions) {
		model.add(session);
	}

	QFETCH(QList<int>, rows);
	model.remove(rows);

	QFETCH(QList<Session>, result);
	QCOMPARE(model.rowCount() - 1, result.count());
	for (int i = 0; i < result.count(); ++i) {
		QCOMPARE(model.session(i), result.at(i));
	}
}

//-----------------------------------------------------------------------------

void TestSessions::takeSessions_data()
{
	QTest::addColumn<QList<Session>>("source");
	QTest::addColumn<QList<Session>>("dest");
	QTest::addColumn<QList<int>>("take");
	QTest::addColumn<QList<Session>>("source_result");
	QTest::addColumn<QList<Session>>("dest_result");

	QTest::newRow("Move to empty model")
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(12, 30, 0), QTime(13, 45, 0), "Source session.", false),
			Session(QDate(2025, 4, 20), QTime(14, 0, 0), QTime(14, 45, 0), "Taken session.", false)
		}
		<< QList<Session>{}
		<< QList<int>{
			1
		}
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(12, 30, 0), QTime(13, 45, 0), "Source session.", false)
		}
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(14, 0, 0), QTime(14, 45, 0), "Taken session.", false)
		};
	QTest::newRow("Move after")
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(16, 30, 0), QTime(17, 45, 0), "Source session.", false),
			Session(QDate(2025, 4, 20), QTime(18, 0, 0), QTime(20, 15, 0), "Taken session.", false)
		}
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(12, 30, 0), QTime(13, 45, 0), "Dest session.", false),
			Session(QDate(2025, 4, 20), QTime(14, 0, 0), QTime(14, 45, 0), "Dest session.", false)
		}
		<< QList<int>{
			1
		}
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(16, 30, 0), QTime(17, 45, 0), "Source session.", false)
		}
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(12, 30, 0), QTime(13, 45, 0), "Dest session.", false),
			Session(QDate(2025, 4, 20), QTime(14, 0, 0), QTime(14, 45, 0), "Dest session.", false),
			Session(QDate(2025, 4, 20), QTime(18, 0, 0), QTime(20, 15, 0), "Taken session.", false)
		};
	QTest::newRow("Move before")
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(12, 30, 0), QTime(13, 45, 0), "Source session.", false),
			Session(QDate(2025, 4, 20), QTime(14, 0, 0), QTime(14, 45, 0), "Taken session.", false),
		}
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(16, 30, 0), QTime(17, 45, 0), "Dest session.", false),
			Session(QDate(2025, 4, 20), QTime(18, 0, 0), QTime(20, 15, 0), "Dest session.", false)
		}
		<< QList<int>{
			1
		}
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(12, 30, 0), QTime(13, 45, 0), "Source session.", false)
		}
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(14, 0, 0), QTime(14, 45, 0), "Taken session.", false),
			Session(QDate(2025, 4, 20), QTime(16, 30, 0), QTime(17, 45, 0), "Dest session.", false),
			Session(QDate(2025, 4, 20), QTime(18, 0, 0), QTime(20, 15, 0), "Dest session.", false)
		};
	QTest::newRow("Move between")
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(14, 0, 0), QTime(14, 45, 0), "Source session.", false),
			Session(QDate(2025, 4, 20), QTime(16, 30, 0), QTime(17, 45, 0), "Taken session.", false)
		}
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(12, 30, 0), QTime(13, 45, 0), "Dest session.", false),
			Session(QDate(2025, 4, 20), QTime(18, 0, 0), QTime(20, 15, 0), "Dest session.", false)
		}
		<< QList<int>{
			1
		}
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(14, 0, 0), QTime(14, 45, 0), "Source session.", false)
		}
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(12, 30, 0), QTime(13, 45, 0), "Dest session.", false),
			Session(QDate(2025, 4, 20), QTime(16, 30, 0), QTime(17, 45, 0), "Taken session.", false),
			Session(QDate(2025, 4, 20), QTime(18, 0, 0), QTime(20, 15, 0), "Dest session.", false)
		};
	QTest::newRow("Prevent moving into billed")
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(12, 30, 0), QTime(13, 45, 0), "Source session.", false),
			Session(QDate(2025, 4, 20), QTime(14, 0, 0), QTime(14, 45, 0), "Taken session.", false)
		}
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(16, 30, 0), QTime(17, 45, 0), "Dest session.", true),
			Session(QDate(2025, 4, 20), QTime(18, 0, 0), QTime(20, 15, 0), "Dest session.", false)
		}
		<< QList<int>{
			1
		}
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(12, 30, 0), QTime(13, 45, 0), "Source session.", false),
			Session(QDate(2025, 4, 20), QTime(14, 0, 0), QTime(14, 45, 0), "Taken session.", false)
		}
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(16, 30, 0), QTime(17, 45, 0), "Dest session.", true),
			Session(QDate(2025, 4, 20), QTime(18, 0, 0), QTime(20, 15, 0), "Dest session.", false)
		};
	QTest::newRow("Prevent taking billed session")
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(12, 30, 0), QTime(13, 45, 0), "Source session.", false),
			Session(QDate(2025, 4, 20), QTime(14, 0, 0), QTime(14, 45, 0), "Taken session.", true)
		}
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(16, 30, 0), QTime(17, 45, 0), "Dest session.", false),
			Session(QDate(2025, 4, 20), QTime(18, 0, 0), QTime(20, 15, 0), "Dest session.", false)
		}
		<< QList<int>{
			1
		}
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(12, 30, 0), QTime(13, 45, 0), "Source session.", false),
			Session(QDate(2025, 4, 20), QTime(14, 0, 0), QTime(14, 45, 0), "Taken session.", true)
		}
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(16, 30, 0), QTime(17, 45, 0), "Dest session.", false),
			Session(QDate(2025, 4, 20), QTime(18, 0, 0), QTime(20, 15, 0), "Dest session.", false)
		};
}

void TestSessions::takeSessions()
{
	SessionModel source_model, dest_model;

	QFETCH(QList<Session>, source);
	for (const Session& session : source) {
		source_model.add(session);
	}

	QFETCH(QList<Session>, dest);
	for (const Session& session : dest) {
		dest_model.add(session);
	}

	QFETCH(QList<int>, take);
	for (int row : take) {
		dest_model.take(&source_model, row);
	}

	QFETCH(QList<Session>, source_result);
	QCOMPARE(source_model.rowCount() - 1, source_result.count());
	for (int i = 0; i < source_result.count(); ++i) {
		QCOMPARE(source_model.session(i), source_result.at(i));
	}

	QFETCH(QList<Session>, dest_result);
	QCOMPARE(dest_model.rowCount() - 1, dest_result.count());
	for (int i = 0; i < dest_result.count(); ++i) {
		QCOMPARE(dest_model.session(i), dest_result.at(i));
	}
}

//-----------------------------------------------------------------------------

void TestSessions::takeMultipleSessions_data()
{
	QTest::addColumn<QList<Session>>("source");
	QTest::addColumn<QList<Session>>("dest");
	QTest::addColumn<QList<int>>("take");
	QTest::addColumn<QList<Session>>("source_result");
	QTest::addColumn<QList<Session>>("dest_result");

	QTest::newRow("Move to empty model")
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(12, 30, 0), QTime(13, 45, 0), "Taken session.", false),
			Session(QDate(2025, 4, 20), QTime(14, 0, 0), QTime(14, 45, 0), "Taken session.", false),
			Session(QDate(2025, 4, 20), QTime(16, 30, 0), QTime(17, 45, 0), "Taken session.", false),
			Session(QDate(2025, 4, 20), QTime(18, 0, 0), QTime(20, 15, 0), "Source session.", false)
		}
		<< QList<Session>{}
		<< QList<int>{
			0,
			1,
			2
		}
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(18, 0, 0), QTime(20, 15, 0), "Source session.", false)
		}
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(12, 30, 0), QTime(13, 45, 0), "Taken session.", false),
			Session(QDate(2025, 4, 20), QTime(14, 0, 0), QTime(14, 45, 0), "Taken session.", false),
			Session(QDate(2025, 4, 20), QTime(16, 30, 0), QTime(17, 45, 0), "Taken session.", false)
		};
	QTest::newRow("Move unsorted to empty model")
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(12, 30, 0), QTime(13, 45, 0), "Source session.", false),
			Session(QDate(2025, 4, 20), QTime(14, 0, 0), QTime(14, 45, 0), "Taken session.", false),
			Session(QDate(2025, 4, 20), QTime(16, 30, 0), QTime(17, 45, 0), "Taken session.", false),
			Session(QDate(2025, 4, 20), QTime(18, 0, 0), QTime(20, 15, 0), "Taken session.", false)
		}
		<< QList<Session>{}
		<< QList<int>{
			3,
			1,
			2
		}
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(12, 30, 0), QTime(13, 45, 0), "Source session.", false)
		}
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(14, 0, 0), QTime(14, 45, 0), "Taken session.", false),
			Session(QDate(2025, 4, 20), QTime(16, 30, 0), QTime(17, 45, 0), "Taken session.", false),
			Session(QDate(2025, 4, 20), QTime(18, 0, 0), QTime(20, 15, 0), "Taken session.", false)
		};
	QTest::newRow("Move after")
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(16, 30, 0), QTime(17, 45, 0), "Source session.", false),
			Session(QDate(2025, 4, 20), QTime(18, 0, 0), QTime(20, 15, 0), "Taken session.", false),
			Session(QDate(2025, 4, 20), QTime(21, 0, 0), QTime(22, 15, 0), "Taken session.", false)
		}
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(12, 30, 0), QTime(13, 45, 0), "Dest session.", false),
			Session(QDate(2025, 4, 20), QTime(14, 0, 0), QTime(14, 45, 0), "Dest session.", false)
		}
		<< QList<int>{
			1,
			2
		}
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(16, 30, 0), QTime(17, 45, 0), "Source session.", false)
		}
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(12, 30, 0), QTime(13, 45, 0), "Dest session.", false),
			Session(QDate(2025, 4, 20), QTime(14, 0, 0), QTime(14, 45, 0), "Dest session.", false),
			Session(QDate(2025, 4, 20), QTime(18, 0, 0), QTime(20, 15, 0), "Taken session.", false),
			Session(QDate(2025, 4, 20), QTime(21, 0, 0), QTime(22, 15, 0), "Taken session.", false)
		};
	QTest::newRow("Move before")
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(10, 30, 0), QTime(11, 45, 0), "Source session.", false),
			Session(QDate(2025, 4, 20), QTime(12, 30, 0), QTime(13, 45, 0), "Taken session.", false),
			Session(QDate(2025, 4, 20), QTime(14, 0, 0), QTime(14, 45, 0), "Taken session.", false),
		}
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(16, 30, 0), QTime(17, 45, 0), "Dest session.", false),
			Session(QDate(2025, 4, 20), QTime(18, 0, 0), QTime(20, 15, 0), "Dest session.", false)
		}
		<< QList<int>{
			1,
			2
		}
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(10, 30, 0), QTime(11, 45, 0), "Source session.", false)
		}
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(12, 30, 0), QTime(13, 45, 0), "Taken session.", false),
			Session(QDate(2025, 4, 20), QTime(14, 0, 0), QTime(14, 45, 0), "Taken session.", false),
			Session(QDate(2025, 4, 20), QTime(16, 30, 0), QTime(17, 45, 0), "Dest session.", false),
			Session(QDate(2025, 4, 20), QTime(18, 0, 0), QTime(20, 15, 0), "Dest session.", false)
		};
	QTest::newRow("Move between")
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(10, 30, 0), QTime(11, 45, 0), "Source session.", false),
			Session(QDate(2025, 4, 20), QTime(14, 0, 0), QTime(14, 45, 0), "Taken session.", false),
			Session(QDate(2025, 4, 20), QTime(16, 30, 0), QTime(17, 45, 0), "Taken session.", false)
		}
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(12, 30, 0), QTime(13, 45, 0), "Dest session.", false),
			Session(QDate(2025, 4, 20), QTime(18, 0, 0), QTime(20, 15, 0), "Dest session.", false)
		}
		<< QList<int>{
			1,
			2
		}
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(10, 30, 0), QTime(11, 45, 0), "Source session.", false)
		}
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(12, 30, 0), QTime(13, 45, 0), "Dest session.", false),
			Session(QDate(2025, 4, 20), QTime(14, 0, 0), QTime(14, 45, 0), "Taken session.", false),
			Session(QDate(2025, 4, 20), QTime(16, 30, 0), QTime(17, 45, 0), "Taken session.", false),
			Session(QDate(2025, 4, 20), QTime(18, 0, 0), QTime(20, 15, 0), "Dest session.", false)
		};
	QTest::newRow("Prevent taking billed session")
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(12, 30, 0), QTime(13, 45, 0), "Taken session.", false),
			Session(QDate(2025, 4, 20), QTime(14, 0, 0), QTime(14, 45, 0), "Taken session.", true),
			Session(QDate(2025, 4, 20), QTime(21, 30, 0), QTime(21, 45, 0), "Taken session.", false),
		}
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(16, 30, 0), QTime(17, 45, 0), "Dest session.", false),
			Session(QDate(2025, 4, 20), QTime(18, 0, 0), QTime(20, 15, 0), "Dest session.", false)
		}
		<< QList<int>{
			0,
			1,
			2
		}
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(12, 30, 0), QTime(13, 45, 0), "Taken session.", false),
			Session(QDate(2025, 4, 20), QTime(14, 0, 0), QTime(14, 45, 0), "Taken session.", true),
			Session(QDate(2025, 4, 20), QTime(21, 30, 0), QTime(21, 45, 0), "Taken session.", false)
		}
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(16, 30, 0), QTime(17, 45, 0), "Dest session.", false),
			Session(QDate(2025, 4, 20), QTime(18, 0, 0), QTime(20, 15, 0), "Dest session.", false)
		};
	QTest::newRow("Prevent moving into billed")
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(12, 30, 0), QTime(13, 45, 0), "Taken session.", false),
			Session(QDate(2025, 4, 20), QTime(14, 0, 0), QTime(14, 45, 0), "Taken session.", false),
			Session(QDate(2025, 4, 20), QTime(21, 30, 0), QTime(21, 45, 0), "Source session.", false)
		}
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(16, 30, 0), QTime(17, 45, 0), "Dest session.", false),
			Session(QDate(2025, 4, 20), QTime(18, 0, 0), QTime(20, 15, 0), "Dest session.", true)
		}
		<< QList<int>{
			0,
			1
		}
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(12, 30, 0), QTime(13, 45, 0), "Taken session.", false),
			Session(QDate(2025, 4, 20), QTime(14, 0, 0), QTime(14, 45, 0), "Taken session.", false),
			Session(QDate(2025, 4, 20), QTime(21, 30, 0), QTime(21, 45, 0), "Source session.", false)
		}
		<< QList<Session>{
			Session(QDate(2025, 4, 20), QTime(16, 30, 0), QTime(17, 45, 0), "Dest session.", false),
			Session(QDate(2025, 4, 20), QTime(18, 0, 0), QTime(20, 15, 0), "Dest session.", true)
		};
}

void TestSessions::takeMultipleSessions()
{
	SessionModel source_model, dest_model;

	QFETCH(QList<Session>, source);
	for (const Session& session : source) {
		source_model.add(session);
	}

	QFETCH(QList<Session>, dest);
	for (const Session& session : dest) {
		dest_model.add(session);
	}

	QFETCH(QList<int>, take);
	dest_model.take(&source_model, take);

	QFETCH(QList<Session>, source_result);
	QCOMPARE(source_model.rowCount() - 1, source_result.count());
	for (int i = 0; i < source_result.count(); ++i) {
		QCOMPARE(source_model.session(i), source_result.at(i));
	}

	QFETCH(QList<Session>, dest_result);
	QCOMPARE(dest_model.rowCount() - 1, dest_result.count());
	for (int i = 0; i < dest_result.count(); ++i) {
		QCOMPARE(dest_model.session(i), dest_result.at(i));
	}
}

//-----------------------------------------------------------------------------

QTEST_MAIN(TestSessions)
