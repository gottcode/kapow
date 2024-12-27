/*
	SPDX-FileCopyrightText: 2008-2023 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "window.h"

#include "contact.h"
#include "date_editor.h"
#include "filter_model.h"
#include "locale_dialog.h"
#include "project.h"
#include "project_delegate.h"
#include "report.h"
#include "session.h"
#include "session_delegate.h"
#include "session_dialog.h"
#include "session_model.h"
#include "settings.h"
#include "time_editor.h"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QComboBox>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QItemEditorFactory>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMetaProperty>
#include <QPushButton>
#include <QSaveFile>
#include <QScrollBar>
#include <QSplitter>
#include <QStack>
#include <QTextDocument>
#include <QTimer>
#include <QTreeView>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QXmlStreamReader>

#include <algorithm>

//-----------------------------------------------------------------------------

Window::Window(const QString& filename, bool backups_enabled, QWidget* parent)
	: QMainWindow(parent)
	, m_filename(filename)
	, m_valid(true)
	, m_blocked(false)
	, m_backups_enabled(backups_enabled)
	, m_dark_mode(true)
	, m_decimals(true)
	, m_inline(true)
	, m_closetotray(false)
	, m_active_project(nullptr)
	, m_active_model(nullptr)
{
	m_theme_manager = new ThemeManager();

	QWidget* contents = new QWidget(this);
	setCentralWidget(contents);

	m_display = new QLabel(tr("00:00:00"), contents);
	QFont font = m_display->font();
	font.setPointSize(32);
	m_display->setFont(font);

	m_current_time = QDateTime::currentDateTime();
	m_timer = new QTimer(this);
	m_timer->setInterval(1000);
	connect(m_timer, &QTimer::timeout, this, &Window::updateTime);
	m_timer->start();

	m_save_timer = new QTimer(this);
	m_save_timer->setInterval(30000);
	connect(m_save_timer, &QTimer::timeout, this, &Window::save);

	m_task = new QLineEdit(contents);
	m_task->setPlaceholderText(SessionModel::tr("Task"));
	m_task->setFocus();
	connect(m_task, &QLineEdit::textChanged, this, &Window::taskChanged);
	connect(m_task, &QLineEdit::returnPressed, this, &Window::taskStart);

	m_start = new QPushButton(tr("Start"), contents);
	m_start->setAutoDefault(true);
	connect(m_start, &QPushButton::clicked, this, &Window::start);
	int button_width = m_start->sizeHint().width();

	m_start_session = new QAction(tr("Start"), this);
	m_start_session->setEnabled(true);
	connect(m_start_session, &QAction::triggered, this, &Window::start);

	m_stop = new QPushButton(tr("Stop"), contents);
	m_stop->setAutoDefault(true);
	m_stop->hide();
	connect(m_stop, &QPushButton::clicked, this, &Window::stop);
	button_width = std::max(m_stop->sizeHint().width(), button_width);

	m_stop_session = new QAction(tr("Stop"), this);
	m_stop_session->setEnabled(false);
	connect(m_stop_session, &QAction::triggered, this, &Window::stop);

	m_cancel = new QPushButton(tr("Cancel"), contents);
	m_cancel->setAutoDefault(true);
	m_cancel->setEnabled(false);
	connect(m_cancel, &QPushButton::clicked, this, &Window::cancel);
	button_width = std::max(m_cancel->sizeHint().width(), button_width);

	m_cancel_session = new QAction(tr("Cancel"), this);
	m_cancel_session->setEnabled(false);
	connect(m_cancel_session, &QAction::triggered, this, &Window::cancel);

	m_start->setMinimumWidth(button_width);
	m_stop->setMinimumWidth(button_width);
	m_cancel->setMinimumWidth(button_width);

	m_toggle_visibility = new QAction(tr("&Minimize"), this);
	connect(m_toggle_visibility, &QAction::triggered, this, &Window::toggleVisible);

	// Load settings
	Settings settings;
	m_dark_mode = settings.value("DarkMode", true).toBool();
	m_decimals = settings.value("DecimalTotals", true).toBool();
	m_inline = settings.value("InlineEditing", true).toBool();
	m_closetotray = settings.value("CloseToTray", false).toBool();

	// Create menus
	QMenu* menu = menuBar()->addMenu(tr("&Project"));
	m_add_project = menu->addAction(tr("&Add"), this, qOverload<>(&Window::addProject));
	m_add_project->setShortcut(tr("Ctrl+Shift+N"));
	m_remove_project = menu->addAction(tr("&Remove"), this, qOverload<>(&Window::removeProject));
	m_remove_project->setShortcut(tr("Ctrl+Shift+Delete"));
	m_remove_project->setEnabled(false);
	menu->addSeparator();
	m_create_report = menu->addAction(tr("&Create Report..."), this, &Window::createReport);
	m_create_report->setEnabled(false);
	m_remove_report = menu->addAction(tr("Re&move Report"), this, &Window::removeReport);
	m_remove_report->setEnabled(false);
	m_view_reports = menu->addAction(tr("View R&eports"), this, &Window::viewReports);
	m_view_reports->setEnabled(false);
	menu->addSeparator();
	QAction* quit_action = menu->addAction(tr("&Quit"), this, &Window::quit);
	quit_action->setShortcut(tr("Ctrl+Q"));
	quit_action->setMenuRole(QAction::QuitRole);

	menu = menuBar()->addMenu(tr("&Session"));
	m_add_session = menu->addAction(tr("&Add"), this, &Window::addSession);
	m_add_session->setShortcut(QKeySequence::New);
	m_edit_session = menu->addAction(tr("&Edit"), this, &Window::editSession);
	m_edit_session->setEnabled(false);
	m_remove_session = menu->addAction(tr("&Remove"), this, &Window::removeSession);
	m_remove_session->setShortcut(tr("Ctrl+Delete"));
	m_remove_session->setEnabled(false);

	menu = menuBar()->addMenu(tr("S&ettings"));
	QMenu* column_menu = menu->addMenu(tr("Columns"));
	menu->addSeparator();

	QAction *action = menu->addAction(tr("&Dark Mode"));
	action->setCheckable(true);
	action->setChecked(m_dark_mode);
	connect(action, &QAction::toggled, this, &Window::setDarkMode);
	Window::setDarkMode(m_dark_mode); // Set the dark/light mode on startup
	menu->addSeparator();
	action = menu->addAction(tr("&Decimal Totals"));
	action->setCheckable(true);
	action->setChecked(m_decimals);
	connect(action, &QAction::toggled, this, &Window::setDecimalTotals);
	action = menu->addAction(tr("&Inline Editing"));
	action->setCheckable(true);
	action->setChecked(m_inline);
	connect(action, &QAction::toggled, this, &Window::setInlineEditing);
	action = menu->addAction(tr("&Close to Tray"));
	action->setCheckable(true);
	action->setChecked(m_closetotray);
	connect(action, &QAction::toggled, this, &Window::setCloseToTray);
	menu->addSeparator();
	menu->addAction(tr("Application &Language..."), this, &Window::setLocaleClicked);

	menu = menuBar()->addMenu(tr("&Help"));
	action = menu->addAction(tr("&About"), this, &Window::about);
	action->setMenuRole(QAction::AboutRole);
	action = menu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
	action->setMenuRole(QAction::AboutQtRole);

	QAction* actions_separator = new QAction(this);
	actions_separator->setSeparator(true);

	QMenu* context_menu = new QMenu(this);
    // Add start and stop timer in the tray icon
    context_menu->addAction(m_start_session);
    context_menu->addAction(m_stop_session);

    context_menu->addSeparator();

	context_menu->addAction(m_toggle_visibility);
	context_menu->addAction(quit_action);

	// Create projects
	m_projects = new QTreeWidget(contents);
	m_projects->setAutoExpandDelay(500);
	m_projects->setDragEnabled(true);
	m_projects->setDragDropMode(QAbstractItemView::InternalMove);
	m_projects->setHeaderLabels(QStringList() << tr("Project") << tr("Timer"));
	m_projects->setItemDelegate(new ProjectDelegate(m_projects));
	m_projects->setSelectionMode(QAbstractItemView::SingleSelection);
	m_projects->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_projects->setSortingEnabled(true);
	m_projects->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	m_projects->sortByColumn(0, Qt::AscendingOrder);
	m_projects->header()->setSortIndicatorShown(false);
	m_projects->header()->setStretchLastSection(false);
	m_projects->header()->setSectionsClickable(false);
	m_projects->header()->setSectionsMovable(false);
	m_projects->header()->setSectionResizeMode(0, QHeaderView::Stretch);
	m_projects->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	m_projects->addAction(m_add_project);
	m_projects->addAction(m_remove_project);
	m_projects->addAction(actions_separator);
	m_projects->addAction(m_start_session);
	m_projects->addAction(m_stop_session);
	m_projects->addAction(m_cancel_session);
	m_projects->setContextMenuPolicy(Qt::ActionsContextMenu);
	connect(m_projects, &QTreeWidget::currentItemChanged, this, &Window::projectActivated);
	connect(m_projects, &QTreeWidget::itemChanged, this, &Window::projectChanged);

	// Create details
	QItemEditorFactory* factory = new QItemEditorFactory;
	factory->registerEditor(QMetaType::QDate, new QStandardItemEditorCreator<DateEditor>());
	factory->registerEditor(QMetaType::QTime, new QStandardItemEditorCreator<TimeEditor>());
	factory->registerEditor(QMetaType::QString, new QStandardItemEditorCreator<QLineEdit>());
	QItemEditorFactory::setDefaultFactory(factory);

	QWidget* details = new QWidget(contents);

	m_filter = new QComboBox(details);
	m_filter->addItem(tr("Show all"), FilterModel::All);
	m_filter->addItem(tr("Show only unbilled"), FilterModel::Unbilled);
	m_filter->addItem(tr("Show only this year"), FilterModel::ThisYear);
	m_filter->addItem(tr("Show only this month"), FilterModel::ThisMonth);
	m_filter->addItem(tr("Show only this week"), FilterModel::ThisWeek);
	connect(m_filter, &QComboBox::activated, this, &Window::filterChanged);

	m_details = new QTreeView(details);
	m_details->setUniformRowHeights(true);
	m_details->setRootIsDecorated(false);
	m_details->setItemsExpandable(false);
	m_details->setIndentation(0);
	m_details->setSelectionMode(QAbstractItemView::SingleSelection);
	m_details->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_details->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	m_details->header()->setSectionsClickable(false);
	m_details->header()->setSectionsMovable(false);
	m_details->addAction(m_add_session);
	m_details->addAction(m_edit_session);
	m_details->addAction(m_remove_session);
	m_details->addAction(actions_separator);
	m_details->setContextMenuPolicy(Qt::ActionsContextMenu);
	connect(m_details, &QTreeView::activated, this, &Window::editSession);
	connect(m_details, &QTreeView::pressed, this, &Window::sessionPressed);
	connect(m_details->verticalScrollBar(), &QScrollBar::valueChanged, this, &Window::sessionsScrolled);
	m_details->setEditTriggers(QAbstractItemView::NoEditTriggers);

	SessionDelegate* delegate = new SessionDelegate(m_details);
	delegate->setDevicePixelRatio(devicePixelRatio());
	m_details->setItemDelegate(delegate);

	QVBoxLayout* details_layout = new QVBoxLayout(details);
	details_layout->setContentsMargins(0, 0, 0, 0);
	details_layout->addWidget(m_filter);
	details_layout->addWidget(m_details);

	// Create contents splitter
	m_contents = new QSplitter(contents);
	m_contents->addWidget(m_projects);
	m_contents->addWidget(details);
	m_contents->setStretchFactor(0, 0);
	m_contents->setStretchFactor(1, 1);

	// Add tray icon
	m_active_icon = QIcon(":/kapow.png");
	QList<QSize> sizes = m_active_icon.availableSizes();
	for (const QSize& size : sizes) {
		m_inactive_icon.addPixmap(m_active_icon.pixmap(size, QIcon::Disabled));
	}
	m_tray_icon = new QSystemTrayIcon(m_inactive_icon, this);
	m_tray_icon->setContextMenu(context_menu);
	updateTrayIcon();
	connect(m_tray_icon, &QSystemTrayIcon::activated, this, &Window::trayIconActivated);
	m_tray_icon->show();

	// Lay out window
	QHBoxLayout* session_buttons = new QHBoxLayout;
	session_buttons->setContentsMargins(0, 0, 0, 0);
	session_buttons->setSpacing(0);
	session_buttons->addStretch(1);
	session_buttons->addWidget(m_task, 2);
	session_buttons->addSpacing(12);
	session_buttons->addWidget(m_start);
	session_buttons->addWidget(m_stop);
	session_buttons->addSpacing(6);
	session_buttons->addWidget(m_cancel);
	session_buttons->addStretch(1);

	QVBoxLayout* session_layout = new QVBoxLayout;
	session_layout->setContentsMargins(0, 0, 0, 0);
	session_layout->setSpacing(0);
	session_layout->addWidget(m_display, 0, Qt::AlignCenter);
	session_layout->addLayout(session_buttons);

	QVBoxLayout* layout = new QVBoxLayout(contents);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addLayout(session_layout, 0);
	layout->addSpacing(6);
	layout->addWidget(m_contents, 1);

	// Restore window geometry
	resize(800, 600);
	restoreGeometry(settings.value("WindowGeometry").toByteArray());
	m_contents->restoreState(settings.value("SplitterSizes").toByteArray());
	show();

	// Load details of all projects
	loadData();
	if (!m_valid) {
		return;
	}

	// Add column actions
	QList<QAction*> column_actions;
	for (int i = 3; i < 10; ++i) {
		action = column_menu->addAction(m_active_model->headerData(i, Qt::Horizontal).toString());
		action->setCheckable(true);
		action->setChecked(true);
		connect(action, &QAction::triggered, this, [this, i] { toggleColumnHidden(i); });
		column_actions.append(action);
	}
	m_details->header()->addActions(column_actions);
	m_details->header()->setContextMenuPolicy(Qt::ActionsContextMenu);
	m_details->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
	m_details->header()->setSectionResizeMode(3, QHeaderView::Stretch);
	m_details->header()->setStretchLastSection(false);

	// Restore hidden columns
	QStringList hidden = settings.value("HiddenColumns", QStringList() << "5" << "6" << "7" << "8" << "9").toStringList();
	for (const QString& column : hidden) {
		int i = column.toInt();
		m_details->setColumnHidden(i, true);
		column_actions[i - 3]->setChecked(false);
	}
}

//-----------------------------------------------------------------------------

bool Window::isValid() const
{
	return m_valid;
}

//-----------------------------------------------------------------------------

bool Window::event(QEvent* event)
{
	if (event->type() == QEvent::WindowBlocked) {
		m_blocked = true;
	} else if (event->type() == QEvent::WindowUnblocked) {
		m_blocked = false;
	}
	return QMainWindow::event(event);
}

//-----------------------------------------------------------------------------

void Window::closeEvent(QCloseEvent* event)
{
	Settings settings;
	settings.setValue("WindowGeometry", saveGeometry());
	settings.setValue("SplitterSizes", m_contents->saveState());

	bool visible = isVisible();

	if (visible && m_closetotray) {
		event->ignore();
		toggleVisible();
		return;
	}

	if (!m_active_timers.isEmpty()) {
		// Show window
		if (!visible) {
			show();
		}
		raise();
		activateWindow();

		// Prompt user about running timers
		if (QMessageBox::question(this, tr("Question"), tr("There are timers running. Stop timers and quit?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
			for (Project* project : std::as_const(m_active_timers)) {
				project->stop(m_current_time);
			}
			m_active_timers.clear();
		} else {
			event->ignore();
			if (!visible) {
				hide();
			}
			return;
		}
	}

	m_save_timer->stop();
	save();
	event->accept();

	// Remove tray icon so that app will quit
	delete m_tray_icon;
	m_tray_icon = nullptr;
}

//-----------------------------------------------------------------------------

void Window::showEvent(QShowEvent* event)
{
	QMainWindow::showEvent(event);
	m_task->setFocus();
}

//-----------------------------------------------------------------------------

void Window::about()
{
	QMessageBox::about(this, tr("About"), QString("<p align='center'><big><b>%1 %2</b></big><br/>%3<br/><small>%4</small><br/><small>%5</small></p>")
		.arg(tr("Kapow Punch Clock"), QCoreApplication::applicationVersion(),
			tr("A program to help track spent time"),
			tr("Copyright &copy; 2008-%1 Graeme Gott").arg("2024"),
			tr("Released under the <a href=%1>GPL 3</a> license").arg("\"http://www.gnu.org/licenses/gpl.html\""))
	);
}

//-----------------------------------------------------------------------------

void Window::quit()
{
	const bool closetotray = m_closetotray;
	m_closetotray = false;
	if (close()) {
		QCoreApplication::quit();
	} else {
		m_closetotray = closetotray;
	}
}

//-----------------------------------------------------------------------------

void Window::setDarkMode(bool is_dark_mode)
{
	m_dark_mode = is_dark_mode;
	m_theme_manager->setTheme(qApp, m_dark_mode);
	Settings().setValue("DarkMode", m_dark_mode);
}

//-----------------------------------------------------------------------------

void Window::setDecimalTotals(bool decimals)
{
	m_decimals = decimals;
	int count = m_projects->topLevelItemCount();
	for (int i = 0; i < count; ++i) {
		Project* project = dynamic_cast<Project*>(m_projects->topLevelItem(i));
		if (project) {
			project->setDecimalTotals(m_decimals);
		}
	}
	Settings().setValue("DecimalTotals", m_decimals);
}

//-----------------------------------------------------------------------------

void Window::setInlineEditing(bool edit)
{
	m_inline = edit;
	if (!edit) {
		m_details->closePersistentEditor(m_details->currentIndex());
	}
	sessionPressed(m_details->currentIndex());
	Settings().setValue("InlineEditing", edit);
}

//-----------------------------------------------------------------------------

void Window::setCloseToTray(bool closetotray)
{
	m_closetotray = closetotray;
	Settings().setValue("CloseToTray", closetotray);
}

//-----------------------------------------------------------------------------

void Window::setLocaleClicked()
{
	LocaleDialog dialog(this);
	dialog.exec();
}

//-----------------------------------------------------------------------------

void Window::start()
{
	if (!m_active_project->start(m_current_time)) {
		QMessageBox::warning(this, tr("Error"), tr("Session conflicts with other sessions."));
		return;
	}
	m_active_timers += m_active_project;
	m_remove_project->setEnabled(false);
	updateDetails();
	m_stop->setFocus();

	m_start_session->setEnabled(false);
	m_stop_session->setEnabled(true);
	m_cancel_session->setEnabled(true);

	if (!m_save_timer->isActive()) {
		m_save_timer->start();
	}
}

//-----------------------------------------------------------------------------

void Window::stop()
{
	if (!m_active_project->stop(m_current_time)) {
		QMessageBox::warning(this, tr("Error"), tr("Session conflicts with other sessions."));
	}
	m_active_timers.removeAll(m_active_project);
	m_remove_project->setEnabled(true);
	updateDetails();
	m_task->clear();
	m_task->setFocus();

	m_start_session->setEnabled(true);
	m_stop_session->setEnabled(false);
	m_cancel_session->setEnabled(false);

	if (m_active_timers.isEmpty()) {
		m_save_timer->stop();
		save();
	}
}

//-----------------------------------------------------------------------------

void Window::cancel()
{
	if (QMessageBox::question(this, tr("Question"), tr("Cancel this session?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
		m_active_project->stop();
		m_active_timers.removeAll(m_active_project);
		m_remove_project->setEnabled(true);
		updateDetails();
		m_task->clear();
		m_task->setFocus();

		m_start_session->setEnabled(true);
		m_stop_session->setEnabled(false);
		m_cancel_session->setEnabled(false);

		if (m_active_timers.isEmpty()) {
			m_save_timer->stop();
			save();
		}
	}
}

//-----------------------------------------------------------------------------

void Window::taskChanged(const QString& task)
{
	m_active_project->setTask(task);
}

//-----------------------------------------------------------------------------

void Window::taskStart()
{
	if (m_start->isVisible()) {
		start();
	}
}

//-----------------------------------------------------------------------------

void Window::updateTime()
{
	m_current_time = QDateTime::currentDateTime();
	int count = m_projects->topLevelItemCount();
	for (int i = 0; i < count; ++i) {
		Project* project = dynamic_cast<Project*>(m_projects->topLevelItem(i));
		if (project) {
			project->updateTime(m_current_time);
		}
	}
	if (m_active_project) {
		updateDisplay();
	}
	if (m_active_timers.count() == 1) {
		m_tray_icon->setToolTip(m_active_timers.first()->time());
	}
}

//-----------------------------------------------------------------------------

void Window::addProject()
{
	bool ok;
	QString name;
	Q_FOREVER {
		name = QInputDialog::getText(this, tr("Add Project"), tr("Enter project name:"), QLineEdit::Normal, name, &ok);
		if (ok && !name.isEmpty()) {
			if (m_projects->findItems(name, Qt::MatchExactly).isEmpty()) {
				addProject(name);
				break;
			} else {
				QMessageBox::warning(this, tr("Error"), tr("A project with that name already exists."));
			}
		} else {
			break;
		}
	}
}

//-----------------------------------------------------------------------------

void Window::editProject()
{
	m_projects->edit(m_projects->currentIndex());
}

//-----------------------------------------------------------------------------

void Window::removeProject()
{
	if (QMessageBox::question(this, tr("Question"), tr("Remove selected project?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
		m_remove_project->setEnabled(false);
		m_remove_session->setEnabled(false);
		removeProject(m_projects->currentItem());
	}
}

//-----------------------------------------------------------------------------

void Window::createReport()
{
	const int last = m_active_model->rowCount() - 2;
	if (last < 0) {
		return;
	}
	int current = currentRow();
	if ((current == -1) || m_active_model->isBilled(current) || (current > last)) {
		current = last;
	}
	Report report(m_active_model, current, &m_contact, &m_rates, this);
	report.exec();
}

//-----------------------------------------------------------------------------

void Window::removeReport()
{
	const auto billed = m_active_model->billedRows();
	if (billed.isEmpty()) {
		return;
	}

	if (QMessageBox::question(this, tr("Question"), Report::tr("Remove newest report?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No) {
		return;
	}
	m_active_model->setBilled(billed.last(), false);
}

//-----------------------------------------------------------------------------

void Window::viewReports()
{
	int current = currentRow();
	if ((current == (m_active_model->rowCount() - 1)) || !m_active_model->isBilled(current)) {
		current = -1;
	}
	Report report(m_active_model, current, &m_contact, &m_rates, this);
	report.exec();
}

//-----------------------------------------------------------------------------

void Window::projectActivated(QTreeWidgetItem* item)
{
	Project* project = dynamic_cast<Project*>(item);
	if (!project) {
		addProject(tr("Untitled"));
		return;
	}

	if (m_active_model) {
		disconnect(m_active_model, &SessionModel::billedStatusChanged, this, &Window::modelBilledStatusChanged);
		disconnect(m_active_model, &SessionModel::rowsInserted, this, &Window::sessionsInserted);
	}
	m_active_project = project;
	m_active_model = m_active_project->model();
	connect(m_active_model, &SessionModel::billedStatusChanged, this, &Window::modelBilledStatusChanged);
	connect(m_active_model, &SessionModel::rowsInserted, this, &Window::sessionsInserted);

	m_details->setModel(m_active_project->filterModel());
	m_details->expandAll();
	m_filter->setCurrentIndex(m_filter->findData(m_active_project->filterModel()->type()));

	updateDisplay();
	updateSessionButtons();
	m_remove_project->setEnabled(!m_active_project->isActive());
	if (!m_active_project->time().isEmpty()) {
		m_start_session->setEnabled(false);
		m_stop_session->setEnabled(true);
		m_cancel_session->setEnabled(true);
	} else {
		m_start_session->setEnabled(true);
		m_stop_session->setEnabled(false);
		m_cancel_session->setEnabled(false);
	}
	m_view_reports->setEnabled(m_active_model->isBilled(0));
	m_edit_session->setEnabled(false);
	m_remove_session->setEnabled(false);

	QCoreApplication::processEvents();
	int value = m_active_project->scrollValue();
	if (value != -1) {
		m_details->verticalScrollBar()->setValue(value);
	} else {
		m_details->scrollToBottom();
	}

	updateWindowTitle(item->text(0));
}

//-----------------------------------------------------------------------------

void Window::projectChanged(QTreeWidgetItem* item, int column)
{
	if (column == 0) {
		updateWindowTitle(item->text(0));
	}
}

//-----------------------------------------------------------------------------

void Window::filterChanged(int index)
{
	m_active_project->filterModel()->setType(m_filter->itemData(index).toInt());
	m_details->expandAll();
	m_details->scrollToBottom();
	updateReportActions();
}

//-----------------------------------------------------------------------------

void Window::modelBilledStatusChanged()
{
	sessionPressed(m_details->currentIndex());
	m_view_reports->setEnabled(m_active_model->isBilled(0));
	m_details->expandAll();
}

//-----------------------------------------------------------------------------

void Window::sessionPressed(const QModelIndex& index)
{
	QModelIndex session = m_active_project->filterModel()->mapUnbilledToSource(index);
	bool enabled = session.isValid();
	m_edit_session->setEnabled(enabled && (!m_inline || session.column() < 4));
	m_remove_session->setEnabled(enabled);
	updateReportActions();

	m_details->removeAction(m_create_report);
	m_details->removeAction(m_remove_report);
	if (!m_active_model->isBilled(m_active_project->filterModel()->mapToSource(index).row())) {
		m_details->addAction(m_create_report);
	} else if (m_active_project->filterModel()->mapLastBilledToSource(index).isValid()) {
		m_details->addAction(m_remove_report);
	}
}

//-----------------------------------------------------------------------------

void Window::sessionsInserted(const QModelIndex&, int, int end)
{
	m_details->setCurrentIndex(m_active_project->filterModel()->mapFromSource(m_active_model->index(end, 0)));
}

//-----------------------------------------------------------------------------

void Window::sessionsScrolled(int value)
{
	if (m_active_project) {
		m_active_project->setScrollValue(value);
	}
}

//-----------------------------------------------------------------------------

void Window::addSession()
{
	Q_ASSERT(m_active_model);
	SessionDialog dialog(this);
	Q_FOREVER {
		if (dialog.exec() == QDialog::Accepted) {
			if (m_active_model->add(dialog.session())) {
				m_edit_session->setEnabled(true);
				m_remove_session->setEnabled(true);
				break;
			} else {
				QMessageBox::warning(this, tr("Error"), tr("Session conflicts with other sessions."));
			}
		} else {
			break;
		}
	}
}

//-----------------------------------------------------------------------------

void Window::editSession()
{
	Q_ASSERT(m_active_model);
	QModelIndex index = m_active_project->filterModel()->mapUnbilledToSource(m_details->currentIndex());
	if (!index.isValid()) {
		return;
	}

	if (!m_inline) {
		int pos = index.row();
		SessionDialog dialog(this);
		dialog.setSession(m_active_model->session(pos));
		Q_FOREVER {
			if (dialog.exec() == QDialog::Accepted) {
				if (m_active_model->edit(pos, dialog.session())) {
					save();
					return;
				} else {
					QMessageBox::warning(this, tr("Error"), tr("Session conflicts with other sessions."));
				}
			} else {
				break;
			}
		}
	} else if (m_active_model->flags(index) & Qt::ItemIsEditable) {
		m_details->edit(m_details->currentIndex());
	}
}

//-----------------------------------------------------------------------------

void Window::removeSession()
{
	Q_ASSERT(m_active_model);
	if (QMessageBox::question(this, tr("Question"), tr("Remove selected session?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
		m_edit_session->setEnabled(false);
		m_remove_session->setEnabled(false);
		m_active_model->remove(m_active_project->filterModel()->mapToSource(m_details->currentIndex()).row());
	}
}

//-----------------------------------------------------------------------------

void Window::toggleColumnHidden(int column)
{
	m_details->setColumnHidden(column, !m_details->isColumnHidden(column));
	QStringList hidden;
	for (int i = 3; i < 10; ++i) {
		if (m_details->isColumnHidden(i)) {
			hidden.append(QString::number(i));
		}
	}
	Settings().setValue("HiddenColumns", hidden);
}

//-----------------------------------------------------------------------------

void Window::save()
{
	if (!m_valid) {
		return;
	}

	// Create temporary backup of time data
	if (m_backups_enabled) {
		QFile::remove(m_filename + ".bak");
		QFile::copy(m_filename, m_filename + ".bak");
	}

	// Open file for writing
	QSaveFile file(m_filename);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
		QMessageBox::critical(this, tr("Error"), tr("Unable to write time data."));
		return;
	}

	// Write time data
	QXmlStreamWriter xml(&file);
	xml.setAutoFormatting(true);
	xml.writeStartDocument();
	xml.writeStartElement("kapow");

	m_contact.toXml(xml);
	m_rates.toXml(xml);

	int count = m_projects->topLevelItemCount();
	for (int i = 0; i < count; ++i) {
		Project* project = dynamic_cast<Project*>(m_projects->topLevelItem(i));
		if (project) {
			project->toXml(xml);
		}
	}

	xml.writeEndDocument();

	// Force time data to disk
	bool saved = file.commit();
	if (!saved) {
		QMessageBox::critical(this, tr("Error"), tr("Unable to write time data."));
	}
}

//-----------------------------------------------------------------------------

void Window::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
	if (reason == QSystemTrayIcon::Trigger) {
		toggleVisible();
	}
}

//-----------------------------------------------------------------------------

void Window::toggleVisible()
{
	if (isVisible() && !m_blocked) {
		m_toggle_visibility->setText(tr("&Restore"));
		hide();
	} else {
		m_toggle_visibility->setText(tr("&Minimize"));
		show();
		raise();
		activateWindow();
	}
}

//-----------------------------------------------------------------------------

int Window::currentRow()
{
	QModelIndex session = m_details->currentIndex();
	if (session.parent().isValid()) {
		session = session.parent();
	}
	session = m_active_project->filterModel()->mapToSource(session);
	return session.isValid() ? session.row() : -1;
}

//-----------------------------------------------------------------------------

void Window::loadData()
{
	// Try to load time data
	loadData(m_filename);
	if (m_valid) {
		if (m_backups_enabled) {
			createDataBackup();
		}
		return;
	}

	if (m_backups_enabled) {
		// Try to load time data from temporary backup
		if (QFile::exists(m_filename + ".bak")) {
			m_valid = true;
			loadData(m_filename + ".bak");
			if (m_valid) {
				return;
			}
		}

		// Try to load time data from any backup
		QStringList backups = QFileInfo(m_filename).dir().entryList(QStringList(m_filename + ".bak-"),
				QDir::Files,
				QDir::Name | QDir::IgnoreCase | QDir::Reversed);
		for (const QString& backup : backups) {
			m_valid = true;
			loadData(backup);
			if (m_valid) {
				return;
			}
		}
	}

	// Try to start with empty time data
	m_valid = true;
	loadData(m_filename);
}

//-----------------------------------------------------------------------------

void Window::loadData(const QString& filename)
{
	// Open data file; create default project if it doesn't exist
	QFile file(filename);
	if (!file.exists()) {
		addProject(tr("Untitled"));
		m_projects->setCurrentItem(m_projects->topLevelItem(0));
		return;
	}

	// Abort if data is not writable
	if (m_valid && file.exists() && !QFileInfo(file).isWritable()) {
		m_valid = false;
		QMessageBox::critical(this, tr("Error"), tr("Unable to write time data."));
		return;
	}

	// Abort if data is unreadable
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		m_valid = false;
		QMessageBox::critical(this, tr("Error"), tr("Unable to read time data."));
		return;
	}

	// Parse data file
	QStack<Project*> projects;
	SessionModel* model = nullptr;
	int filter = 0;
	QTreeWidgetItem* item = m_projects->invisibleRootItem();
	QTreeWidgetItem* current = nullptr;

	QXmlStreamReader xml(&file);
	while (!xml.atEnd()) {
		xml.readNext();
		if (xml.isStartElement()) {
			QXmlStreamAttributes attributes = xml.attributes();
			// Add session
			if ((xml.name() == QLatin1String("session")) && model) {
				QDate date = QDate::fromString(attributes.value(QLatin1String("date")).toString(), Qt::ISODate);
				QTime start = QTime::fromString(attributes.value(QLatin1String("start")).toString(), Qt::ISODate);
				QTime stop = QTime::fromString(attributes.value(QLatin1String("stop")).toString(), Qt::ISODate);
				QString task = attributes.value(QLatin1String("note")).toString();
				bool billed = attributes.value(QLatin1String("billed")).toString().toInt();
				if (!model->add(Session(date, start, stop, task, billed))) {
					xml.raiseError(tr("Session conflicts with other sessions."));
				}
			// Start adding project
			} else if (xml.name() == QLatin1String("project")) {
				m_projects->blockSignals(true);
				projects.push(new Project(item, attributes.value(QLatin1String("name")).toString()));
				item = projects.top();
				item->setExpanded(attributes.value(QLatin1String("expanded")) == QLatin1String("1"));
				current = (attributes.value(QLatin1String("current")) == QLatin1String("1")) ? projects.top() : current;
				filter = attributes.value(QLatin1String("filter")).toString().toInt();
				model = projects.top()->model();
				model->setDecimalTotals(m_decimals);
				model->beginLoad();
				m_projects->blockSignals(false);
			// Read contact information
			} else if (xml.name() == QLatin1String("contact")) {
				m_contact.fromXml(xml);
			// Read rates
			} else if (xml.name() == QLatin1String("rates")) {
				m_rates.fromXml(xml);
			// Add autosaved time as new session to current project
			} else if ((xml.name() == QLatin1String("autosave")) && model) {
				QDateTime start = QDateTime::fromString(attributes.value(QLatin1String("start")).toString(), Qt::ISODate);
				QDateTime stop = QDateTime::fromString(attributes.value(QLatin1String("stop")).toString(), Qt::ISODate);
				QString task = attributes.value(QLatin1String("note")).toString();
				if (!model->add(start, stop, task)) {
					xml.raiseError(tr("Session conflicts with other sessions."));
				}
			}
		// Finish adding project
		} else if (xml.isEndElement() && xml.name() == QLatin1String("project")) {
			model->endLoad();
			projects.top()->filterModel()->setType(filter);
			projects.pop();
			if (!projects.isEmpty()) {
				model = projects.top()->model();
				item = projects.top();
			} else {
				model = nullptr;
				item = m_projects->invisibleRootItem();
			}
		}
	}

	// Abort if data is corrupt
	if (xml.hasError()) {
		m_valid = false;

		// Move aside invalid data
		QString path = QFileInfo(file).canonicalFilePath();
		QFile::rename(filename, filename + ".invalid-" + QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz"));

		// Clear invalid data
		m_contact = Contact();
		m_rates = Rates();
		m_projects->clear();

		// Warn user that data is invalid
		QMessageBox message(QMessageBox::Critical, tr("Error"), tr("Unable to read time data."), QMessageBox::Ok, this);
		message.setInformativeText(QString("%1:%2:%3: %4")
				.arg(path)
				.arg(xml.lineNumber())
				.arg(xml.columnNumber())
				.arg(xml.errorString().toHtmlEscaped())
		);
		message.exec();
		return;
	}
	file.close();

	// Create default project if data file has none
	if (m_projects->topLevelItemCount() == 0) {
		addProject(tr("Untitled"));
		current = nullptr;
	}

	// Select last used project
	if (!current) {
		current = m_projects->topLevelItem(0);
	}
	m_projects->setCurrentItem(current);
}

//-----------------------------------------------------------------------------

void Window::createDataBackup()
{
	// Create daily backup
	QDate date = QDate::currentDate();
	QString path = m_filename + ".bak-" + date.toString("yyyyMMdd");
	if (QFile::exists(path)) {
		return;
	}
	QFile::copy(m_filename, path);

	// Create list of daily and weekly backups
	QStringList days;
	for (int i = 0; i < 7; ++i) {
		QDate d = date.addDays(-i);
		days.append(m_filename + ".bak-" + d.toString("yyyyMMdd"));
	}
	QStringList weeks;
	for (int i = 1; i < 5; ++i) {
		QDate d = date.addDays(-i * 7);
		int year;
		int week = d.weekNumber(&year);
		weeks.prepend(QString("%1-%2").arg(year).arg(week));
	}

	// Reduce previous backups
	QString current_month = date.addMonths(-1).addYears(-1).toString("yyyyMM");
	int current_year = 0;
	QDir dir = QFileInfo(m_filename).dir();
	QStringList backups = dir.entryList(QStringList(m_filename + ".bak-"), QDir::Files, QDir::Name | QDir::IgnoreCase);
	for (const QString& backup : backups) {
		// Keep one backup a day for the last 7 days
		if (days.contains(backup)) {
			continue;
		}

		QDate d = QDate::fromString(backup.right(8), "yyyyMMdd");
		if (!d.isValid()) {
			continue;
		}

		// Keep one backup a week for the last 4 weeks
		int year;
		int week = d.weekNumber(&year);
		QString week_string = QString("%1-%2").arg(year).arg(week);
		if (weeks.first() == week_string) {
			weeks.removeFirst();
			continue;
		}

		// Keep one backup a month for the the last 12 months
		QString month = d.toString("yyyyMM");
		if (month > current_month) {
			current_month = month;
			if (d.year() > current_year) {
				current_year = d.year();
			}
			continue;
		}

		// Keep one backup a year from the beginning
		if (d.year() > current_year) {
			current_year = d.year();
			continue;
		}

		// Remove extra backup
		dir.remove(backup);
	}
}

//-----------------------------------------------------------------------------

void Window::addProject(const QString& name)
{
	Project* project = new Project(m_projects, name);
	project->model()->setDecimalTotals(m_decimals);
	m_projects->setCurrentItem(project);
}

//-----------------------------------------------------------------------------

void Window::removeProject(QTreeWidgetItem* item)
{
	if (!item) {
		return;
	}

	// Recursively remove subprojects
	for (int i = item->childCount() - 1; i >= 0; i--) {
		removeProject(item->child(i));
	}

	// Unset active model before removing project
	Project* project = dynamic_cast<Project*>(item);
	if (project && project->model() == m_active_model) {
		m_active_model = nullptr;
	}

	delete item;
	item = nullptr;
}

//-----------------------------------------------------------------------------

void Window::updateDetails()
{
	Q_ASSERT(m_active_project);
	updateDisplay();
	updateSessionButtons();
	updateTrayIcon();
	m_projects->resizeColumnToContents(1);
}

//-----------------------------------------------------------------------------

void Window::updateDisplay()
{
	QString time = m_active_project->time();
	m_display->setText(!time.isEmpty() ? time : "00:00:00");
}

//-----------------------------------------------------------------------------

void Window::updateReportActions()
{
	m_create_report->setEnabled(m_active_model->canBill());
	m_remove_report->setEnabled(!m_active_model->billedRows().isEmpty());
}

//-----------------------------------------------------------------------------

void Window::updateSessionButtons()
{
	updateReportActions();
	if (!m_active_project->time().isEmpty()) {
		m_start->hide();
		m_stop->show();
		m_cancel->setEnabled(true);
		m_create_report->setEnabled(false);
	} else {
		m_stop->hide();
		m_start->show();
		m_cancel->setEnabled(false);
	}
}

//-----------------------------------------------------------------------------

void Window::updateTrayIcon()
{
	int count = m_active_timers.count();
	if (count == 1) {
		m_tray_icon->setIcon(m_active_icon);
		m_tray_icon->setToolTip(m_active_timers.first()->time());
	} else if (count == 0) {
		m_tray_icon->setIcon(m_inactive_icon);
		m_tray_icon->setToolTip(tr("Kapow Punch Clock"));
	} else {
		m_tray_icon->setIcon(m_active_icon);
		m_tray_icon->setToolTip(tr("%n timer(s) running", "", count));
	}
}

//-----------------------------------------------------------------------------

void Window::updateWindowTitle(const QString& project)
{
	setWindowFilePath(project);
}

//-----------------------------------------------------------------------------
