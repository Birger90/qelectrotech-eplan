// SPDX-License-Identifier: GPL-2.0-or-later
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
#include "qetapptest.h"
#include "conductorproperties.h"
#include "ElementsCollection/xmlelementcollection.h"
#include "factory/elementfactory.h"
#include "qetgraphicsitem/conductor.h"
#include "qetgraphicsitem/conductorendlabels.h"
#include "qetgraphicsitem/element.h"
#include "qetgraphicsitem/terminal.h"
#include "qetproject.h"

/**
	The conductor's text written at each end: the setting is saved and
	opened again (and absent from older files), the text lies along the
	wire without ever being upside down, and the item follows the setting.
*/
class tst_conductorendlabels : public QObject
{
	Q_OBJECT

	Diagram *m_diagram = nullptr;

	Element *place(const QPointF &pos)
	{
		int state = 0;
		Element *element = ElementFactory::Instance()->createElement(
			ElementsLocation(QStringLiteral("embed://import/end.elmt"), m_diagram->project()),
			nullptr, &state);
		if (state || !element)
			return nullptr;
		m_diagram->addItem(element);
		element->setPos(pos);
		return element;
	}

	static int labelItems(const Conductor *conductor)
	{
		int count = 0;
		for (QGraphicsItem *item : conductor->childItems())
			if (item->type() == ConductorEndLabels::Type)
				++count;
		return count;
	}

	private slots:
		void initTestCase()
		{
			m_diagram = QetAppTest::diagram();
			QVERIFY(m_diagram);
			QDomDocument doc;
			doc.setContent(QStringLiteral(
				R"(<definition type="element" link_type="simple" width="10" height="10" hotspot_x="0" hotspot_y="5">
<names><name lang="en">end</name></names><description>
<terminal x="0" y="0" orientation="e" name="1"/></description></definition>)"));
			QVERIFY(m_diagram->project()->embeddedElementCollection()->addElementDefinition(
				QStringLiteral("import"), QStringLiteral("end"), doc.documentElement()));
		}

		void settingIsOnlyWrittenWhenOn()
		{
			ConductorProperties properties;
			QVERIFY(!properties.m_end_labels);
			QDomDocument doc;
			QDomElement off = doc.createElement(QStringLiteral("conductor"));
			properties.toXml(off);
			QVERIFY(!off.hasAttribute(QStringLiteral("end_labels")));

			properties.m_end_labels = true;
			QDomElement on = doc.createElement(QStringLiteral("conductor"));
			properties.toXml(on);
			ConductorProperties read;
			read.fromXml(on);
			QVERIFY(read.m_end_labels);

				//A file saved before has none
			ConductorProperties old;
			old.fromXml(off);
			QVERIFY(!old.m_end_labels);
			ConductorProperties copy = old;
			QVERIFY(copy == old);
			copy.m_end_labels = true;
			QVERIFY(!(copy == old));
		}

		void textLiesAlongTheWireNeverUpsideDown()
		{
			QRectF rect;
				//Wire running right from its end: the text lies inward
			QTransform t = ConductorEndLabels::labelTransform(
				QPointF(10, 10), QPointF(5, 0), 20, &rect);
			QCOMPARE(t.map(QPointF(0, 0)), QPointF(10, 10));
			QVERIFY(rect.left() > 0);
			QCOMPARE(t.map(QPointF(1, 0)), QPointF(11, 10));

				//Wire running left: turned, so the text lies toward the terminal
			t = ConductorEndLabels::labelTransform(
				QPointF(10, 10), QPointF(-5, 0), 20, &rect);
			QCOMPARE(t.map(QPointF(1, 0)), QPointF(11, 10));
			QVERIFY(rect.right() < 0);

				//Wire running down: read from bottom to top
			t = ConductorEndLabels::labelTransform(
				QPointF(10, 10), QPointF(0, 5), 20, &rect);
			QCOMPARE(t.map(QPointF(1, 0)), QPointF(10, 9));
			QVERIFY(rect.right() < 0);

				//Wire running up: read from bottom to top too
			t = ConductorEndLabels::labelTransform(
				QPointF(10, 10), QPointF(0, -5), 20, &rect);
			QCOMPARE(t.map(QPointF(1, 0)), QPointF(10, 9));
			QVERIFY(rect.left() > 0);
		}

		void endsSkipStretchesOfNoLength()
		{
			const QList<QPointF> points{QPointF(0, 0), QPointF(0, 0),
						    QPointF(10, 0), QPointF(10, 10)};
			QPointF end, inward;
			QVERIFY(ConductorEndLabels::endOf(points, true, &end, &inward));
			QCOMPARE(end, QPointF(0, 0));
			QCOMPARE(inward, QPointF(10, 0));
			QVERIFY(ConductorEndLabels::endOf(points, false, &end, &inward));
			QCOMPARE(end, QPointF(10, 10));
			QCOMPARE(inward, QPointF(0, -10));
			QVERIFY(!ConductorEndLabels::endOf({QPointF(1, 1)}, true, &end, &inward));
			QVERIFY(!ConductorEndLabels::endOf({QPointF(1, 1), QPointF(1, 1)}, true, &end, &inward));
		}

		void conductorFollowsTheSetting()
		{
			Element *a = place(QPointF(100, 100));
			Element *b = place(QPointF(300, 200));
			QVERIFY(a && b);
			auto *conductor = new Conductor(a->terminals().first(), b->terminals().first());
			m_diagram->addItem(conductor);
			QCOMPARE(labelItems(conductor), 0);

			ConductorProperties properties = conductor->properties();
			properties.text = QStringLiteral("W12");
			properties.m_end_labels = true;
			conductor->setProperties(properties);
			QCOMPARE(labelItems(conductor), 1);

			properties.m_end_labels = false;
			conductor->setProperties(properties);
			QCOMPARE(labelItems(conductor), 0);
		}
};

QET_APP_TEST_MAIN(tst_conductorendlabels)
#include "tst_conductorendlabels.moc"
