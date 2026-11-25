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
// The layout file of the THM print (sources/marking/thmlayouts): a small
// made up file in the same form as the maker's (fixtures/
// thm_layouts_example.json, no data of any real article) turned into
// profiles and jobs the way the maker's software writes its THM jobs:
// S over the whole page, page height as label height and pitch, offset
// from print_position_mmp, H with the MultiMark speed and darkness, B30
// and O T,P for tear off, else O P, zO. Also the limits of the two device
// families and the usual and loaded article of a printer.
#include "marking/thmjob.h"
#include "marking/thmlayouts.h"
#include "marking/thmprinters.h"

#include <QSettings>
#include <QTemporaryDir>
#include <QTest>

class TestThmLayouts : public QObject
{
	Q_OBJECT

	QList<ThmLayouts::Variant> m_all;

	const ThmLayouts::Variant &variant(const char *article)
	{
		const int i = ThmLayouts::best(m_all, QString::fromLatin1(article));
		if (i < 0) qFatal("no test variant %s", article);
		return m_all.at(i);
	}

	ThmJob::Profile profileOf(const char *article, ThmJob::Family family)
	{
		ThmJob::Profile p;
		QString error;
		if (!ThmLayouts::profile(variant(article), family, &p, &error))
			qFatal("%s", qPrintable(error));
		return p;
	}

	private slots:
		void initTestCase()
		{
			QString error;
			m_all = ThmLayouts::read(QStringLiteral(THM_LAYOUTS_FIXTURE), &error);
			QVERIFY2(error.isEmpty(), qPrintable(error));
			QCOMPARE(m_all.size(), 6);
		}

		void readGeometry()
		{
			const ThmLayouts::Variant &v = variant("9900000002");
			QCOMPARE(v.columns, 3);              //markers sorted by x
			QCOMPARE(v.first_x, 2.0);
			QCOMPARE(v.column_pitch, 16.0);
			QCOMPARE(v.marker_width, 14.0);
			QCOMPARE(v.page_width, 50.0);
			QCOMPARE(v.orientation, 90);         //field "Marker", not "Project1"
			QCOMPARE(v.position_y, -0.5);
			QVERIFY(v.refusal.isEmpty());
		}

		void bestSkipsRefused()
		{
				//two variants of one article: the one whose layout is missing
				//is refused, the other one is taken
			const QList<int> both = ThmLayouts::find(m_all, QStringLiteral("9900000001"));
			QCOMPARE(both.size(), 2);
			QVERIFY(!m_all.at(both.at(1)).refusal.isEmpty());
			QCOMPARE(ThmLayouts::best(m_all, QStringLiteral("9900000001")), both.at(0));
			QCOMPARE(ThmLayouts::find(m_all, QStringLiteral("9900000001/test_missing.mcf")), QList<int>{both.at(1)});
		}

		void singleLabel()
		{
			const ThmJob::Profile p = profileOf("9900000001", ThmJob::Family::MultiMark);
			QVERIFY(qAbs(p.text_size - 3.175) < 0.01);  //9 pt
			QString error;
			const QByteArray job = ThmJob::build({{QStringLiteral("A1"), 2}}, p, &error);
			QVERIFY2(error.isEmpty(), qPrintable(error));
			const QByteArray expected = "m m\rzO\rJ\rH 40,12,T\rS l1;0,-0.5,9,9,60\rO P\rT 31,"
					+ ThmJob::number(2.5 + p.text_size / 2).toLatin1() + ",0,3,"
					+ ThmJob::number(p.text_size).toLatin1() + ";[J:c14]A1\rA 2\r";
			QCOMPARE(job, expected);
		}

		void threeColumnsTurned()
		{
			const ThmJob::Profile p = profileOf("9900000002", ThmJob::Family::MultiMark);
			QCOMPARE(p.text_size, 2.82);         //8 pt
			QString error;
			const QByteArray job = ThmJob::build({{QStringLiteral("A"), 1}, {QStringLiteral("B"), 1},
												  {QStringLiteral("C"), 1}}, p, &error);
			QVERIFY2(error.isEmpty(), qPrintable(error));
				//turned 90 degrees: the baseline runs up the field from its
				//bottom, half the text size right of the label's middle
			QCOMPARE(job, QByteArray("m m\rzO\rJ\rH 40,10,T,B30\rS l1;0,-0.5,12,12,50\rO T,P\r"
									 "T 10.41,12,90,3,2.82;[J:c12]A\r"
									 "T 26.41,12,90,3,2.82;[J:c12]B\r"
									 "T 42.41,12,90,3,2.82;[J:c12]C\rA 1\r"));
				//the Plus/MMP speed of the same variant
			QCOMPARE(profileOf("9900000002", ThmJob::Family::Mmp).speed, 50);
		}

		void pitchedStrip()
		{
				//page 3 x 5 mm with turned axes: 3 mm along the transport,
				//5 mm across
			const ThmJob::Profile p = profileOf("9900000003", ThmJob::Family::MultiMark);
			QCOMPARE(p.text_size, 2.12);         //6 pt
			QString error;
			const QByteArray job = ThmJob::build({{QStringLiteral("X"), 1}}, p, &error);
			QVERIFY2(error.isEmpty(), qPrintable(error));
			QCOMPARE(job, QByteArray("m m\rzO\rJ\rH 40,12,T\rS l1;0,0,3,3,5\rO P\r"
									 "T 0,2.56,0,3,2.12;[J:c5]X\rA 1\r"));
		}

		void refused()
		{
			const ThmLayouts::Variant &endless = variant("9900000004");
			QVERIFY(endless.refusal.contains(QStringLiteral("multi_rows")));
			QString error;
			QVERIFY(!ThmLayouts::profile(endless, ThmJob::Family::MultiMark, nullptr, &error));
			QVERIFY(error.contains(QStringLiteral("multi_rows")));
				//110 mm: wider than either print head
			QVERIFY(!ThmLayouts::profile(variant("9900000005"), ThmJob::Family::MultiMark, nullptr, &error));
			QVERIFY2(error.contains(QStringLiteral("106")), qPrintable(error));
		}

		void familyLimits()
		{
			ThmJob::Profile p = profileOf("9900000001", ThmJob::Family::MultiMark);
			p.heat = -15;
			QVERIFY(ThmJob::validate(p));
			p.family = ThmJob::Family::Mmp;
			QVERIFY(!ThmJob::validate(p));
			p.heat = 0;
			p.speed = 150;
			QVERIFY(!ThmJob::validate(p));
			p.family = ThmJob::Family::MultiMark;
			QVERIFY(ThmJob::validate(p));
			QCOMPARE(ThmJob::familyFromModel(QStringLiteral("5.36 Feb 23, 2021 (THM MultiMark)")),
					 ThmJob::Family::MultiMark);
			QCOMPARE(ThmJob::familyFromModel(QStringLiteral("4.29 Jan 03, 2017 (THM MMP)")),
					 ThmJob::Family::Mmp);
			QCOMPARE(ThmJob::familyFromModel(QString()), ThmJob::Family::Mmp);
		}

		void jobProfileSource()
		{
			ThmPrinter own;
			own.name = QStringLiteral("Test");
			own.model = QStringLiteral("THM MultiMark");
			own.profile.width = 12;
			own.profile.height = 6;
			ThmJob::Profile p;
			QString source, error;
				//an article the file knows: its layout, the printer's family
			QVERIFY(ThmLayouts::jobProfile(m_all, QStringLiteral("9900000002"), &own, &p, &source, &error));
			QCOMPARE(p.speed, 40);
			QVERIFY(source.contains(QStringLiteral("test_three.mcf")));
				//one it does not know: the printer's own profile
			QVERIFY(ThmLayouts::jobProfile(m_all, QStringLiteral("1234"), &own, &p, &source, &error));
			QCOMPARE(p.width, 12.0);
				//a refused one is never printed with something else
			QVERIFY(!ThmLayouts::jobProfile(m_all, QStringLiteral("9900000004"), &own, &p, &source, &error));
		}

		void usualAndLoaded()
		{
			QTemporaryDir dir;
			QVERIFY(dir.isValid());
			ThmPrinter p;
			p.id = QStringLiteral("thm1");
			p.name = QStringLiteral("Test");
			p.host = QStringLiteral("127.0.0.1");
			ThmPrinters::confirmLoading(&p, QStringLiteral("9900000001"), QStringLiteral("TEST SINGLE GE"),
										QDateTime(QDate(2026, 1, 2), QTime(3, 4, 5)), true);
			QVERIFY(!ThmPrinters::changedFromUsual(p));
			ThmPrinters::confirmLoading(&p, QStringLiteral("9900000002"), QStringLiteral("TEST THREE GE"),
										QDateTime(QDate(2026, 1, 3), QTime(8, 0)));
			QVERIFY(ThmPrinters::changedFromUsual(p));
			QCOMPARE(p.usual_material_no, QStringLiteral("9900000001"));
			p.roller = QStringLiteral("test roller");

			QSettings settings(dir.filePath(QStringLiteral("thm.ini")), QSettings::IniFormat);
			ThmPrinters::save(settings, {p});
			const QList<ThmPrinter> back = ThmPrinters::load(settings);
			QCOMPARE(back.size(), 1);
			QCOMPARE(back.first().material_no, QStringLiteral("9900000002"));
			QCOMPARE(back.first().usual_material_no, QStringLiteral("9900000001"));
			QCOMPARE(back.first().loaded_at, QDateTime(QDate(2026, 1, 3), QTime(8, 0)));
			QCOMPARE(back.first().roller, QStringLiteral("test roller"));
				//the target is chosen by what is loaded, not by the usual one
			QCOMPARE(ThmPrinters::choose(back, QStringLiteral("9900000002")).outcome,
					 ThmPrinters::Choice::Unique);
			QCOMPARE(ThmPrinters::choose(back, QStringLiteral("9900000001")).outcome,
					 ThmPrinters::Choice::NoMatch);
				//an article with a layout needs no own profile of the printer
			QVERIFY(ThmPrinters::validate(back.first(), nullptr, false));
			QVERIFY(!ThmPrinters::validate(back.first()));
		}
};

QTEST_GUILESS_MAIN(TestThmLayouts)
#include "tst_thmlayouts.moc"
