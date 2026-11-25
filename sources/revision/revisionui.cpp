/*
	Copyright 2006-2025 The QElectroTech Team
	This file is part of QElectroTech.

	QElectroTech is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	(at your option) any later version.

	QElectroTech is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with QElectroTech.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "revisionui.h"

#include "projectrevisions.h"
#include "../diagram.h"
#include "../projectview.h"
#include "../qetdiagrameditor.h"
#include "../qetgraphicsitem/conductor.h"
#include "../qetgraphicsitem/element.h"
#include "../qetproject.h"
#include "../shortcutmanager.h"

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDesktopServices>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGraphicsView>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QStatusBar>
#include <QTreeWidget>
#include <QUrl>
#include <QVBoxLayout>

using namespace QetRevision;

namespace {

/**
	Project > Revision > Close revision: index and short text, the rest by
	itself (title blocks, PDF, snapshot, marks reset), then saved
*/
void closeRevision(QETDiagramEditor *editor)
{
	ProjectView *view = editor->currentProjectView();
	QETProject *project = editor->currentProject();
	if (!view || !project) return;
	if (project->isReadOnly()) {
		QMessageBox::information(editor, QObject::tr("Close revision"),
								 QObject::tr("The project is read-only."));
		return;
	}
	if (project->filePath().isEmpty()) {
		QMessageBox::information(editor, QObject::tr("Close revision"),
								 QObject::tr("Save the project to a file first: the snapshot "
											 "of the revision is kept beside it."));
		return;
	}
	ProjectRevisions *revisions = project->revisionManager();
	QDialog dialog(editor);
	dialog.setWindowTitle(QObject::tr("Close revision"));
	auto *form = new QFormLayout(&dialog);
	auto *index = new QLineEdit(revisions->proposedIndex(), &dialog);
	auto *text = new QLineEdit(&dialog);
	text->setPlaceholderText(QObject::tr("What changed, e.g. Motor M2 added"));
	text->setMinimumWidth(320);
	auto *pdf = new QCheckBox(QObject::tr("Keep a PDF with the changes marked"), &dialog);
	pdf->setChecked(true);
	QString since;
	if (!revisions->hasBaseline()) {
		since = QObject::tr("First revision: the project as it is now becomes the reference.");
	} else {
		const int n = int(revisions->changes().size());
		since = QObject::tr("%n change(s) since revision %1.", "", n).arg(revisions->lastIndex());
	}
	auto *note = new QLabel(since + QLatin1Char(' ')
							+ QObject::tr("The index goes in every title block; the project is saved."),
							&dialog);
	note->setWordWrap(true);
	form->addRow(QObject::tr("Revision index:"), index);
	form->addRow(QObject::tr("Short text:"), text);
	form->addRow(pdf);
	form->addRow(note);
	auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
	buttons->button(QDialogButtonBox::Ok)->setText(QObject::tr("Close revision"));
	QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
	QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
	form->addRow(buttons);
	text->setFocus();
	if (dialog.exec() != QDialog::Accepted) return;

	QApplication::setOverrideCursor(Qt::WaitCursor);
	const ProjectRevisions::CloseResult result =
			revisions->closeRevision(index->text(), text->text(), pdf->isChecked(), false);
	QApplication::restoreOverrideCursor();
	if (!result.ok) {
		QMessageBox::warning(editor, QObject::tr("Close revision"),
							 QObject::tr("The revision was not closed: %1").arg(result.error));
		return;
	}
	view->save();
	editor->statusBar()->showMessage(
				QObject::tr("Revision %1 closed: %n change(s), snapshot in %2", "", result.changes)
				.arg(index->text().trimmed(), QDir::toNativeSeparators(revisions->folder())), 8000);
}

/// A state to compare: the project open now, a revision's snapshot, a file
struct State {
	QString label;
	QString index;		///< of a revision
	QString path;		///< empty: the project open now
	bool baseline = false;	///< the last revision: its fingerprint is in the project
};

/**
	Project > Revision > Compare: what changed from one state to another,
	as a list (a double click shows it) and as a PDF with the changes marked
*/
void compareRevisions(QETDiagramEditor *editor)
{
	QETProject *project = editor->currentProject();
	if (!project) return;
	ProjectRevisions *revisions = project->revisionManager();
	const QList<ProjectRevisions::Entry> entries = revisions->entries();

	QList<State> states;
	states << State{QObject::tr("Current state"), QString(), QString(), false};
	for (int i = int(entries.size()) - 1 ; i >= 0 ; --i) {
		const ProjectRevisions::Entry &e = entries.at(i);
		states << State{QObject::tr("Revision %1 (%2) %3").arg(e.index, e.date, e.text), e.index,
						revisions->snapshotPath(e), i == entries.size() - 1};
	}

	QDialog dialog(editor);
	dialog.setWindowTitle(QObject::tr("Compare revisions"));
	auto *layout = new QVBoxLayout(&dialog);
	auto *form = new QFormLayout();
	auto *older = new QComboBox(&dialog);
	auto *newer = new QComboBox(&dialog);
	for (int i = 1 ; i < states.size() ; ++i) older->addItem(states.at(i).label, i);
	older->addItem(QObject::tr("Project file..."), -1);
	for (int i = 0 ; i < states.size() ; ++i) newer->addItem(states.at(i).label, i);
	newer->addItem(QObject::tr("Project file..."), -1);
	form->addRow(QObject::tr("Older state:"), older);
	form->addRow(QObject::tr("Newer state:"), newer);
	layout->addLayout(form);
	auto *summary = new QLabel(&dialog);
	summary->setWordWrap(true);
	layout->addWidget(summary);
	auto *list = new QTreeWidget(&dialog);
	list->setHeaderLabels({QObject::tr("Folio"), QObject::tr("Kind"), QObject::tr("Name"), QObject::tr("Change")});
	list->setRootIsDecorated(false);
	list->header()->setStretchLastSection(true);
	layout->addWidget(list);
	auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
	auto *pdf = buttons->addButton(QObject::tr("PDF with marks..."), QDialogButtonBox::ActionRole);
	QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
	layout->addWidget(buttons);

		//Chosen files, by combo
	QString older_file, newer_file;
	Fingerprint before, now;
	const auto stateOf = [&](QComboBox *combo, const QString &file) {
		const int i = combo->currentData().toInt();
		if (i < 0) return State{QFileInfo(file).fileName(), QString(), file, false};
		return states.value(i);
	};
	const auto fingerprintOf = [&](const State &s, QString *error) {
		if (s.path.isEmpty() && s.index.isEmpty()) return fingerprint(project);
		if (s.baseline && revisions->hasBaseline()) return revisions->baseline();
		return fingerprintOfFile(s.path, error);
	};
	const auto refresh = [&]() {
		list->clear();
		QString error;
		QApplication::setOverrideCursor(Qt::WaitCursor);
		before = fingerprintOf(stateOf(older, older_file), &error);
		if (error.isEmpty()) now = fingerprintOf(stateOf(newer, newer_file), &error);
		QApplication::restoreOverrideCursor();
		if (!error.isEmpty()) {
			summary->setText(error);
			pdf->setEnabled(false);
			return;
		}
		const QList<Change> changes = compare(before, now);
		summary->setText(QObject::tr("%n change(s). A double click shows it on its folio.", "",
									 int(changes.size())));
		for (const Change &c : changes) {
			const Item &i = c.item();
			const Fingerprint &where = c.kind == Change::Removed ? before : now;
			auto *row = new QTreeWidgetItem(list, {where.folio_names.value(i.folio), kindName(i.kind),
												   i.name, i.kind == QLatin1Char('f') && c.kind == Change::Changed
														   ? QObject::tr("title block changed")
														   : changeName(c.kind)});
			row->setData(0, Qt::UserRole, i.folio);
			row->setData(1, Qt::UserRole, i.uuid);
			row->setData(2, Qt::UserRole, c.kind == Change::Removed);
		}
		for (int i = 0 ; i < 3 ; ++i) list->resizeColumnToContents(i);
		pdf->setEnabled(true);
	};
	const auto pickFile = [&](QComboBox *combo, QString &file) {
		if (combo->currentData().toInt() >= 0) return true;
		const QString chosen = QFileDialog::getOpenFileName(
					&dialog, QObject::tr("Project to compare"), QFileInfo(project->filePath()).path(),
					QObject::tr("QElectroTech projects (*.qet *.qetz *.qetdb)"));
		if (chosen.isEmpty()) { combo->setCurrentIndex(0); return true; }
		file = chosen;
		return true;
	};
	QObject::connect(older, QOverload<int>::of(&QComboBox::activated), &dialog, [&]() {
		if (pickFile(older, older_file)) refresh();
	});
	QObject::connect(newer, QOverload<int>::of(&QComboBox::activated), &dialog, [&]() {
		if (pickFile(newer, newer_file)) refresh();
	});
	QObject::connect(list, &QTreeWidget::itemDoubleClicked, &dialog, [&](QTreeWidgetItem *row) {
		const State s = stateOf(newer, newer_file);
		if (!s.path.isEmpty() || !s.index.isEmpty() || row->data(2, Qt::UserRole).toBool()) return;
		Diagram *diagram = project->diagramByUuid(QUuid(row->data(0, Qt::UserRole).toString()));
		if (!diagram) return;
		diagram->showMe();
		diagram->clearSelection();
		const QUuid uuid(row->data(1, Qt::UserRole).toString());
		for (QGraphicsItem *graphics : diagram->items()) {
			const auto *element = qgraphicsitem_cast<Element *>(graphics);
			const auto *conductor = qgraphicsitem_cast<Conductor *>(graphics);
			if ((element && element->uuid() == uuid) || (conductor && conductor->uuid() == uuid)) {
				graphics->setSelected(true);
				for (QGraphicsView *v : diagram->views()) v->centerOn(graphics);
				break;
			}
		}
	});
	QObject::connect(pdf, &QPushButton::clicked, &dialog, [&]() {
		const State newer_state = stateOf(newer, newer_file);
		const bool current = newer_state.path.isEmpty() && newer_state.index.isEmpty();
		const QString suggested = QDir(revisions->folder()).filePath(
					QFileInfo(project->filePath()).completeBaseName() + QObject::tr("_comparison.pdf"));
		const QString path = QFileDialog::getSaveFileName(&dialog, QObject::tr("PDF with marks"),
														  suggested, QObject::tr("PDF (*.pdf)"));
		if (path.isEmpty()) return;
		QDir().mkpath(QFileInfo(path).path());
		const QString label = current ? revisions->proposedIndex()
									  : newer_state.index.isEmpty() ? QStringLiteral("*") : newer_state.index;
		QString error;
		bool ok = false;
		QApplication::setOverrideCursor(Qt::WaitCursor);
		if (current) {
			revisions->beginComparison(before, label);
			ok = ProjectRevisions::exportPdf(project, path, &error);
			revisions->endComparison();
		} else {
			QETProject other(newer_state.path);
			if (other.state() == QETProject::Ok) {
				other.revisionManager()->beginComparison(before, label);
				ok = ProjectRevisions::exportPdf(&other, path, &error);
			} else {
				error = QObject::tr("%1 could not be opened").arg(newer_state.path);
			}
		}
		QApplication::restoreOverrideCursor();
		if (!ok) {
			QMessageBox::warning(&dialog, QObject::tr("PDF with marks"), error);
			return;
		}
		QDesktopServices::openUrl(QUrl::fromLocalFile(path));
	});
	if (entries.isEmpty()) {
		summary->setText(QObject::tr("No revision is closed yet: choose a project file as the older state."));
		pdf->setEnabled(false);
	} else {
		refresh();
	}
	dialog.resize(720, 460);
	dialog.exec();
}

} // namespace

void RevisionUi::addMenu(QETDiagramEditor *editor, QMenu *project_menu)
{
	QMenu *menu = project_menu->addMenu(QObject::tr("Revision"));
	auto *close = menu->addAction(QObject::tr("Close revision..."));
	close->setStatusTip(QObject::tr("Index and short text in the title blocks, a snapshot and a PDF of the "
									"changes beside the project"));
	ShortcutManager::instance().registerAction(close, "diagrameditor.revision_close",
											   QObject::tr("Diagram editors"), QKeySequence());
	QObject::connect(close, &QAction::triggered, editor, [editor]() { closeRevision(editor); });

	auto *marks = menu->addAction(QObject::tr("Mark changes"));
	marks->setCheckable(true);
	marks->setStatusTip(QObject::tr("Show what changed since the last revision with a cloud, "
									"on screen and in print and PDF"));
	ShortcutManager::instance().registerAction(marks, "diagrameditor.revision_marks",
											   QObject::tr("Diagram editors"), QKeySequence());
	QObject::connect(marks, &QAction::triggered, editor, [editor](bool on) {
		if (QETProject *project = editor->currentProject()) project->revisionManager()->setMarksShown(on);
	});

	auto *compare = menu->addAction(QObject::tr("Compare revisions..."));
	compare->setStatusTip(QObject::tr("What changed between two states, as a list and as a PDF with marks"));
	ShortcutManager::instance().registerAction(compare, "diagrameditor.revision_compare",
											   QObject::tr("Diagram editors"), QKeySequence());
	QObject::connect(compare, &QAction::triggered, editor, [editor]() { compareRevisions(editor); });

	auto *folder = menu->addAction(QObject::tr("Open revision folder"));
	QObject::connect(folder, &QAction::triggered, editor, [editor]() {
		if (QETProject *project = editor->currentProject()) {
			const QString path = project->revisionManager()->folder();
			if (QFileInfo::exists(path)) QDesktopServices::openUrl(QUrl::fromLocalFile(path));
		}
	});

	QObject::connect(menu, &QMenu::aboutToShow, editor, [=]() {
		QETProject *project = editor->currentProject();
		const bool writable = project && !project->isReadOnly();
		close->setEnabled(writable);
		marks->setEnabled(writable && project->revisionManager()->hasBaseline());
		marks->setChecked(project && project->revisionManager()->marksShown());
		compare->setEnabled(project != nullptr);
		folder->setEnabled(project && !project->filePath().isEmpty()
						   && QFileInfo::exists(project->revisionManager()->folder()));
	});
}
