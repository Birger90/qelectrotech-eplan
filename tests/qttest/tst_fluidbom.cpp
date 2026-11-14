// SPDX-License-Identifier: GPL-2.0-or-later
#include <QtTest>

#include <QDomDocument>

#include "conductorproperties.h"
#include "fluid/fluidbom.h"
#include "qetapp.h"

	// qet.cpp needs it; the application is not linked
QString QETApp::m_interface_language;

// The fluid parts list: what is grouped, what each line says, and that the
// lines of a project are counted by kind and size, with the lengths typed
// into them added up and those without one counted.
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

		void lengthsAreRead()
		{
			double mm = 0;
			QVERIFY(FluidBom::lengthInMm("1.5 m", &mm));
			QCOMPARE(mm, 1500.0);
			QVERIFY(FluidBom::lengthInMm("1,25m", &mm));
			QCOMPARE(mm, 1250.0);
			QVERIFY(FluidBom::lengthInMm(" 80 cm ", &mm));
			QCOMPARE(mm, 800.0);
			QVERIFY(FluidBom::lengthInMm("350 MM", &mm));
			QCOMPARE(mm, 350.0);
			QVERIFY(FluidBom::lengthInMm("2", &mm));   // no unit: metres
			QCOMPARE(mm, 2000.0);
			QVERIFY(!FluidBom::lengthInMm("", &mm));
			QVERIFY(!FluidBom::lengthInMm("about 2 m", &mm));
			QVERIFY(!FluidBom::lengthInMm("2 ft", &mm));
			QVERIFY(!FluidBom::lengthInMm("-1 m", &mm));
		}

		void lengthsAreAddedUp()
		{
			using K = Fluid::LineKind;
			const QList<FluidBom::Line> lines {
				{K::Working, "8x6", "1.5 m"}, {K::Working, "8x6", "1200 mm"},
				{K::Working, "8x6", "50 cm"}};
			int rows = 0;
			const auto r = records(FluidBom::csv({}, lines, &rows));
			QCOMPARE(rows, 1);
			QCOMPARE(r.at(1).at(1), QString("3"));
			QCOMPARE(r.at(1).at(5), QString("size 8x6; total 3.2 m"));
		}

		void linesWithoutLengthAreCounted()
		{
			using K = Fluid::LineKind;
			const QList<FluidBom::Line> lines {
				{K::Working, "8x6", "1.5 m"}, {K::Working, "8x6", ""},
				{K::Working, "8x6", "1,7 m"}, {K::Working, "8x6", " "},
				{K::Pilot, "", "ca. 2 m"}, {K::Pilot, "", ""},
				{K::Exhaust, "6x4", ""}};
			int rows = 0;
			const auto r = records(FluidBom::csv({}, lines, &rows));
			QCOMPARE(rows, 3);
			QCOMPARE(r.at(1).at(1), QString("4"));
			QCOMPARE(r.at(1).at(5),
				 QString("size 8x6; total 3.2 m; 2 line(s) without length"));
				// A length not understood is named, not guessed at
			QCOMPARE(r.at(2).at(5),
				 QString("size not given; length to be measured; "
					 "1 length(s) not understood: ca. 2 m"));
				// No length at all: still to be measured, as before
			QCOMPARE(r.at(3).at(5),
				 QString("size 6x4; length to be measured"));
		}

		void lineLengthRoundTrip()
		{
			QDomDocument document;
			QDomElement element = document.createElement(QStringLiteral("conductor"));
			document.appendChild(element);
			ConductorProperties none;
			none.m_fluid_line = Fluid::LineKind::Working;
			none.toXml(element);
			QVERIFY2(!element.hasAttribute(QStringLiteral("length")),
				 "a line without a length writes no attribute");

			ConductorProperties properties;
			properties.m_fluid_line = Fluid::LineKind::Working;
			properties.m_wire_section = QStringLiteral("8x6");
			properties.m_length = QStringLiteral("1,5 m");
			QDomElement written = document.createElement(QStringLiteral("conductor"));
			properties.toXml(written);
			QCOMPARE(written.attribute(QStringLiteral("length")),
				 QStringLiteral("1,5 m"));
			ConductorProperties read_back;
			read_back.fromXml(written);
			QCOMPARE(read_back.m_length, QStringLiteral("1,5 m"));
			QCOMPARE(read_back.m_fluid_line, Fluid::LineKind::Working);
			QCOMPARE(read_back.m_wire_section, QStringLiteral("8x6"));

				// A file from before: no attribute, no length
			QDomElement old = document.createElement(QStringLiteral("conductor"));
			old.setAttribute(QStringLiteral("conductor_section"),
					 QStringLiteral("8x6"));
			ConductorProperties old_read;
			old_read.m_length = QStringLiteral("left over");
			old_read.fromXml(old);
			QVERIFY(old_read.m_length.isEmpty());
		}
};

QTEST_GUILESS_MAIN(tst_fluidbom)
#include "tst_fluidbom.moc"
