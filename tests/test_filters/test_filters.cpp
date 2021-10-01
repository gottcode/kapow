/*
	SPDX-FileCopyrightText: 2013-2020 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "test_filters.h"

#include "filter_model.h"
#include "session.h"
#include "session_model.h"
Q_DECLARE_METATYPE(Session)

#include <QTest>

//-----------------------------------------------------------------------------

void TestFilters::checkFilters_data()
{
	QTest::addColumn<QList<Session>>("sessions");
	QTest::addColumn<int>("type");
	QTest::addColumn<QList<Session>>("result");

	QDate today = QDate::currentDate();
	QTest::newRow("Filter for billed")
		<< QList<Session>{
			Session(today, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(today, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true),
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		}
		<< int(FilterModel::Unbilled)
		<< QList<Session>{
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		};

	QDate before = today.addYears(-1);
	QTest::newRow("Filter for this year")
		<< QList<Session>{
			Session(before, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(before, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false),
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		}
		<< int(FilterModel::ThisYear)
		<< QList<Session>{
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		};

	before = today.addMonths(-1);
	QTest::newRow("Filter for this month")
		<< QList<Session>{
			Session(before, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(before, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false),
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		}
		<< int(FilterModel::ThisMonth)
		<< QList<Session>{
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		};

	before = today.addDays(-8);
	QTest::newRow("Filter for this week")
		<< QList<Session>{
			Session(before, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(before, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false),
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		}
		<< int(FilterModel::ThisWeek)
		<< QList<Session>{
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		};
}

void TestFilters::checkFilters()
{
	SessionModel session_model;

	QFETCH(QList<Session>, sessions);
	for (const Session& session : sessions) {
		session_model.add(session);
	}

	FilterModel model(&session_model);
	QFETCH(int, type);
	model.setType(type);

	QFETCH(QList<Session>, result);
	QCOMPARE(model.rowCount() - 1, result.count());
	for (int i = 0; i < result.count(); ++i) {
		int row = model.mapToSource(model.index(i, 0)).row();
		QCOMPARE(session_model.session(row), result.at(i));
	}
}

//-----------------------------------------------------------------------------

void TestFilters::addSessions_data()
{
	QTest::addColumn<QList<Session>>("start_sessions");
	QTest::addColumn<int>("type");
	QTest::addColumn<QList<Session>>("add_sessions");
	QTest::addColumn<QList<Session>>("result");

	QDate today = QDate::currentDate();
	QTest::newRow("Filtered by billed")
		<< QList<Session>{
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		}
		<< int(FilterModel::Unbilled)
		<< QList<Session>{
			Session(today, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(today, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true)
		}
		<< QList<Session>{
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		};
	QTest::newRow("Unfiltered by billed")
		<< QList<Session>{
			Session(today, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(today, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true)
		}
		<< int(FilterModel::Unbilled)
		<< QList<Session>{
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		}
		<< QList<Session>{
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		};


	QDate before = today.addYears(-1);
	QTest::newRow("Filtered by this year")
		<< QList<Session>{
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		}
		<< int(FilterModel::ThisYear)
		<< QList<Session>{
			Session(before, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(before, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false)
		}
		<< QList<Session>{
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		};
	QTest::newRow("Unfiltered by this year")
		<< QList<Session>{
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		}
		<< int(FilterModel::ThisYear)
		<< QList<Session>{
			Session(today, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(today, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false)
		}
		<< QList<Session>{
			Session(today, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(today, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false),
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		};

	before = today.addMonths(-1);
	QTest::newRow("Filtered by this month")
		<< QList<Session>{
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		}
		<< int(FilterModel::ThisMonth)
		<< QList<Session>{
			Session(before, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(before, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false)
		}
		<< QList<Session>{
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		};
	QTest::newRow("Unfiltered by this month")
		<< QList<Session>{
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		}
		<< int(FilterModel::ThisMonth)
		<< QList<Session>{
			Session(today, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(today, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false)
		}
		<< QList<Session>{
			Session(today, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(today, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false),
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		};

	before = today.addDays(-8);
	QTest::newRow("Filtered by this week")
		<< QList<Session>{
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		}
		<< int(FilterModel::ThisWeek)
		<< QList<Session>{
			Session(before, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(before, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false)
		}
		<< QList<Session>{
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		};
	QTest::newRow("Unfiltered by this week")
		<< QList<Session>{
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		}
		<< int(FilterModel::ThisWeek)
		<< QList<Session>{
			Session(today, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(today, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false)
		}
		<< QList<Session>{
			Session(today, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(today, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false),
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		};
}

void TestFilters::addSessions()
{
	SessionModel session_model;

	QFETCH(QList<Session>, start_sessions);
	for (const Session& session : start_sessions) {
		session_model.add(session);
	}

	FilterModel model(&session_model);
	QFETCH(int, type);
	model.setType(type);

	QFETCH(QList<Session>, add_sessions);
	for (const Session& session : add_sessions) {
		session_model.add(session);
	}

	QFETCH(QList<Session>, result);
	QCOMPARE(model.rowCount() - 1, result.count());
	for (int i = 0; i < result.count(); ++i) {
		int row = model.mapToSource(model.index(i, 0)).row();
		QCOMPARE(session_model.session(row), result.at(i));
	}
}

//-----------------------------------------------------------------------------

void TestFilters::editSessions_data()
{
	QTest::addColumn<QList<Session>>("sessions");
	QTest::addColumn<int>("type");
	QTest::addColumn<int>("position");
	QTest::addColumn<Session>("replacement");
	QTest::addColumn<QList<Session>>("result");

	QDate today = QDate::currentDate();
	QTest::newRow("Unfiltered by billed")
		<< QList<Session>{
			Session(today, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(today, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true),
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		}
		<< int(FilterModel::Unbilled)
		<< 2
		<< Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		<< QList<Session>{
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		};

	QDate before = today.addYears(-1);
	QTest::newRow("Filtered by this year")
		<< QList<Session>{
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		}
		<< int(FilterModel::ThisYear)
		<< 1
		<< Session(before, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		<< QList<Session>{
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		};
	QTest::newRow("Unfiltered by this year")
		<< QList<Session>{
			Session(before, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false),
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		}
		<< int(FilterModel::ThisYear)
		<< 0
		<< Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		<< QList<Session>{
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		};

	before = today.addMonths(-1);
	QTest::newRow("Filtered by this month")
		<< QList<Session>{
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		}
		<< int(FilterModel::ThisMonth)
		<< 1
		<< Session(before, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		<< QList<Session>{
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		};
	QTest::newRow("Unfiltered by this month")
		<< QList<Session>{
			Session(before, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false),
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		}
		<< int(FilterModel::ThisMonth)
		<< 0
		<< Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		<< QList<Session>{
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		};

	before = today.addDays(-8);
	QTest::newRow("Filtered by this week")
		<< QList<Session>{
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		}
		<< int(FilterModel::ThisWeek)
		<< 1
		<< Session(before, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		<< QList<Session>{
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		};
	QTest::newRow("Unfiltered by this week")
		<< QList<Session>{
			Session(before, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false),
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		}
		<< int(FilterModel::ThisWeek)
		<< 0
		<< Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		<< QList<Session>{
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		};
}

void TestFilters::editSessions()
{
	SessionModel session_model;

	QFETCH(QList<Session>, sessions);
	for (const Session& session : sessions) {
		session_model.add(session);
	}

	FilterModel model(&session_model);
	QFETCH(int, type);
	model.setType(type);

	QFETCH(int, position);
	QFETCH(Session, replacement);
	session_model.edit(position, replacement);

	QFETCH(QList<Session>, result);
	QCOMPARE(model.rowCount() - 1, result.count());
	for (int i = 0; i < result.count(); ++i) {
		int row = model.mapToSource(model.index(i, 0)).row();
		QCOMPARE(session_model.session(row), result.at(i));
	}
}

//-----------------------------------------------------------------------------

void TestFilters::removeSessions_data()
{
	QTest::addColumn<QList<Session>>("sessions");
	QTest::addColumn<int>("type");
	QTest::addColumn<int>("position");
	QTest::addColumn<QList<Session>>("result");

	QDate today = QDate::currentDate();
	QTest::newRow("Unfiltered by billed")
		<< QList<Session>{
			Session(today, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(today, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true),
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		}
		<< int(FilterModel::Unbilled)
		<< 2
		<< QList<Session>{
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		};

	QDate before = today.addYears(-1);
	QTest::newRow("Filtered by this year")
		<< QList<Session>{
			Session(before, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(before, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false),
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		}
		<< int(FilterModel::ThisYear)
		<< 1
		<< QList<Session>{
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		};
	QTest::newRow("Unfiltered by this year")
		<< QList<Session>{
			Session(before, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(before, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false),
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		}
		<< int(FilterModel::ThisYear)
		<< 2
		<< QList<Session>{
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		};

	before = today.addMonths(-1);
	QTest::newRow("Filtered by this month")
		<< QList<Session>{
			Session(before, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(before, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false),
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		}
		<< int(FilterModel::ThisMonth)
		<< 1
		<< QList<Session>{
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		};
	QTest::newRow("Unfiltered by this month")
		<< QList<Session>{
			Session(before, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(before, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false),
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		}
		<< int(FilterModel::ThisMonth)
		<< 2
		<< QList<Session>{
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		};

	before = today.addDays(-8);
	QTest::newRow("Filtered by this week")
		<< QList<Session>{
			Session(before, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(before, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false),
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		}
		<< int(FilterModel::ThisWeek)
		<< 1
		<< QList<Session>{
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		};
	QTest::newRow("Unfiltered by this week")
		<< QList<Session>{
			Session(before, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(before, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false),
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		}
		<< int(FilterModel::ThisWeek)
		<< 2
		<< QList<Session>{
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		};
}

void TestFilters::removeSessions()
{
	SessionModel session_model;

	QFETCH(QList<Session>, sessions);
	for (const Session& session : sessions) {
		session_model.add(session);
	}

	FilterModel model(&session_model);
	QFETCH(int, type);
	model.setType(type);

	QFETCH(int, position);
	session_model.remove(position);

	QFETCH(QList<Session>, result);
	QCOMPARE(model.rowCount() - 1, result.count());
	for (int i = 0; i < result.count(); ++i) {
		int row = model.mapToSource(model.index(i, 0)).row();
		QCOMPARE(session_model.session(row), result.at(i));
	}
}

//-----------------------------------------------------------------------------

void TestFilters::unbilledFilter_data()
{
	QTest::addColumn<QList<Session>>("sessions");
	QTest::addColumn<int>("position");
	QTest::addColumn<bool>("billed");
	QTest::addColumn<QList<Session>>("result");

	QDate today = QDate::currentDate();
	QTest::newRow("Bill sessions")
		<< QList<Session>{
			Session(today, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(today, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false),
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		}
		<< 1
		<< true
		<< QList<Session>{
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		};
	QTest::newRow("Unbill sessions")
		<< QList<Session>{
			Session(today, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(today, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true),
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		}
		<< 1
		<< false
		<< QList<Session>{
			Session(today, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(today, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false),
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		};
}

void TestFilters::unbilledFilter()
{
	SessionModel session_model;

	QFETCH(QList<Session>, sessions);
	for (const Session& session : sessions) {
		session_model.add(session);
	}

	FilterModel model(&session_model);
	model.setType(FilterModel::Unbilled);

	QFETCH(int, position);
	QFETCH(bool, billed);
	session_model.setBilled(position, billed);

	QFETCH(QList<Session>, result);
	QCOMPARE(model.rowCount() - 1, result.count());
	for (int i = 0; i < result.count(); ++i) {
		int row = model.mapToSource(model.index(i, 0)).row();
		QCOMPARE(session_model.session(row), result.at(i));
	}
}

//-----------------------------------------------------------------------------

void TestFilters::mapUnbilled_data()
{
	QTest::addColumn<QList<Session>>("sessions");
	QTest::addColumn<int>("type");
	QTest::addColumn<int>("first_unbilled_row");
	QTest::addColumn<QList<int>>("mapped_rows");

	QDate today = QDate::currentDate();
	QTest::newRow("Filter for billed")
		<< QList<Session>{
			Session(today, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(today, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true),
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		}
		<< int(FilterModel::Unbilled)
		<< 0
		<< QList<int>{
			2,
			3
		};

	QDate before = today.addYears(-1);
	QTest::newRow("Filter for this year")
		<< QList<Session>{
			Session(before, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(before, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false),
			Session(today, QTime(15, 0, 0), QTime(16, 0, 0), "Test session.", true),
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		}
		<< int(FilterModel::ThisYear)
		<< 1
		<< QList<int>{
			3,
			4
		};

	before = today.addMonths(-1);
	QTest::newRow("Filter for this month")
		<< QList<Session>{
			Session(before, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(before, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false),
			Session(today, QTime(15, 0, 0), QTime(16, 0, 0), "Test session.", true),
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		}
		<< int(FilterModel::ThisMonth)
		<< 1
		<< QList<int>{
			3,
			4
		};

	before = today.addDays(-8);
	QTest::newRow("Filter for this week")
		<< QList<Session>{
			Session(before, QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(before, QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false),
			Session(today, QTime(15, 0, 0), QTime(16, 0, 0), "Test session.", true),
			Session(today, QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false),
			Session(today, QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false)
		}
		<< int(FilterModel::ThisWeek)
		<< 1
		<< QList<int>{
			3,
			4
		};
}

void TestFilters::mapUnbilled()
{
	SessionModel session_model;

	QFETCH(QList<Session>, sessions);
	for (const Session& session : sessions) {
		session_model.add(session);
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

	QFETCH(QList<int>, mapped_rows);
	QCOMPARE(model.rowCount() - 1, first_unbilled_row + mapped_rows.count());
	for (int i = 0; i < mapped_rows.count(); ++i) {
		int proxy_row = i + first_unbilled_row;
		int row = model.mapUnbilledToSource(model.index(proxy_row, 0)).row();
		QCOMPARE(row, mapped_rows.at(i));
	}
}

//-----------------------------------------------------------------------------

QTEST_MAIN(TestFilters)
