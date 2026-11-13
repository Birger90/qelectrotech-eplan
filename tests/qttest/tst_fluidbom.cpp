// SPDX-License-Identifier: GPL-2.0-or-later
#include <QtTest>

#include "fluid/fluidbom.h"

// The fluid parts list: what is grouped, what each line says, and that the
// lines of a project are counted by kind and size, their length left open.
class tst_fluidbom : public QObject
{
	Q_OBJECT

	static QList<QStringList> records(const QByteArray &csv)
	{
		QList<QStringList> out;
		for (const QByteArray &line : csv.split('\n')) {
			if (line.isEmpty())
				continue;
			// ';' between fields, a field in quotes may hold ';' and "" for "
			QStringList fields;
			QString field;
			bool quoted = false;
			const QString text = QString::fromUtf8(line);
			for (int i = 0; i < text.size(); ++i) {
				const QChar ch = text.at(i);
				if (ch == QLatin1Char('"')) {
					if (quoted && i + 1 < text.size() && text.at(i + 1) == QLatin1Char('"')) {
						field += ch;
						++i;
					} else {
						quoted = !quoted;
					}
				} else if (ch == QLatin1Char(';') && !quoted) {
					fields << field;
					field.clear();
				} else {
					field += ch;
				}
			}
			fields << field;
			out << fields;
		}
		return out;
	}

	private slots:
		void identicalPartsShareALine()
		{
			FluidBom::Part a {"5/2 valve", "Directional valve", "2 positions",
							  "Festo", "123", "-QM1"};
			FluidBom::Part b = a;
			b.label = "-QM2";
			FluidBom::Part c = a;
			c.reference = "456";   // another order number: another line
			c.label = "-QM3";
			int rows = 0;
			const auto r = records(FluidBom::csv({a, b, c}, {}, &rows));
			QCOMPARE(rows, 2);
			QCOMPARE(r.size(), 3);                 // header + 2
			QCOMPARE(r.at(1).at(1), QString("2"));
			QCOMPARE(r.at(1).at(8), QString("-QM1, -QM2"));
			QCOMPARE(r.at(2).at(1), QString("1"));
			QCOMPARE(r.at(2).at(7), QString("456"));
		}

		void valveAndCylinderFigures()
		{
			FluidData v;
			v.m_kind = FluidData::Kind::Valve;
			v.m_positions << FluidData::Position() << FluidData::Position();
			v.m_left.types << "solenoid";
			v.m_right.spring = true;
			const QString dv = FluidBom::details(v);
			QVERIFY(dv.contains("2 position"));
			QVERIFY(dv.contains("left: solenoid"));
			QVERIFY(dv.contains("right: spring"));

			FluidData z;
			z.m_kind = FluidData::Kind::Cylinder;
			z.m_action = FluidData::CylinderAction::Single;
			z.m_spring = FluidData::SpringSide::Rod;
			z.m_bore_mm = 20;
			z.m_rod_mm = 8;
			z.m_stroke_mm = 50;
			QCOMPARE(FluidBom::details(z),
					 QString("single acting; bore 20 mm; rod 8 mm; stroke 50 mm; "
							 "spring on the rod side"));
			QVERIFY(FluidBom::details(FluidData()).isEmpty());
		}

		void linesCountedByKindAndSize()
		{
			using K = Fluid::LineKind;
			const QList<FluidBom::Line> lines {
				{K::Working, "8x6"}, {K::Working, "8x6"}, {K::Pilot, "4x2.7"},
				{K::Working, "6x4"}, {K::None, "8x6"}};   // not a fluid line
			int rows = 0;
			const auto r = records(FluidBom::csv({}, lines, &rows));
			QCOMPARE(rows, 3);
			QCOMPARE(r.at(1).at(1), QString("2"));
			QCOMPARE(r.at(1).at(3), QString("Working line"));
			QVERIFY(r.at(1).at(5).contains("8x6"));
			QVERIFY(r.at(1).at(5).contains("length to be measured"));
			QCOMPARE(r.at(2).at(3), QString("Pilot line"));
			QCOMPARE(r.at(3).at(1), QString("1"));
		}
};

QTEST_GUILESS_MAIN(tst_fluidbom)
#include "tst_fluidbom.moc"
