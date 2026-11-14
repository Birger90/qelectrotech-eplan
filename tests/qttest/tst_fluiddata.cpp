// SPDX-License-Identifier: GPL-2.0-or-later
#include <QtTest>

#include <QDomDocument>

#include "conductorproperties.h"
#include "fluid/fluiddata.h"
#include "fluid/fluidtypes.h"
#include "qetapp.h"

// qet.cpp reads it; the test has no QETApp
QString QETApp::m_interface_language;

// The fluid power data model: the words written to the files, the <fluid>
// block of an element, and that nothing changes for an electric conductor.
class tst_fluiddata : public QObject
{
	Q_OBJECT

	static FluidData valve52()
	{
		FluidData d;
		d.m_kind = FluidData::Kind::Valve;
		FluidData::Position a;
		a.paths << FluidData::Path{"1", "4"} << FluidData::Path{"2", "3"}
				<< FluidData::Path{"5", ""};
		FluidData::Position b;
		b.paths << FluidData::Path{"1", "2"} << FluidData::Path{"4", "5"}
				<< FluidData::Path{"3", ""};
		d.m_positions << a << b;
		d.m_initial_position = 2;
		d.m_left.types << "solenoid" << "manual";
		d.m_left.pilot = true;
		d.m_right.spring = true;
		d.m_flow_qn_nl_min = 750.0;
		return d;
	}

	static FluidData roundTrip(const FluidData &d)
	{
		QDomDocument doc;
		auto def = doc.createElement("definition");
		doc.appendChild(def);
		const auto e = d.toXml(doc);
		if (!e.isNull())
			def.appendChild(e);
			// through text, as a file would
		QDomDocument read;
		read.setContent(doc.toString());
		return FluidData::fromElementDefinition(read.documentElement());
	}

	private slots:
		void wordsRoundTrip()
		{
			using namespace Fluid;
			for (Medium m : {Medium::Electric, Medium::Pneumatic,
							 Medium::Hydraulic, Medium::Mechanic,
							 Medium::Logic})
				QCOMPARE(mediumFromString(mediumToString(m)), m);
			for (PortEnd p : {PortEnd::None, PortEnd::Silencer, PortEnd::Plug})
				QCOMPARE(portEndFromString(portEndToString(p)), p);
			for (LineKind k : {LineKind::None, LineKind::Working,
							   LineKind::Pilot, LineKind::Exhaust})
				QCOMPARE(lineKindFromString(lineKindToString(k)), k);
		}

		void unknownWordsAreTheOldDefault()
		{
			QCOMPARE(Fluid::mediumFromString(QString()),
					 Fluid::Medium::Electric);
			QCOMPARE(Fluid::mediumFromString("steam"),
					 Fluid::Medium::Electric);
			QCOMPARE(Fluid::portEndFromString("cork"), Fluid::PortEnd::None);
			QCOMPARE(Fluid::lineKindFromString("x"), Fluid::LineKind::None);
		}

		void onlyLikeMediaConnect()
		{
			using Fluid::Medium;
			QVERIFY(Fluid::canConnect(Medium::Electric, Medium::Electric));
			QVERIFY(Fluid::canConnect(Medium::Pneumatic, Medium::Pneumatic));
			QVERIFY(!Fluid::canConnect(Medium::Pneumatic, Medium::Electric));
			QVERIFY(!Fluid::canConnect(Medium::Pneumatic, Medium::Hydraulic));
			QVERIFY(!Fluid::canConnect(Medium::Logic, Medium::Logic));
		}

		void nullDataWritesNothing()
		{
			QDomDocument doc;
			QVERIFY(FluidData().isNull());
			QVERIFY(FluidData().toXml(doc).isNull());
			QVERIFY(roundTrip(FluidData()).isNull());
		}

		void elementWithoutBlockIsNull()
		{
			QDomDocument doc;
			doc.setContent(QString(
				"<definition type=\"element\" link_type=\"simple\">"
				"<description/></definition>"));
			QVERIFY(FluidData::fromElementDefinition(
						doc.documentElement()).isNull());
		}

		void valveRoundTrips()
		{
			const FluidData d = valve52();
			QVERIFY(d.isValid());
			const FluidData r = roundTrip(d);
			QVERIFY(!r.isNull());
			QCOMPARE(r.m_positions.size(), 2);
			QCOMPARE(r.m_positions.at(0).paths.at(2).from, QString("5"));
			QVERIFY(r.m_positions.at(0).paths.at(2).to.isEmpty());
			QCOMPARE(r.m_initial_position, 2);
			QCOMPARE(r.m_left.types, (QStringList{"solenoid", "manual"}));
			QVERIFY(r.m_left.pilot);
			QVERIFY(r.m_right.spring);
			QCOMPARE(r.m_flow_qn_nl_min, 750.0);
			QVERIFY(r == d);
		}

		void cylinderRoundTrips()
		{
			FluidData d;
			d.m_kind = FluidData::Kind::Cylinder;
			d.m_action = FluidData::CylinderAction::Single;
			d.m_spring = FluidData::SpringSide::Rod;
			d.m_cushioning = FluidData::Cushioning::Adjustable;
			d.m_magnet_piston = true;
			d.m_bore_mm = 32;
			d.m_rod_mm = 12;
			d.m_stroke_mm = 100.5;
			const FluidData r = roundTrip(d);
			QVERIFY(r == d);

				// a plain double acting cylinder writes only its action
			FluidData plain;
			plain.m_kind = FluidData::Kind::Cylinder;
			QDomDocument doc;
			const auto cyl = plain.toXml(doc).firstChildElement("cylinder");
			QCOMPARE(cyl.attributes().count(), 1);
			QCOMPARE(cyl.attribute("action"), QString("double"));
		}

		void otherKindsRoundTrip()
		{
			for (auto k : {FluidData::Kind::Source, FluidData::Kind::Treatment,
						   FluidData::Kind::FlowControl,
						   FluidData::Kind::LogicValve,
						   FluidData::Kind::Sensor, FluidData::Kind::Other,
						   FluidData::Kind::Solenoid}) {
				FluidData d;
				d.m_kind = k;
				QVERIFY(roundTrip(d) == d);
			}
		}

		void functionAndParametersRoundTrip()
		{
			FluidData d;
			d.m_kind = FluidData::Kind::Other;
			d.m_function = "pressure_relief";
			d.m_parameters << FluidData::Parameter{"p_open", "5", "MPa"}
						   << FluidData::Parameter{"note", "x", ""};
			QDomDocument doc;
			const auto e = d.toXml(doc);
			QCOMPARE(e.attribute("function"), QString("pressure_relief"));
			QCOMPARE(e.elementsByTagName("parameter").count(), 2);
			const FluidData r = roundTrip(d);
			QVERIFY(r == d);
			QCOMPARE(r.parameter("p_open"), QString("5"));
			QCOMPARE(r.parameter("missing", "-"), QString("-"));
				// nothing written when not given
			FluidData plain;
			plain.m_kind = FluidData::Kind::Other;
			QVERIFY(!plain.toXml(doc).hasAttribute("function"));
		}

		void prefixRoundTripsAndIsOptional()
		{
			FluidData d = valve52();
			d.m_prefix = "QM";
			QDomDocument doc;
			QCOMPARE(d.toXml(doc).attribute("prefix"), QString("QM"));
			QVERIFY(roundTrip(d) == d);
			QCOMPARE(roundTrip(d).m_prefix, QString("QM"));
				// without a prefix nothing is written
			QVERIFY(!valve52().toXml(doc).hasAttribute("prefix"));
		}

		void solenoidSidesCountsEachSideOnce()
		{
			FluidData d = valve52();
			QCOMPARE(d.solenoidSides(), 1);
			d.m_right.types << "solenoid";
			QCOMPARE(d.solenoidSides(), 2);
			d.m_left.types.clear();
			d.m_right.types = QStringList{"lever"};
			QCOMPARE(d.solenoidSides(), 0);
				// a coil is not a valve: no sides of its own
			FluidData coil;
			coil.m_kind = FluidData::Kind::Solenoid;
			QCOMPARE(coil.solenoidSides(), 0);
			QCOMPARE(FluidData::kindToString(coil.m_kind), QString("solenoid"));
		}

		void openPortIsAFreeFluidPort()
		{
			using Fluid::Medium;
			using Fluid::PortEnd;
			QVERIFY(Fluid::isOpenPort(Medium::Pneumatic, PortEnd::None, 0));
			QVERIFY(Fluid::isOpenPort(Medium::Hydraulic, PortEnd::None, 0));
				// a line, a silencer or a plug closes it
			QVERIFY(!Fluid::isOpenPort(Medium::Pneumatic, PortEnd::None, 1));
			QVERIFY(!Fluid::isOpenPort(Medium::Pneumatic, PortEnd::Silencer, 0));
			QVERIFY(!Fluid::isOpenPort(Medium::Pneumatic, PortEnd::Plug, 0));
				// an electric terminal is never a fluid port
			QVERIFY(!Fluid::isOpenPort(Medium::Electric, PortEnd::None, 0));
			QVERIFY(!Fluid::isOpenPort(Medium::Logic, PortEnd::None, 0));
		}

		void portEndsAreDrawnAwayFromTheElement()
		{
			QVERIFY(Fluid::portEndLines(Fluid::PortEnd::None).isEmpty());
			for (auto end : {Fluid::PortEnd::Silencer, Fluid::PortEnd::Plug}) {
				const auto lines = Fluid::portEndLines(end);
				QVERIFY(!lines.isEmpty());
					// starts at the port, never reaches back into the element
				QCOMPARE(lines.first().p1(), QPointF(0, 0));
				QRectF box;
				for (const QLineF &l : lines) {
					box |= QRectF(l.p1(), l.p2()).normalized();
					QVERIFY(l.x1() >= 0 && l.x2() >= 0);
				}
					// symmetric across the line, small enough for the grid
				QCOMPARE(box.top(), -box.bottom());
				QVERIFY(box.right() <= 20);
			}
		}

		void invalidValveIsRefused()
		{
			FluidData d = valve52();
			d.m_initial_position = 3;   // only two positions
			QVERIFY(!d.isValid());
			QVERIFY(roundTrip(d).isNull());

			FluidData none;
			none.m_kind = FluidData::Kind::Valve; // no position at all
			QVERIFY(!none.isValid());

			FluidData five = valve52();
			for (int i = 0; i < 3; ++i)
				five.m_positions << FluidData::Position();
			QVERIFY(!five.isValid());

			QDomDocument doc;
			doc.setContent(QString(
				"<definition><fluid kind=\"teapot\"/></definition>"));
			QVERIFY(FluidData::fromElementDefinition(
						doc.documentElement()).isNull());
		}

		void electricConductorIsWrittenAsBefore()
		{
			QDomDocument doc;
			auto e = doc.createElement("conductor");
			ConductorProperties().toXml(e);
			QVERIFY(!e.hasAttribute("fluid_line"));
		}

		void lineKindOfAConductorRoundTrips()
		{
			ConductorProperties p;
			p.m_fluid_line = Fluid::LineKind::Pilot;
			QVERIFY(p != ConductorProperties());

			QDomDocument doc;
			auto e = doc.createElement("conductor");
			p.toXml(e);
			QCOMPARE(e.attribute("fluid_line"), QString("pilot"));

			ConductorProperties r;
			r.fromXml(e);
			QCOMPARE(r.m_fluid_line, Fluid::LineKind::Pilot);

				// a read conductor differs from a fresh one in other
				// fields: compare with one that was read as well
			QDomDocument plain_doc;
			auto plain_e = plain_doc.createElement("conductor");
			ConductorProperties().toXml(plain_e);
			ConductorProperties plain;
			plain.fromXml(plain_e);
			QVERIFY(r != plain);
			plain.m_fluid_line = Fluid::LineKind::Pilot;
			QVERIFY(r == plain);
		}

		void lineKindMergesPerField()
		{
			ConductorProperties a, b;
			a.m_fluid_line = Fluid::LineKind::Working;
			b.m_fluid_line = Fluid::LineKind::Working;
			ConductorProperties same;
			same.applyForEqualAttributes({a, b});
			QCOMPARE(same.m_fluid_line, Fluid::LineKind::Working);

			b.m_fluid_line = Fluid::LineKind::Exhaust;
			ConductorProperties mixed;
			mixed.applyForEqualAttributes({a, b});
			QCOMPARE(mixed.m_fluid_line, Fluid::LineKind::None);
		}

			// the panel hands over only what the user changed
		void lineKindIsAnEditLikeAnyOther()
		{
			ConductorProperties shown, edited;
			edited.m_fluid_line = Fluid::LineKind::Pilot;
			edited.style = Qt::DashLine;

			ConductorProperties target;
			target.m_function = "keep me";
			target.applyChanges(shown, edited);
			QCOMPARE(target.m_fluid_line, Fluid::LineKind::Pilot);
			QCOMPARE(target.style, Qt::DashLine);
			QCOMPARE(target.m_function, QString("keep me"));

				// an edit of something else leaves the kind alone
			ConductorProperties other_edit;
			other_edit.m_function = "x";
			target.applyChanges(shown, other_edit);
			QCOMPARE(target.m_fluid_line, Fluid::LineKind::Pilot);
		}
};

QTEST_GUILESS_MAIN(tst_fluiddata)
#include "tst_fluiddata.moc"
