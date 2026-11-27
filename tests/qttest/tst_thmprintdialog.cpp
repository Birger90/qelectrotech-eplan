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

#include "marking/thmlabelsheet.h"
#include "marking/thmprintdialog.h"
#include "marking/thmprinters.h"
#include "marking/thmrender.h"
#include "qetapptest.h"
#include "qetproject.h"

#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
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

		void pictureJob()
		{
				//A TM-I 18 as its layout gives it: row 65 x 10.5 mm, label
				//18 mm from x 32, text field 16 x 4 mm at the top
			ThmJob::Profile p;
			p.family = ThmJob::Family::MultiMark;
			p.width = 18;
			p.height = 10.5;
			p.page_width = 65;
			p.first_x = 32;
			p.margin_left = 1;
			p.margin_right = 1;
			p.field_top = 0;
			p.field_bottom = 4;
			p.text_size = 3;
			p.offset_y = -0.5;
			const QImage row = ThmRender::renderRow({QStringLiteral("-X1:1")}, p);
			QCOMPARE(row.size(), QSize(768, 124));  // 300 dpi, as M-Print PRO's picture
				//Black only inside the text field
			int black = 0, outside = 0;
			for (int y = 0; y < row.height(); ++y)
				for (int x = 0; x < row.width(); ++x)
					if (qGray(row.pixel(x, y)) < 128) {
						++black;
						const double mx = x / ThmRender::kDotsPerMm, my = y / ThmRender::kDotsPerMm;
						if (mx < 33 || mx > 49 || my > 4.2) ++outside;
					}
			QVERIFY(black > 100);
			QCOMPARE(outside, 0);

			QString error;
			const QByteArray job = ThmRender::build({{QStringLiteral("-X1:1"), 2}, {QStringLiteral("-X1:2"), 1}}, p, &error);
			QVERIFY2(error.isEmpty(), qPrintable(error));
			QVERIFY(job.startsWith(QByteArray("eIMG;*\r\ndPNG;0\r\n\x1b:\x89PNG")));
			QCOMPARE(job.count("eIMG;*"), 2);  // two different rows
			QVERIFY(job.contains("S l1;0,-0.5,10.5,10.5,65\r\nI:Field1;0,0,0;0\r\nA2\r\n")
					|| job.contains("Sl1;0,-0.5,10.5,10.5,65\r\nI:Field1;0,0,0;0\r\nA2\r\n"));
			QVERIFY(job.contains("\x1b" "end-of-data\r\nmm\r\nJ\r\n"));
				//A text too long for the field is shrunk, not cut
			QVERIFY(ThmRender::overflows(p, QStringLiteral("-X100:1234567890")));
			QVERIFY(!ThmRender::overflows(p, QStringLiteral("-K1")));
			const QImage longer = ThmRender::renderRow({QStringLiteral("-X100:1234567890")}, p);
			for (int y = 0; y < longer.height(); ++y)
				for (int x = 0; x < longer.width(); ++x)
					if (qGray(longer.pixel(x, y)) < 128) QVERIFY(x / ThmRender::kDotsPerMm >= 32.9 && x / ThmRender::kDotsPerMm <= 49.1);
		}

		void numberingSeries()
		{
			ThmPrintDialog::Series s;
			s.prefix = QStringLiteral("-X1:");
			s.start = QStringLiteral("1");
			s.end = QStringLiteral("5");
			s.step = 2;
			QCOMPARE(ThmPrintDialog::series(s), (QStringList{QStringLiteral("-X1:1"), QStringLiteral("-X1:3"), QStringLiteral("-X1:5")}));
			s.start = QStringLiteral("3");
			s.end = QStringLiteral("1");
			s.step = 1;
			s.repeat = 2;
			s.digits = 2;
			s.prefix.clear();
			QCOMPARE(ThmPrintDialog::series(s), (QStringList{QStringLiteral("03"), QStringLiteral("03"), QStringLiteral("02"),
															 QStringLiteral("02"), QStringLiteral("01"), QStringLiteral("01")}));
			ThmPrintDialog::Series plc;
			plc.kind = ThmPrintDialog::Series::Plc;
			plc.prefix = QStringLiteral("I");
			plc.start = QStringLiteral("4.6");
			plc.end = QStringLiteral("5.1");
			QCOMPARE(ThmPrintDialog::series(plc), (QStringList{QStringLiteral("I4.6"), QStringLiteral("I4.7"),
															   QStringLiteral("I5.0"), QStringLiteral("I5.1")}));
			ThmPrintDialog::Series letters;
			letters.kind = ThmPrintDialog::Series::Letters;
			letters.start = QStringLiteral("a");
			letters.end = QStringLiteral("C");
			QCOMPARE(ThmPrintDialog::series(letters), (QStringList{QStringLiteral("A"), QStringLiteral("B"), QStringLiteral("C")}));
			QString error;
			plc.start = QStringLiteral("4.8");
			QVERIFY(ThmPrintDialog::series(plc, &error).isEmpty());
			QVERIFY(!error.isEmpty());
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
				//Started strip: the labels before the first stay empty
			auto *first = dialog.findChild<QSpinBox *>(QStringLiteral("thm_first_label"));
			QVERIFY(first);
			first->setValue(3);
			QCOMPARE(dialog.labels().first().text, QString());
			QCOMPARE(ThmJob::labelCount(dialog.labels()), 5);
			first->setValue(1);
			auto *sheet = dialog.findChild<ThmLabelSheet *>(QStringLiteral("thm_label_sheet"));
			QVERIFY(sheet);
			QCOMPARE(sheet->texts(), (QStringList{QStringLiteral("T1"), QStringLiteral("T2"), QStringLiteral("T2")}));
			QVERIFY(sheet->isEditable());

				//A picture of the sheet with a real material, when asked:
				//QET_THM_SHEET_SCREENSHOT=<png>, QET_THM_LAYOUTS=<layout file>,
				//QET_THM_SHEET_ARTICLE=<article>
			const QString shot = qEnvironmentVariable("QET_THM_SHEET_SCREENSHOT");
			if (!shot.isEmpty()) {
				auto *article = dialog.findChild<QComboBox *>(QStringLiteral("thm_article"));
				text->setPlainText(QStringLiteral("-K1\n-K2\n-X1:1\n-X1:2\n-Q1\n-F3"));
				article->setCurrentText(qEnvironmentVariable("QET_THM_SHEET_ARTICLE", QStringLiteral("2005610000")));
				dialog.resize(1400, 720);
				dialog.show();
				QCoreApplication::processEvents();
				QVERIFY(dialog.grab().save(shot));
			}
		}
};

int main(int argc, char **argv)
{
	return QetAppTest::run<TestThmPrintDialog>(argc, argv);
}
#include "tst_thmprintdialog.moc"
