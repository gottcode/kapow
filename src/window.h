/*
	SPDX-FileCopyrightText: 2008-2025 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KAPOW_WINDOW_H
#define KAPOW_WINDOW_H

#include "contact.h"
#include "rates.h"
class Project;
class SessionModel;

#include <QDateTime>
#include <QMainWindow>
#include <QSystemTrayIcon>
class QComboBox;
class QLabel;
class QLineEdit;
class QModelIndex;
class QPushButton;
class QSplitter;
class QTimer;
class QTreeView;
class QTreeWidget;
class QTreeWidgetItem;

class Window : public QMainWindow
{
	Q_OBJECT

public:
	Window(const QString& filename, bool backups_enabled, bool start_in_tray = false, QWidget* parent = nullptr);

	bool isValid() const;

protected:
	bool event(QEvent* event) override;
	void closeEvent(QCloseEvent* event) override;
	void showEvent(QShowEvent* event) override;

private Q_SLOTS:
	void about();
	void quit();
	void setDecimalTotals(bool decimals);
	void setInlineEditing(bool edit);
	void setCloseToTray(bool closetotray);
	void setStartMinimized(bool minimized);
	void setLocaleClicked();
	void start();
	void stop();
	void stopRunningProject();
	void stopAll();
	void cancel();
	void cancelRunningProject();
	void taskChanged(const QString& task);
	void taskStart();
	void updateTime();
	void addProject();
	void editProject();
	void removeProject();
	void createReport();
	void removeReport();
	void viewReports();
	void projectActivated(QTreeWidgetItem* item);
	void projectChanged(QTreeWidgetItem* item, int column);
	void filterChanged(int index);
	void modelBilledStatusChanged();
	void sessionPressed(const QModelIndex& index);
	void sessionsInserted(const QModelIndex& parent, int start, int end);
	void sessionsScrolled(int value);
	void addSession();
	void editSession();
	void removeSession();
	void toggleColumnHidden(int column);
	void save();
	void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
	void toggleVisible();

private:
	int currentRow();
	void loadData();
	void loadData(const QString& filename);
	void createDataBackup();
	void addProject(const QString& name);
	void removeProject(QTreeWidgetItem* item);
	void minimizeToTray();
	void restoreFromTray();
	bool showRunningProject();
	void updateDetails();
	void updateDisplay();
	void updateReportActions();
	void updateSessionButtons();
	void updateTrayActions();
	void updateTrayIcon();
	void updateWindowTitle(const QString& project);

private:
	QString m_filename;
	bool m_valid;
	bool m_blocked;
	bool m_backups_enabled;
	bool m_decimals;
	bool m_inline;
	bool m_closetotray;
	bool m_start_minimized;

	QSplitter* m_contents;
	QTreeWidget* m_projects;
	QLabel* m_display;
	QLineEdit* m_task;
	QPushButton* m_start;
	QPushButton* m_stop;
	QPushButton* m_cancel;
	QComboBox* m_filter;
	QTreeView* m_details;
	Contact m_contact;
	Rates m_rates;

	QTimer* m_timer;
	QTimer* m_save_timer;
	QDateTime m_current_time;

	Project* m_active_project;
	SessionModel* m_active_model;
	QList<Project*> m_active_timers;

	QAction* m_add_project;
	QAction* m_remove_project;
	QAction* m_create_report;
	QAction* m_remove_report;
	QAction* m_view_reports;
	QAction* m_add_session;
	QAction* m_edit_session;
	QAction* m_remove_session;
	QAction* m_start_session;
	QAction* m_stop_session;
	QAction* m_cancel_session;
	QAction* m_tray_start_session;
	QAction* m_tray_stop_session;
	QAction* m_tray_stop_all_session;
	QAction* m_tray_cancel_session;
	QAction* m_toggle_visibility;

	QSystemTrayIcon* m_tray_icon;
	QIcon m_active_icon;
	QIcon m_inactive_icon;
};

#endif // KAPOW_WINDOW_H
