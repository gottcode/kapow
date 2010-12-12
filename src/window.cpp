/***********************************************************************
 *
 * Copyright (C) 2008, 2009, 2010 Graeme Gott <graeme@gottcode.org>
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

#include "data_model.h"
#include "date_editor.h"
#include "filter_model.h"
#include "locale_dialog.h"
#include "project.h"
#include "report.h"
#include "session.h"
#include "session_dialog.h"
#include "time_editor.h"

#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QDateTime>
#include <QFile>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QItemEditorFactory>
#include <QLabel>
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
#include <QTimer>
#include <QTreeView>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QXmlStreamReader>

/*****************************************************************************/

namespace {
	class Delegate : public QStyledItemDelegate {
	public:
		Delegate(QObject* parent = 0)
		: QStyledItemDelegate(parent) {
		}

		QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const {
			if (index.column() == 0) {
				return QStyledItemDelegate::createEditor(parent, option, index);
			} else {
				return 0;
			}
		}
	};
}

/*****************************************************************************/

Window::Window(const QString& filename, QWidget* parent)
: QMainWindow(parent), m_filename(filename), m_decimals(true), m_inline(true), m_active_project(0), m_active_model(0), m_active_timers(0) {
	setWindowIcon(QIcon(":/kapow.png"));

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

	m_start = new QPushButton(tr("Start"), contents);
	connect(m_start, SIGNAL(clicked()), this, SLOT(start()));

	m_stop = new QPushButton(tr("Stop"), contents);
	m_stop->hide();
	connect(m_stop, SIGNAL(clicked()), this, SLOT(stop()));

	m_cancel = new QPushButton(tr("Cancel"), contents);
	m_cancel->setEnabled(false);
	connect(m_cancel, SIGNAL(clicked()), this, SLOT(cancel()));

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
	menu->addAction(tr("&Report"), this, SLOT(showReport()));
	menu->addSeparator();
	menu->addAction(tr("&Quit"), qApp, SLOT(quit()), tr("Ctrl+Q"));

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
	m_tray_icon = new QSystemTrayIcon(QIcon(":/kapow.png"), this);
	updateTrayIcon();
	connect(m_tray_icon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));
	m_tray_icon->show();

	// Lay out window
	QVBoxLayout* session_buttons = new QVBoxLayout;
	session_buttons->setMargin(0);
	session_buttons->setSpacing(0);
	session_buttons->addWidget(m_start);
	session_buttons->addWidget(m_stop);
	session_buttons->addWidget(m_cancel);

	QHBoxLayout* session_layout = new QHBoxLayout;
	session_layout->setMargin(0);
	session_layout->addStretch();
	session_layout->addWidget(m_display);
	session_layout->addLayout(session_buttons);
	session_layout->addStretch();

	QVBoxLayout* layout = new QVBoxLayout(contents);
	layout->addLayout(session_layout);
	layout->addWidget(m_contents);

	// Restore window geometry
	resize(800, 600);
	restoreGeometry(settings.value("WindowGeometry").toByteArray());
	m_contents->restoreState(settings.value("SplitterSizes").toByteArray());
	show();

	// Load details of all projects
	QTreeWidgetItem* current = 0;
	QFile file(m_filename);
	if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		Project* project = 0;
		int filter = 0;
		QTreeWidgetItem* item = 0;

		QXmlStreamReader xml(&file);
		while (!xml.atEnd()) {
			xml.readNext();
			if (xml.isStartElement()) {
				QXmlStreamAttributes attributes = xml.attributes();
				if (xml.name() == QLatin1String("session")) {
					QDate date = QDate::fromString(attributes.value("date").toString(), Qt::ISODate);
					QTime start = QTime::fromString(attributes.value("start").toString(), Qt::ISODate);
					QTime stop = QTime::fromString(attributes.value("stop").toString(), Qt::ISODate);
					bool billed = attributes.value("billed").toString().toInt();
					QString task = attributes.value("note").toString();
					project->model()->add(Session(date, start, stop, task, billed));
				} else if (xml.name() == QLatin1String("project")) {
					m_projects->blockSignals(true);
					QString name = attributes.value("name").toString();
					if (item == 0) {
						project = new Project(m_projects, name);
					} else {
						project = new Project(item, name);
					}
					project->setExpanded(attributes.value("expanded") == "1");
					current = (attributes.value("current") == "1") ? project : current;
					filter = attributes.value("filter").toString().toInt();
					item = project;
					project->model()->setDecimalTotals(m_decimals);
					m_projects->blockSignals(false);
				} else if (xml.name() == QLatin1String("autosave")) {
					QDateTime start = QDateTime::fromString(attributes.value("start").toString(), Qt::ISODate);
					QDateTime stop = QDateTime::fromString(attributes.value("stop").toString(), Qt::ISODate);
					project->model()->add(start, stop);
				}
			} else if (xml.isEndElement() && xml.name() == QLatin1String("project")) {
				project->filterModel()->setType(filter);
				item = item->parent();
			}
		}
	}
	if (m_projects->topLevelItemCount() == 0) {
		addProject(tr("Untitled"));
	}

	// Load last used project
	if (current == 0) {
		current = m_projects->topLevelItem(0);
	}
	m_projects->setCurrentItem(current);

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
	QStringList hidden = QSettings().value("HiddenColumns", QStringList() << "5" << "6" << "7" << "8").toStringList();
	foreach (const QString& column, hidden) {
		int i = column.toInt();
		m_details->setColumnHidden(i, true);
		column_actions[i - 3]->setChecked(false);
	}
}

/*****************************************************************************/

void Window::closeEvent(QCloseEvent* event) {
	QSettings settings;
	settings.setValue("WindowGeometry", saveGeometry());
	settings.setValue("SplitterSizes", m_contents->saveState());
	save();
	QMainWindow::closeEvent(event);
}

/*****************************************************************************/

void Window::about() {
	QMessageBox::about(this, tr("About"),
		QString("<p><center><big><b>%1</b></big><br/>%2<br/><small>%3</small><br/><small>%4</small></center></p>")
		.arg(tr("Kapow Punch Clock %1").arg(QCoreApplication::applicationVersion()))
		.arg(tr("A program to help track spent time"))
		.arg(tr("Copyright &copy; 2008-2010 Graeme Gott"))
		.arg(tr("Released under the <a href=\"http://www.gnu.org/licenses/gpl.html\">GPL 3</a> license")));
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
}

/*****************************************************************************/

void Window::stop() {
	m_active_project->stop(m_current_time);
	m_active_timers--;
	m_remove_project->setEnabled(true);
	updateDetails();
}

/*****************************************************************************/

void Window::cancel() {
	if (QMessageBox::question(this, tr("Question"), tr("Cancel this session?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
		m_active_project->stop();
		m_active_timers--;
		m_remove_project->setEnabled(true);
		updateDetails();
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

void Window::showReport() {
	Report report(m_active_model, this);
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
		disconnect(m_active_model, SIGNAL(rowsInserted(const QModelIndex&, int, int)), this, SLOT(sessionsInserted(const QModelIndex&, int, int)));
	}
	m_active_project = project;
	m_active_model = m_active_project->model();
	connect(m_active_model, SIGNAL(rowsInserted(const QModelIndex&, int, int)), this, SLOT(sessionsInserted(const QModelIndex&, int, int)));

	updateDisplay();
	updateSessionButtons();
	m_details->setModel(m_active_project->filterModel());
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
	m_details->scrollToBottom();
}

/*****************************************************************************/

void Window::sessionPressed(const QModelIndex& index) {
	int row = m_active_project->filterModel()->mapToSource(index).row();
	bool enabled = !m_active_model->isBilled(row) && (row + 1 < m_active_model->rowCount());
	m_edit_session->setEnabled(enabled && (index.column() < 4));
	m_remove_session->setEnabled(enabled);
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
	QModelIndex index = m_active_project->filterModel()->mapToSource(m_details->currentIndex());
	if (!(m_active_model->flags(index) & Qt::ItemIsEditable)) {
		return;
	}

	if (m_inline) {
		m_details->edit(m_details->currentIndex());
	} else {
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
	QFile file(m_filename);
	file.open(QIODevice::WriteOnly | QIODevice::Text);
	QXmlStreamWriter xml(&file);
	xml.setAutoFormatting(true);
	xml.writeStartDocument();
	xml.writeStartElement("kapow");
	int count = m_projects->topLevelItemCount();
	for (int i = 0; i < count; ++i) {
		Project* project = dynamic_cast<Project*>(m_projects->topLevelItem(i));
		if (project) {
			project->toXml(xml);
		}
	}
	xml.writeEndDocument();
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
	} else {
		m_stop->hide();
		m_start->show();
		m_cancel->setEnabled(false);
	}
}

/*****************************************************************************/

void Window::updateTrayIcon() {
	m_tray_icon->setIcon(QIcon(m_active_timers ? ":/kapow.png" : ":/kapow-inactive.png"));
	m_tray_icon->setToolTip(tr("%n timer(s) running", "", m_active_timers));
}

/*****************************************************************************/

void Window::updateWindowTitle(const QString& project) {
	setWindowTitle(project + ' ' + QChar(0x2014) + ' ' + tr("Kapow Punch Clock"));
}

/*****************************************************************************/
