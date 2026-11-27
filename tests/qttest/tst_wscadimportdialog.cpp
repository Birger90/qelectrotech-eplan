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
// File > Import WSCAD project: the sPlan import dialog in its WSCAD kind.
// A WSCAD project is a folder with Database/WSCAD6.db; the folder, a page in
// it or the database lead to the same project; the program gets the folder,
// the target and --pdf; its progress steps read as text; the menu has the
// command. With QET_WSCAD_SCREENSHOT set, the dialog is saved there as an
// image. The translation itself (tools/wscad_import) has its own tests.

#include "import/splan/splanimportdialog.h"
#include "qetapptest.h"

#include <QFile>
#include <QLineEdit>
#include <QPushButton>

class TestWscadImportDialog : public QObject
{
	Q_OBJECT

	QTemporaryDir m_dir;
	QString m_project;	///< a WSCAD project folder with a space in its name

	private slots:
		void initTestCase()
		{
			QVERIFY(m_dir.isValid());
			m_project = QDir(m_dir.path()).filePath(QStringLiteral("Demo Projekt"));
			QVERIFY(QDir().mkpath(m_project + QStringLiteral("/Database")));
			QFile db(m_project + QStringLiteral("/Database/WSCAD6.db"));
			QVERIFY(db.open(QIODevice::WriteOnly));
			db.close();
			QFile page(m_project + QStringLiteral("/=A1+ST_Plan.0001.wsELD"));
			QVERIFY(page.open(QIODevice::WriteOnly));
			page.close();
		}

		void projectFolderFromFolderPageOrDatabase()
		{
			using D = SplanImportDialog;
			const QString folder = QDir::cleanPath(m_project);
			QCOMPARE(D::wscadProjectFolder(m_project), folder);
			QCOMPARE(D::wscadProjectFolder(m_project + QStringLiteral("/=A1+ST_Plan.0001.wsELD")), folder);
			QCOMPARE(D::wscadProjectFolder(m_project + QStringLiteral("/Database/WSCAD6.db")), folder);
			QCOMPARE(D::wscadProjectFolder(m_project + QStringLiteral("/Database")), folder);
			QVERIFY(D::wscadProjectFolder(m_dir.path()).isEmpty());
			QVERIFY(D::wscadProjectFolder(QString()).isEmpty());
		}

		void targetAndArguments()
		{
			using D = SplanImportDialog;
			const QString target = D::defaultTarget(m_project, D::Kind::Wscad);
			QCOMPARE(target, QDir(m_project).filePath(QStringLiteral("QET/Demo_Projekt_WSCAD_Import.qet")));
			// the sPlan kind is unchanged
			QCOMPARE(D::defaultTarget(QStringLiteral("C:/x/a b.spl8")), QStringLiteral("C:/x/QET/a_b.qet"));

			const QStringList args = D::arguments(QStringLiteral("C:/t/wscad_nach_qet.py"),
				m_project + QStringLiteral("/=A1+ST_Plan.0001.wsELD"), target, true, D::Kind::Wscad);
			QCOMPARE(args, (QStringList{QStringLiteral("-u"), QDir::toNativeSeparators(QStringLiteral("C:/t/wscad_nach_qet.py")),
						    QDir::toNativeSeparators(QDir::cleanPath(m_project)),
						    QDir::toNativeSeparators(target), QStringLiteral("--pdf")}));
			QVERIFY(!D::arguments(QStringLiteral("s.py"), m_project, target, false, D::Kind::Wscad)
					 .contains(QStringLiteral("--pdf")));
		}

		void progressSteps()
		{
			using D = SplanImportDialog;
			int percent = 0;
			QVERIFY(!D::progressText(QStringLiteral("FORTSCHRITT 86 verbindungen"), &percent, D::Kind::Wscad).isEmpty());
			QCOMPARE(percent, 86);
			QVERIFY(D::progressText(QStringLiteral("FORTSCHRITT 2 lesen"), nullptr, D::Kind::Wscad)
				!= D::progressText(QStringLiteral("FORTSCHRITT 2 lesen"), nullptr, D::Kind::Splan));
			QVERIFY(D::progressText(QStringLiteral("Seiten 27, Bauteile 311"), &percent, D::Kind::Wscad).isEmpty());
		}

		void findsTheProgram()
		{
			qputenv("QET_WSCAD_IMPORT", QByteArray(QET_WSCAD_SCRIPT));
			const QString found = SplanImportDialog::findScript(QCoreApplication::applicationDirPath(),
									    SplanImportDialog::Kind::Wscad);
			QCOMPARE(QFileInfo(found).fileName(), QStringLiteral("wscad_nach_qet.py"));
			QVERIFY(QFileInfo(found).isFile());
		}

		void dialogTakesTheFolder()
		{
			qputenv("QET_WSCAD_IMPORT", QByteArray(QET_WSCAD_SCRIPT));
			SplanImportDialog dialog(QetAppTest::editor(), SplanImportDialog::Kind::Wscad);
			QCOMPARE(dialog.windowTitle(), QStringLiteral("Import a WSCAD project"));
			const QList<QLineEdit *> edits = dialog.findChildren<QLineEdit *>();
			QVERIFY(edits.size() >= 2);
			dialog.setSource(m_project + QStringLiteral("/Database/WSCAD6.db"));
			// the target follows the project, as long as nobody typed one
			QCOMPARE(QDir::fromNativeSeparators(edits.at(1)->text()),
				 SplanImportDialog::defaultTarget(m_project, SplanImportDialog::Kind::Wscad));
			QPushButton *start = nullptr;
			for (QPushButton *b : dialog.findChildren<QPushButton *>())
				if (b->text() == QStringLiteral("Import")) start = b;
			QVERIFY(start);
			QVERIFY(start->isEnabled());
			dialog.setSource(m_dir.path());			// no WSCAD project
			QVERIFY(!start->isEnabled());

			const QString shot = qEnvironmentVariable("QET_WSCAD_SCREENSHOT");
			if (!shot.isEmpty()) {
				dialog.setSource(m_project);
				dialog.show();
				QCoreApplication::processEvents();
				QVERIFY(dialog.grab().save(shot));
			}
		}

		void menuHasTheCommand()
		{
			QETDiagramEditor *e = QetAppTest::editor();
			QVERIFY(e);
			QAction *a = e->findChild<QAction *>(QStringLiteral("import_wscad"));
			QVERIFY(a);
			QVERIFY(a->text().contains(QStringLiteral("WSCAD")));
		}
};

int main(int argc, char **argv)
{
	return QetAppTest::run<TestWscadImportDialog>(argc, argv);
}
#include "tst_wscadimportdialog.moc"
