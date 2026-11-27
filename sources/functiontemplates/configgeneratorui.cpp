/*
	Copyright 2006-2026 The QElectroTech Team
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
#include "configgeneratorui.h"

#include "configgenerator.h"

#include "../projectview.h"
#include "../qetproject.h"
#include "../qetdiagrameditor.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>

namespace
{
	QString tr(const char *text)
	{
		return QCoreApplication::translate("ConfigGeneratorUi", text);
	}
}

void ConfigGeneratorUi::run(QETDiagramEditor *editor)
{
	const QString title = tr("Generate plan from configuration");
	QSettings settings;
	const QString last = settings.value(QStringLiteral("configurator/last_dir"), QDir::homePath()).toString();
	const QString table = QFileDialog::getOpenFileName(
				editor, title, last, tr("Configuration tables (*.xlsx *.csv);;All files (*)"));
	if (table.isEmpty()) return;
	settings.setValue(QStringLiteral("configurator/last_dir"), QFileInfo(table).absolutePath());

	const ConfigGenerator::Table rows = ConfigGenerator::readTable(table);
	if (!rows.error.isEmpty()) {
		QMessageBox::warning(editor, title, rows.error);
		return;
	}

	const QFileInfo info(table);
	QString target = QFileDialog::getSaveFileName(
				editor, tr("Project the plan goes into"),
				info.absoluteDir().filePath(info.completeBaseName() + QStringLiteral(".qet")),
				tr("QElectroTech projects (*.qet)"), nullptr, QFileDialog::DontConfirmOverwrite);
	if (target.isEmpty()) return;
	if (!target.endsWith(QLatin1String(".qet"), Qt::CaseInsensitive)) target += QStringLiteral(".qet");

	ConfigGenerator::Options options;
	if (QFileInfo::exists(target)) {
			//Generated again: what to do with folios changed by hand
		QMessageBox box(QMessageBox::Question, title,
						tr("%1 exists. Objects which did not change keep their folios; changed ones are drawn again.\n\n"
						   "A folio changed by hand since it was generated:")
							.arg(QDir::toNativeSeparators(target)),
						QMessageBox::NoButton, editor);
		QPushButton *keep = box.addButton(tr("Keep it, the new one beside it"), QMessageBox::AcceptRole);
		QPushButton *overwrite = box.addButton(tr("Overwrite it"), QMessageBox::DestructiveRole);
		box.addButton(QMessageBox::Cancel);
		box.setDefaultButton(keep);
		box.exec();
		if (box.clickedButton() == overwrite) options.overwrite = true;
		else if (box.clickedButton() != keep) return;

		ProjectView *open = nullptr;
		for (ProjectView *view : editor->openedProjects())
			if (view->project() && QFileInfo(view->project()->filePath()) == QFileInfo(target)) open = view;
		if (ProjectView *view = open) {
			if (QMessageBox::question(editor, title,
									  tr("The project is open. Close it to generate into it?"))
					!= QMessageBox::Yes)
				return;
			if (!editor->closeProject(view)) return;
		}
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);
	const ConfigGenerator::Result result = ConfigGenerator::generate(table, target, options);
	QApplication::restoreOverrideCursor();

	if (!result.ok) {
		QMessageBox box(QMessageBox::Warning, title, result.error, QMessageBox::Ok, editor);
		if (!result.report.isEmpty()) box.setDetailedText(result.report.join(QLatin1Char('\n')));
		box.exec();
		return;
	}
	QString text = result.report.value(0);
	if (!result.conflicts.isEmpty())
		text += QStringLiteral("\n\n") + tr("Folios changed by hand were kept; the new ones follow them "
											"with [new] in their title. See the details.");
	QMessageBox box(result.conflicts.isEmpty() ? QMessageBox::Information : QMessageBox::Warning,
					title, text, QMessageBox::Ok, editor);
	box.setDetailedText((result.conflicts + result.report.mid(1)).join(QLatin1Char('\n')));
	box.exec();
	editor->openAndAddProject(result.target);
}
