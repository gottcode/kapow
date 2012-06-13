/***********************************************************************
 *
 * Copyright (C) 2008, 2009, 2010, 2011, 2012 Graeme Gott <graeme@gottcode.org>
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

#include "window.h"

#include "contact.h"
#include "data_model.h"
#include "date_editor.h"
#include "filter_model.h"
#include "locale_dialog.h"
#include "project.h"
#include "report.h"
#include "session.h"
#include "session_delegate.h"
#include "session_dialog.h"
#include "time_editor.h"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QComboBox>
#include <QDateTime>
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
#include <QScrollBar>
#include <QSettings>
#include <QSignalMapper>
#include <QSplitter>
#include <QStyledItemDelegate>
#include <QTextDocument>
#include <QTimer>
#include <QTreeView>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QXmlStreamReader>

#if defined(Q_OS_MAC)
#include <sys/fcntl.h>
#elif defined(Q_OS_UNIX)
#include <unistd.h>
#elif defined(Q_OS_WIN)
#include <windows.h>
#include <io.h>
#endif

/*****************************************************************************/

namespace {
	class Delegate : public QStyledItemDelegate {
	public:
		Delegate(QObject* parent = 0)
		: QStyledItemDelegate(parent) {
			m_height = QLineEdit().sizeHint().height();
		}

		QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const {
			if (index.column() == 0) {
				return QStyledItemDelegate::createEditor(parent, option, index);
			} else {
				return 0;
			}
		}

		QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
		{
			QSize size = QStyledItemDelegate::sizeHint(option, index);
			size.setHeight(m_height);
			return size;
		}

	private:
		int m_height;
	};
}

/*****************************************************************************/

Window::Window(const QString& filename, QWidget* parent)
: QMainWindow(parent), m_filename(filename), m_valid(true), m_decimals(true), m_inline(true), m_active_project(0), m_active_model(0), m_active_timers(0) {
	QWidget* contents = new QWidget(this);
	setCentralWidget(contents);

	m_display = new QLabel(tr("00:00:00"), contents);
	QFont font = m_display->font();
	font.setPointSize(32);
	m_display->setFont(font);

	m_current_time = QDateTime::currentDateTime();
	m_timer = new QTimer(this);
	m_timer->setInterval(1000);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(updateTime()));
	m_timer->start();

	QTimer* save_timer = new QTimer(this);
	save_timer->setInterval(30000);
	connect(save_timer, SIGNAL(timeout()), this, SLOT(save()));
	save_timer->start();

	m_task = new QLineEdit(contents);
	m_task->setPlaceholderText(DataModel::tr("Task"));
	m_task->setFocus();
	connect(m_task, SIGNAL(textChanged(QString)), this, SLOT(taskChanged(QString)));
	connect(m_task, SIGNAL(returnPressed()), this, SLOT(taskStart()));

	m_start = new QPushButton(tr("Start"), contents);
	m_start->setAutoDefault(true);
	connect(m_start, SIGNAL(clicked()), this, SLOT(start()));
	int button_width = m_start->sizeHint().width();

	m_stop = new QPushButton(tr("Stop"), contents);
	m_stop->setAutoDefault(true);
	m_stop->hide();
	connect(m_stop, SIGNAL(clicked()), this, SLOT(stop()));
	button_width = qMax(m_stop->sizeHint().width(), button_width);

	m_cancel = new QPushButton(tr("Cancel"), contents);
	m_cancel->setAutoDefault(true);
	m_cancel->setEnabled(false);
	connect(m_cancel, SIGNAL(clicked()), this, SLOT(cancel()));
	button_width = qMax(m_cancel->sizeHint().width(), button_width);

	m_start->setMinimumWidth(button_width);
	m_stop->setMinimumWidth(button_width);
	m_cancel->setMinimumWidth(button_width);

	// Load settings
	QSettings settings;
	m_decimals = settings.value("DecimalTotals", true).toBool();
	m_inline = settings.value("InlineEditing", true).toBool();

	// Create menus
	QMenu* menu = menuBar()->addMenu(tr("&Project"));
	m_add_project = menu->addAction(tr("&Add"), this, SLOT(addProject()));
	m_remove_project = menu->addAction(tr("&Remove"), this, SLOT(removeProject()));
	m_remove_project->setEnabled(false);
	menu->addSeparator();
	m_create_report = menu->addAction(tr("&Create Report..."), this, SLOT(createReport()));
	m_create_report->setEnabled(false);
	menu->addAction(tr("View R&eports"), this, SLOT(viewReports()));
	menu->addSeparator();
	menu->addAction(tr("&Quit"), this, SLOT(close()), tr("Ctrl+Q"));

	menu = menuBar()->addMenu(tr("&Session"));
	m_add_session = menu->addAction(tr("&Add"), this, SLOT(addSession()));
	m_edit_session = menu->addAction(tr("&Edit"), this, SLOT(editSession()));
	m_edit_session->setEnabled(false);
	m_remove_session = menu->addAction(tr("&Remove"), this, SLOT(removeSession()));
	m_remove_session->setEnabled(false);

	menu = menuBar()->addMenu(tr("S&ettings"));
	QMenu* column_menu = menu->addMenu(tr("Columns"));
	QAction* action = menu->addAction(tr("&Decimal Totals"));
	action->setCheckable(true);
	action->setChecked(m_decimals);
	connect(action, SIGNAL(toggled(bool)), this, SLOT(setDecimalTotals(bool)));
	action = menu->addAction(tr("&Inline Editing"));
	action->setCheckable(true);
	action->setChecked(m_inline);
	connect(action, SIGNAL(toggled(bool)), this, SLOT(setInlineEditing(bool)));
	menu->addSeparator();
	menu->addAction(tr("Application &Language..."), this, SLOT(setLocaleClicked()));

	menu = menuBar()->addMenu(tr("&Help"));
	menu->addAction(tr("&About"), this, SLOT(about()));
	menu->addAction(tr("About &Qt"), qApp, SLOT(aboutQt()));

	// Create projects
	m_projects = new QTreeWidget(contents);
	m_projects->setAutoExpandDelay(500);
	m_projects->setDragEnabled(true);
	m_projects->setDragDropMode(QAbstractItemView::InternalMove);
	m_projects->setHeaderLabels(QStringList() << tr("Project") << tr("Timer"));
	m_projects->setItemDelegate(new Delegate(m_projects));
	m_projects->setSelectionMode(QAbstractItemView::SingleSelection);
	m_projects->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_projects->setSortingEnabled(true);
	m_projects->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	m_projects->sortByColumn(0, Qt::AscendingOrder);
	m_projects->header()->setClickable(false);
	m_projects->header()->setMovable(false);
	m_projects->header()->setSortIndicatorShown(false);
	m_projects->header()->setStretchLastSection(false);
	m_projects->header()->setResizeMode(0, QHeaderView::Stretch);
	m_projects->header()->setResizeMode(1, QHeaderView::ResizeToContents);
	m_projects->addAction(m_add_project);
	m_projects->addAction(m_remove_project);
	m_projects->setContextMenuPolicy(Qt::ActionsContextMenu);
	connect(m_projects, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT(projectActivated(QTreeWidgetItem*)));
	connect(m_projects, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(projectChanged(QTreeWidgetItem*, int)));

	// Create details
	QItemEditorFactory* factory = new QItemEditorFactory;
	factory->registerEditor(QVariant::Date, new QStandardItemEditorCreator<DateEditor>());
	factory->registerEditor(QVariant::Time, new QStandardItemEditorCreator<TimeEditor>());
	factory->registerEditor(QVariant::String, new QStandardItemEditorCreator<QLineEdit>());
	QItemEditorFactory::setDefaultFactory(factory);

	QWidget* details = new QWidget(contents);

	m_filter = new QComboBox(details);
	m_filter->addItem(tr("Show all"), FilterModel::All);
	m_filter->addItem(tr("Show only unbilled"), FilterModel::Unbilled);
	m_filter->addItem(tr("Show only this year"), FilterModel::ThisYear);
	m_filter->addItem(tr("Show only this month"), FilterModel::ThisMonth);
	m_filter->addItem(tr("Show only this week"), FilterModel::ThisWeek);
	connect(m_filter, SIGNAL(activated(int)), this, SLOT(filterChanged(int)));

	m_details = new QTreeView(details);
	m_details->setRootIsDecorated(false);
	m_details->setItemsExpandable(false);
	m_details->setItemDelegate(new SessionDelegate(m_details));
	m_details->setIndentation(0);
	m_details->setSelectionMode(QAbstractItemView::SingleSelection);
	m_details->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_details->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	m_details->header()->setClickable(false);
	m_details->header()->setMovable(false);
	m_details->addAction(m_add_session);
	m_details->addAction(m_edit_session);
	m_details->addAction(m_remove_session);
	m_details->setContextMenuPolicy(Qt::ActionsContextMenu);
	connect(m_details, SIGNAL(activated(const QModelIndex&)), this, SLOT(editSession()));
	connect(m_details, SIGNAL(pressed(const QModelIndex&)), this, SLOT(sessionPressed(const QModelIndex&)));
	connect(m_details->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(sessionsScrolled(int)));
	m_details->setEditTriggers(QAbstractItemView::NoEditTriggers);

	QVBoxLayout* details_layout = new QVBoxLayout(details);
	details_layout->setMargin(0);
	details_layout->addWidget(m_filter);
	details_layout->addWidget(m_details);

	// Create contents splitter
	m_contents = new QSplitter(contents);
	m_contents->addWidget(m_projects);
	m_contents->addWidget(details);
	m_contents->setStretchFactor(0, 0);
	m_contents->setStretchFactor(1, 1);

	// Add tray icon
	m_active_icon = windowIcon();
	QList<QSize> sizes = m_active_icon.availableSizes();
	foreach (const QSize& size, sizes) {
		m_inactive_icon.addPixmap(m_active_icon.pixmap(size, QIcon::Disabled));
	}
	m_tray_icon = new QSystemTrayIcon(m_inactive_icon, this);
	updateTrayIcon();
	connect(m_tray_icon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));
	m_tray_icon->show();

	// Lay out window
	QHBoxLayout* session_buttons = new QHBoxLayout;
	session_buttons->setMargin(0);
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
	session_layout->setMargin(0);
	session_layout->setSpacing(0);
	session_layout->addWidget(m_display, 0, Qt::AlignCenter);
	session_layout->addLayout(session_buttons);

	QVBoxLayout* layout = new QVBoxLayout(contents);
	layout->setMargin(0);
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
	QSignalMapper* mapper = new QSignalMapper(this);
	connect(mapper, SIGNAL(mapped(int)), this, SLOT(toggleColumnHidden(int)));
	QList<QAction*> column_actions;
	for (int i = 3; i < 10; ++i) {
		action = column_menu->addAction(m_active_model->headerData(i, Qt::Horizontal).toString(), mapper, SLOT(map()));
		action->setCheckable(true);
		action->setChecked(true);
		mapper->setMapping(action, i);
		column_actions.append(action);
	}
	m_details->header()->addActions(column_actions);
	m_details->header()->setContextMenuPolicy(Qt::ActionsContextMenu);
	m_details->header()->setResizeMode(QHeaderView::ResizeToContents);
	m_details->header()->setResizeMode(3, QHeaderView::Stretch);
	m_details->header()->setStretchLastSection(false);

	// Restore hidden columns
	QStringList hidden = QSettings().value("HiddenColumns", QStringList() << "5" << "6" << "7" << "8" << "9").toStringList();
	foreach (const QString& column, hidden) {
		int i = column.toInt();
		m_details->setColumnHidden(i, true);
		column_actions[i - 3]->setChecked(false);
	}
}

/*****************************************************************************/

bool Window::isValid() const {
	return m_valid;
}

/*****************************************************************************/

void Window::closeEvent(QCloseEvent* event) {
	QSettings settings;
	settings.setValue("WindowGeometry", saveGeometry());
	settings.setValue("SplitterSizes", m_contents->saveState());

	if (m_active_timers) {
		// Show window
		bool visible = isVisible();
		if (!visible) {
			show();
		}
		raise();
		activateWindow();

		// Prompt user about running timers
		if (QMessageBox::question(this, tr("Question"), tr("There are timers running. Stop timers and quit?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
			int count = m_projects->topLevelItemCount();
			for (int i = 0; i < count; ++i) {
				Project* project = dynamic_cast<Project*>(m_projects->topLevelItem(i));
				if (project && project->isActive()) {
					project->stop(m_current_time);
				}
			}
		} else {
			event->ignore();
			if (!visible) {
				hide();
			}
			return;
		}
	}

	save();
	event->accept();
}

/*****************************************************************************/

void Window::about() {
	QMessageBox::about(this, tr("About"), QString("<p align='center'><big><b>%1 %2</b></big><br/>%3<br/><small>%4</small><br/><small>%5</small></p>")
		.arg(tr("Kapow Punch Clock"), QCoreApplication::applicationVersion(),
			tr("A program to help track spent time"),
			tr("Copyright &copy; 2008-%1 Graeme Gott").arg("2012"),
			tr("Released under the <a href=%1>GPL 3</a> license").arg("\"http://www.gnu.org/licenses/gpl.html\""))
	);
}

/*****************************************************************************/

void Window::setDecimalTotals(bool decimals) {
	m_decimals = decimals;
	int count = m_projects->topLevelItemCount();
	for (int i = 0; i < count; ++i) {
		Project* project = dynamic_cast<Project*>(m_projects->topLevelItem(i));
		if (project) {
			project->setDecimalTotals(m_decimals);
		}
	}
	QSettings().setValue("DecimalTotals", m_decimals);
}

/*****************************************************************************/

void Window::setInlineEditing(bool edit) {
	m_inline = edit;
	if (!edit) {
		m_details->closePersistentEditor(m_details->currentIndex());
	}
	sessionPressed(m_details->currentIndex());
	QSettings().setValue("InlineEditing", edit);
}

/*****************************************************************************/

void Window::setLocaleClicked() {
	LocaleDialog dialog(this);
	dialog.exec();
}

/*****************************************************************************/

void Window::start() {
	m_active_project->start(m_current_time);
	m_active_timers++;
	m_remove_project->setEnabled(false);
	updateDetails();
	m_stop->setFocus();
}

/*****************************************************************************/

void Window::stop() {
	m_active_project->stop(m_current_time);
	m_active_timers--;
	m_remove_project->setEnabled(true);
	updateDetails();
	m_task->clear();
	m_task->setFocus();
}

/*****************************************************************************/

void Window::cancel() {
	if (QMessageBox::question(this, tr("Question"), tr("Cancel this session?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
		m_active_project->stop();
		m_active_timers--;
		m_remove_project->setEnabled(true);
		updateDetails();
		m_task->clear();
		m_task->setFocus();
	}
}

/*****************************************************************************/

void Window::taskChanged(const QString& task) {
	m_active_project->setTask(task);
}

/*****************************************************************************/

void Window::taskStart() {
	if (m_start->isEnabled()) {
		start();
	}
}

/*****************************************************************************/

void Window::updateTime() {
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
}

/*****************************************************************************/

void Window::addProject() {
	bool ok;
	QString name;
	forever {
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

/*****************************************************************************/

void Window::editProject() {
	m_projects->edit(m_projects->currentIndex());
}

/*****************************************************************************/

void Window::removeProject() {
	if (QMessageBox::question(this, tr("Question"), tr("Remove selected project?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
		m_remove_project->setEnabled(false);
		m_remove_session->setEnabled(false);
		removeProject(m_projects->currentItem());
	}
}

/*****************************************************************************/

void Window::createReport() {
	int current = currentRow();
	if (current == -1) {
		return;
	}
	Report report(m_active_model, current, &m_contact, &m_rates, this);
	report.exec();
}

/*****************************************************************************/

void Window::viewReports() {
	int current = currentRow();
	if ((current == (m_active_model->rowCount() - 1)) || !m_active_model->isBilled(current)) {
		current = -1;
	}
	Report report(m_active_model, current, &m_contact, &m_rates, this);
	report.exec();
}

/*****************************************************************************/

void Window::projectActivated(QTreeWidgetItem* item) {
	Project* project = dynamic_cast<Project*>(item);
	if (!project) {
		addProject(tr("Untitled"));
		return;
	}

	if (m_active_model) {
		disconnect(m_active_model, SIGNAL(billedStatusChanged(bool)), this, SLOT(modelBilledStatusChanged()));
		disconnect(m_active_model, SIGNAL(rowsInserted(const QModelIndex&, int, int)), this, SLOT(sessionsInserted(const QModelIndex&, int, int)));
	}
	m_active_project = project;
	m_active_model = m_active_project->model();
	connect(m_active_model, SIGNAL(billedStatusChanged(bool)), this, SLOT(modelBilledStatusChanged()));
	connect(m_active_model, SIGNAL(rowsInserted(const QModelIndex&, int, int)), this, SLOT(sessionsInserted(const QModelIndex&, int, int)));

	updateDisplay();
	updateSessionButtons();
	m_details->setModel(m_active_project->filterModel());
	m_details->expandAll();
	m_filter->setCurrentIndex(m_filter->findData(m_active_project->filterModel()->type()));
	m_remove_project->setEnabled(!project->isActive());
	m_edit_session->setEnabled(false);
	m_remove_session->setEnabled(false);

	QApplication::processEvents();
	int value = m_active_project->scrollValue();
	if (value != -1) {
		m_details->verticalScrollBar()->setValue(value);
	} else {
		m_details->scrollToBottom();
	}

	updateWindowTitle(item->text(0));
}

/*****************************************************************************/

void Window::projectChanged(QTreeWidgetItem* item, int column) {
	if (column == 0) {
		updateWindowTitle(item->text(0));
	}
}

/*****************************************************************************/

void Window::filterChanged(int index) {
	m_active_project->filterModel()->setType(m_filter->itemData(index).toInt());
	m_details->expandAll();
	m_details->scrollToBottom();
}

/*****************************************************************************/

void Window::modelBilledStatusChanged() {
	sessionPressed(m_details->currentIndex());
	m_details->expandAll();
}

/*****************************************************************************/

void Window::sessionPressed(const QModelIndex& index) {
	QModelIndex session = m_active_project->filterModel()->mapUnbilledToSource(index);
	bool enabled = session.isValid();
	m_edit_session->setEnabled(enabled && (!m_inline || session.column() < 4));
	m_remove_session->setEnabled(enabled);
	m_create_report->setEnabled(enabled);
}

/*****************************************************************************/

void Window::sessionsInserted(const QModelIndex&, int, int end) {
	m_details->setCurrentIndex(m_active_project->filterModel()->mapFromSource(m_active_model->index(end, 0)));
}

/*****************************************************************************/

void Window::sessionsScrolled(int value) {
	if (m_active_project) {
		m_active_project->setScrollValue(value);
	}
}

/*****************************************************************************/

void Window::addSession() {
	Q_ASSERT(m_active_model != 0);
	SessionDialog dialog(this);
	forever {
		if (dialog.exec() == QDialog::Accepted) {
			if (m_active_model->add(dialog.session())) {
				m_edit_session->setEnabled(true);
				m_remove_session->setEnabled(true);
				break;
			}
		} else {
			break;
		}
	}
}

/*****************************************************************************/

void Window::editSession() {
	Q_ASSERT(m_active_model != 0);
	QModelIndex index = m_active_project->filterModel()->mapUnbilledToSource(m_details->currentIndex());
	if (!index.isValid()) {
		return;
	}

	if (!m_inline) {
		int pos = index.row();
		SessionDialog dialog(this);
		dialog.setSession(m_active_model->session(pos));
		forever {
			if (dialog.exec() == QDialog::Accepted) {
				if (m_active_model->edit(pos, dialog.session())) {
					save();
					return;
				}
			} else {
				break;
			}
		}
	} else if (m_active_model->flags(index) & Qt::ItemIsEditable) {
		m_details->edit(m_details->currentIndex());
	}
}

/*****************************************************************************/

void Window::removeSession() {
	Q_ASSERT(m_active_model != 0);
	if (QMessageBox::question(this, tr("Question"), tr("Remove selected session?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
		m_edit_session->setEnabled(false);
		m_remove_session->setEnabled(false);
		m_active_model->remove(m_active_project->filterModel()->mapToSource(m_details->currentIndex()).row());
	}
}

/*****************************************************************************/

void Window::toggleColumnHidden(int column) {
	m_details->setColumnHidden(column, !m_details->isColumnHidden(column));
	QStringList hidden;
	for (int i = 3; i < 10; ++i) {
		if (m_details->isColumnHidden(i)) {
			hidden.append(QString::number(i));
		}
	}
	QSettings().setValue("HiddenColumns", hidden);
}

/*****************************************************************************/

void Window::save() {
	if (!m_valid) {
		return;
	}

	// Open temporary file for writing
	QFile file(m_filename + ".tmp");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
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
	bool saved = true;
	if (file.isOpen()) {
#if defined(Q_OS_MAC)
		saved &= (fcntl(file.handle(), F_FULLFSYNC, NULL) == 0);
#elif defined(Q_OS_UNIX)
		saved &= (fsync(file.handle()) == 0);
#elif defined(Q_OS_WIN)
		saved &= (FlushFileBuffers(reinterpret_cast<HANDLE>(_get_osfhandle(file.handle()))) != 0);
#endif
		saved &= (file.error() == QFile::NoError);
		file.close();
	}

	// Replace data file with temporary file
	if (saved) {
		saved &= QFile::remove(m_filename);
		saved &= QFile::rename(m_filename + ".tmp", m_filename);
	}
	if (!saved) {
		QMessageBox::critical(this, tr("Error"), tr("Unable to write time data."));
	}
}

/*****************************************************************************/

void Window::trayIconActivated(QSystemTrayIcon::ActivationReason reason) {
	if (reason == QSystemTrayIcon::Trigger) {
		if (isVisible()) {
			hide();
		} else {
			show();
			raise();
			activateWindow();
		}
	}
}

/*****************************************************************************/

int Window::currentRow() {
	QModelIndex session = m_details->currentIndex();
	if (session.parent().isValid()) {
		session = session.parent();
	}
	session = m_active_project->filterModel()->mapToSource(session);
	return session.isValid() ? session.row() : -1;
}

/*****************************************************************************/

void Window::loadData() {
	// Open data file; create default project if it doesn't exist
	QFile file(m_filename);
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
	Project* project = 0;
	int filter = 0;
	QTreeWidgetItem* item = m_projects->invisibleRootItem();
	QTreeWidgetItem* current = 0;

	QXmlStreamReader xml(&file);
	while (!xml.atEnd()) {
		xml.readNext();
		if (xml.isStartElement()) {
			QXmlStreamAttributes attributes = xml.attributes();
			// Add session
			if (xml.name() == QLatin1String("session")) {
				QDate date = QDate::fromString(attributes.value(QLatin1String("date")).toString(), Qt::ISODate);
				QTime start = QTime::fromString(attributes.value(QLatin1String("start")).toString(), Qt::ISODate);
				QTime stop = QTime::fromString(attributes.value(QLatin1String("stop")).toString(), Qt::ISODate);
				QString task = attributes.value(QLatin1String("note")).toString();
				bool billed = attributes.value(QLatin1String("billed")).toString().toInt();
				project->model()->add(Session(date, start, stop, task, billed));
			// Start adding project
			} else if (xml.name() == QLatin1String("project")) {
				m_projects->blockSignals(true);
				item = project = new Project(item, attributes.value(QLatin1String("name")).toString());
				project->setExpanded(attributes.value(QLatin1String("expanded")) == "1");
				current = (attributes.value(QLatin1String("current")) == "1") ? project : current;
				filter = attributes.value(QLatin1String("filter")).toString().toInt();
				project->model()->setDecimalTotals(m_decimals);
				project->model()->beginLoad();
				m_projects->blockSignals(false);
			// Read contact information
			} else if (xml.name() == QLatin1String("contact")) {
				m_contact.fromXml(xml);
			// Read rates
			} else if (xml.name() == QLatin1String("rates")) {
				m_rates.fromXml(xml);
			// Add autosaved time as new session to current project
			} else if (xml.name() == QLatin1String("autosave")) {
				QDateTime start = QDateTime::fromString(attributes.value(QLatin1String("start")).toString(), Qt::ISODate);
				QDateTime stop = QDateTime::fromString(attributes.value(QLatin1String("stop")).toString(), Qt::ISODate);
				QString task = attributes.value(QLatin1String("note")).toString();
				project->model()->add(start, stop, task);
			}
		// Finish adding project
		} else if (xml.isEndElement() && xml.name() == QLatin1String("project")) {
			project->model()->endLoad();
			project->filterModel()->setType(filter);
			item = item->parent() ? item->parent() : m_projects->invisibleRootItem();
		}
	}

	// Abort if data is corrupt
	if (xml.hasError()) {
		m_valid = false;
		QMessageBox message(QMessageBox::Critical, tr("Error"), tr("Unable to read time data."), QMessageBox::Ok, this);
		message.setInformativeText(QString("%1:%2:%3: %4")
			.arg(QFileInfo(file).canonicalFilePath())
			.arg(xml.lineNumber())
			.arg(xml.columnNumber())
			.arg(Qt::escape(xml.errorString()))
		);
		message.exec();
		return;
	}
	file.close();

	// Create default project if data file has none
	if (m_projects->topLevelItemCount() == 0) {
		addProject(tr("Untitled"));
		current = 0;
	}

	// Select last used project
	if (current == 0) {
		current = m_projects->topLevelItem(0);
	}
	m_projects->setCurrentItem(current);
}

/*****************************************************************************/

void Window::addProject(const QString& name) {
	Project* project = new Project(m_projects, name);
	project->model()->setDecimalTotals(m_decimals);
	m_projects->setCurrentItem(project);
}

/*****************************************************************************/

void Window::removeProject(QTreeWidgetItem* item) {
	int count = item->childCount();
	for (int i = 0; i < count; ++i) {
		removeProject(item->child(i));
	}
	Project* project = dynamic_cast<Project*>(item);
	if (project && project->model() == m_active_model)  {
		m_active_model = 0;
	}
	delete item;
	item = 0;
}

/*****************************************************************************/

void Window::updateDetails() {
	Q_ASSERT(m_active_project != 0);
	updateDisplay();
	updateSessionButtons();
	updateTrayIcon();
	m_projects->resizeColumnToContents(1);
}

/*****************************************************************************/

void Window::updateDisplay() {
	QString time = m_active_project->time();
	m_display->setText(!time.isEmpty() ? time : "00:00:00");
}

/*****************************************************************************/

void Window::updateSessionButtons() {
	if (!m_active_project->time().isEmpty()) {
		m_start->hide();
		m_stop->show();
		m_cancel->setEnabled(true);
		m_create_report->setEnabled(false);
	} else {
		m_stop->hide();
		m_start->show();
		m_cancel->setEnabled(false);
		m_create_report->setEnabled(m_active_project->filterModel()->mapUnbilledToSource(m_details->currentIndex()).isValid());
	}
}

/*****************************************************************************/

void Window::updateTrayIcon() {
	m_tray_icon->setIcon(m_active_timers ? m_active_icon : m_inactive_icon);
	m_tray_icon->setToolTip(tr("%n timer(s) running", "", m_active_timers));
}

/*****************************************************************************/

void Window::updateWindowTitle(const QString& project) {
	setWindowTitle(project + ' ' + QChar(0x2014) + ' ' + tr("Kapow Punch Clock"));
}

/*****************************************************************************/
