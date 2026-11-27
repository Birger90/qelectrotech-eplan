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
// SPDX-License-Identifier: GPL-2.0-or-later
// Fork: renders the Check panel offscreen for each project given and saves
// it as a PNG in <out-dir>, with the head line (verdict) in the report.
//     check_panel_probe <report.txt> <out-dir> <project.qet>...
// QET_PROBE_QM may name a .qm file to show the panel translated.
#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QLabel>
#include <QTemporaryDir>
#include <QTranslator>
#include <QTreeWidget>
#include <cstdio>

#include "../../sources/qetapp.h"
#include "../../sources/qetmessagebox.h"
#include "../../sources/qetproject.h"
#include "../../sources/ui/checkpaneldockwidget.h"

int main(int argc, char **argv)
{
	if (argc < 4) return 2;
	const QString report = QString::fromLocal8Bit(argv[1]);
	const QString output = QString::fromLocal8Bit(argv[2]);
	QTemporaryDir isolated;
	QByteArray config = "--config-dir=" + isolated.path().toUtf8();
	QByteArray data = "--data-dir=" + isolated.path().toUtf8();
	char *args[] = {argv[0], config.data(), data.data()};
	int count = 3;
	QApplication app(count, args);
	std::freopen(report.toLocal8Bit().constData(), "w", stdout);
	std::setbuf(stdout, nullptr);
	QTranslator translation;
	const QString qm = qEnvironmentVariable("QET_PROBE_QM");
	if (!qm.isEmpty() && translation.load(qm)) app.installTranslator(&translation);
	QETProject::setBackupEnabled(false);
	QET::QetMessageBox::setNonInteractive(true);
	auto *qet = new QETApp;
	Q_UNUSED(qet);
	int failed = 0;
	for (int i = 3; i < argc; ++i) {
		const QString path = QString::fromLocal8Bit(argv[i]);
		QETProject project(path);
		if (project.state() != QETProject::Ok) {
			std::printf("FEHLER %s: nicht geöffnet\n", qPrintable(path));
			++failed;
			continue;
		}
		CheckPanelDockWidget panel;
		panel.resize(900, qEnvironmentVariableIntValue("QET_PROBE_HEIGHT") > 0 ? qEnvironmentVariableIntValue("QET_PROBE_HEIGHT") : 560);
		panel.setProject(&project);
		panel.show();
		panel.run();
		QApplication::processEvents();
		if (auto *tree = panel.findChild<QTreeWidget *>()) tree->expandAll();
		QApplication::processEvents();
		QStringList heads;
		for (QLabel *label : panel.findChildren<QLabel *>())
			if (label->isVisibleTo(&panel) && !label->text().isEmpty()) heads << label->text();
		const QString png = QDir(output).filePath(QFileInfo(path).completeBaseName() + QStringLiteral(".png"));
		panel.grab().save(png);
		std::printf("%s: %s\n", qPrintable(QFileInfo(path).fileName()), qPrintable(heads.join(QStringLiteral(" | "))));
	}
	return failed ? 1 : 0;
}
