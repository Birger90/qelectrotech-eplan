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
// Label printing > Print THM labels...: the markers of a project in the preview,
// equal texts merged, and the printer chosen by the article loaded: one
// printer preselected, none preselected among several, a message when
// none fits. The printers are made up; nothing is sent. With
// QET_THM_SCREENSHOT set, the dialog is saved there as an image.

#include "marking/thmprintdialog.h"
#include "marking/thmprinters.h"
#include "qetapptest.h"
#include "qetproject.h"

#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QRadioButton>
#include <QPlainTextEdit>

namespace {
ThmPrinter printer(const QString &id, const QString &name, const QString &material)
{
	ThmPrinter p;
	p.id = id;
	p.name = name;
	p.host = QHostAddress(QHostAddress::LocalHost).toString();
	p.material_no = material;
	p.profile.width = 12;
	p.profile.height = 6;
	p.profile.gap = 2;
	return p;
}
}

class TestThmPrintDialog : public QObject
{
	Q_OBJECT

	private slots:
		void initTestCase()
		{
			ThmPrinters::save({printer(QStringLiteral("thm1"), QStringLiteral("Printer A"), QStringLiteral("BSP-1")),
							   printer(QStringLiteral("thm2"), QStringLiteral("Printer B"), QStringLiteral("BSP-2")),
							   printer(QStringLiteral("thm3"), QStringLiteral("Printer C"), QStringLiteral("BSP-2"))});
		}

		void previewAndTarget()
		{
			QETProject project(QFINDTESTDATA("fixtures/every_feature.qet"));
			QCOMPARE(project.state(), QETProject::Ok);
			ThmPrintDialog dialog(&project);
			auto *preview = dialog.findChild<QTableWidget *>(QStringLiteral("thm_preview"));
			auto *article = dialog.findChild<QComboBox *>(QStringLiteral("thm_article"));
			auto *target = dialog.findChild<QComboBox *>(QStringLiteral("thm_printer"));
			auto *message = dialog.findChild<QLabel *>(QStringLiteral("thm_target_message"));
			auto *print = dialog.findChild<QPushButton *>(QStringLiteral("thm_print"));
			auto *merge = dialog.findChild<QCheckBox *>(QStringLiteral("thm_merge"));
			QVERIFY(preview && article && target && message && print && merge);

				//Wire numbers, both ends, equal texts in one row
			const QList<ThmJob::Label> labels = dialog.labels();
			QVERIFY(!labels.isEmpty());
			QCOMPARE(preview->rowCount(), labels.size());
			QCOMPARE(preview->item(0, 0)->text(), labels.first().text);
			QCOMPARE(preview->item(0, 1)->text(), QString::number(labels.first().count));
			const int total = ThmJob::labelCount(labels);
			merge->setChecked(false);
			QVERIFY(preview->rowCount() > labels.size());
			QCOMPARE(ThmJob::labelCount(dialog.labels()), total);
			merge->setChecked(true);

				//The articles of the printers are offered
			QVERIFY(article->findData(QStringLiteral("BSP-1")) >= 0);
			QVERIFY(article->findData(QStringLiteral("BSP-2")) >= 0);

				//One printer loaded with it: chosen
			article->setCurrentText(QStringLiteral("BSP-1"));
			QCOMPARE(target->count(), 1);
			QCOMPARE(target->currentData().toString(), QStringLiteral("thm1"));
			QVERIFY(print->isEnabled());

				//Several: the user picks, none is chosen
			article->setCurrentText(QStringLiteral("BSP-2"));
			QCOMPARE(target->count(), 3);
			QVERIFY(target->currentData().toString().isEmpty());
			QVERIFY(!print->isEnabled());
			target->setCurrentIndex(target->findData(QStringLiteral("thm3")));
			QCOMPARE(target->currentData().toString(), QStringLiteral("thm3"));

				//None: a message names the article
			article->setCurrentText(QStringLiteral("BSP-77"));
			QCOMPARE(target->count(), 0);
			QVERIFY(!target->isEnabled());
			QVERIFY(message->text().contains(QStringLiteral("BSP-77")));
			QVERIFY(!print->isEnabled());

			const QString shot = qEnvironmentVariable("QET_THM_SCREENSHOT");
			if (!shot.isEmpty()) {
				article->setCurrentText(QStringLiteral("BSP-1"));
				dialog.resize(1000, 640);
				dialog.show();
				QCoreApplication::processEvents();
				QVERIFY(dialog.grab().save(shot));
			}
		}

		void typedByHand()
		{
				//One label per line, "text;n" for n, empty lines skipped
			QList<ThmJob::Label> l = ThmPrintDialog::freeLabels(QStringLiteral(" -K1 \n\n-X1:1;3\n-K1\nA;B"), 2, true);
			QCOMPARE(l.size(), 3);
			QCOMPARE(l.at(0).text, QStringLiteral("-K1"));
			QCOMPARE(l.at(0).count, 4);
			QCOMPARE(l.at(1).text, QStringLiteral("-X1:1"));
			QCOMPARE(l.at(1).count, 6);
			QCOMPARE(l.at(2).text, QStringLiteral("A;B"));  // not a number: part of the text
			QCOMPARE(ThmPrintDialog::freeLabels(QStringLiteral("-K1\n-K1"), 1, false).size(), 2);

				//Without a project the dialog takes the typed texts
			ThmPrintDialog dialog(nullptr);
			auto *text = dialog.findChild<QPlainTextEdit *>(QStringLiteral("thm_free_text"));
			auto *by_hand = dialog.findChild<QRadioButton *>(QStringLiteral("thm_from_text"));
			auto *preview = dialog.findChild<QTableWidget *>(QStringLiteral("thm_preview"));
			QVERIFY(text && by_hand && preview);
			QVERIFY(by_hand->isChecked());
			text->setPlainText(QStringLiteral("T1\nT2;2"));
			QCOMPARE(preview->rowCount(), 2);
			QCOMPARE(preview->item(1, 0)->text(), QStringLiteral("T2"));
			QCOMPARE(ThmJob::labelCount(dialog.labels()), 3);
		}
};

int main(int argc, char **argv)
{
	return QetAppTest::run<TestThmPrintDialog>(argc, argv);
}
#include "tst_thmprintdialog.moc"
