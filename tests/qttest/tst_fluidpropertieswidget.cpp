// SPDX-License-Identifier: GPL-2.0-or-later
#include "qetapptest.h"

#include "diagram.h"
#include "projectview.h"
#include "qetgraphicsitem/element.h"
#include "qetgraphicsitem/terminal.h"
#include "qetproject.h"
#include "ui/elementpropertieswidget.h"
#include "ui/fluidpropertieswidget.h"

#include <QDoubleSpinBox>
#include <QFile>
#include <QTabWidget>
#include <QTemporaryDir>
#include <QUndoStack>

// The "Fluid power" tab of a placed symbol: there only for a symbol with
// fluid data, it sets the bore, rod and stroke of one placed cylinder
// through the folio's undo stack, the symbol's values staying for the
// others.
namespace {

QString definition(const QString &name, const QString &uuid,
				   const QString &fluid)
{
	return QStringLiteral(
				"<element name=\"%1.elmt\">"
				"<definition width=\"30\" hotspot_y=\"21\" version=\"0.100.0\" type=\"element\""
				" link_type=\"simple\" hotspot_x=\"15\" height=\"42\">"
				"<uuid uuid=\"%2\"/>"
				"<names><name lang=\"en\">%1</name></names>"
				"<informations/>%3"
				"<description><rect x=\"-5\" y=\"-6\" width=\"10\" height=\"12\" antialias=\"true\""
				" style=\"line-style:normal;line-weight:normal;filling:none;color:black\"/>"
				"</description></definition></element>").arg(name, uuid, fluid);
}

QString placed(const QString &name, const QString &uuid, int x)
{
	return QStringLiteral(
				"<element type=\"embed://import/t/%1.elmt\" uuid=\"%2\" x=\"%3\" y=\"100\" z=\"10\""
				" orientation=\"0\"><terminals/><inputs/></element>").arg(name, uuid).arg(x);
}

const QString uuid_a = QStringLiteral("{11111111-1111-4111-8111-111111111111}");
const QString uuid_b = QStringLiteral("{22222222-2222-4222-8222-222222222222}");
const QString uuid_box = QStringLiteral("{44444444-4444-4444-8444-444444444444}");

QString project()
{
	return QStringLiteral(
				"<project title=\"fluid\" version=\"0.100.1\">"
				"<diagram title=\"f\" cols=\"8\" rows=\"4\" colsize=\"60\" rowsize=\"80\">"
				"<elements>%1%2%3</elements><conductors/></diagram>"
				"<collection><category name=\"import\"><category name=\"t\">%4%5"
				"</category></category></collection></project>")
			.arg(placed(QStringLiteral("cylinder"), uuid_a, 100),
				 placed(QStringLiteral("cylinder"), uuid_b, 200),
				 placed(QStringLiteral("box"), uuid_box, 300),
				 definition(QStringLiteral("cylinder"),
							QStringLiteral("{33333333-3333-4333-8333-333333333333}"),
							QStringLiteral("<fluid kind=\"cylinder\"><cylinder action=\"double\""
										   " bore_mm=\"32\" rod_mm=\"12\" stroke_mm=\"100\"/></fluid>")),
				 definition(QStringLiteral("box"),
							QStringLiteral("{55555555-5555-4555-8555-555555555555}"),
							QString()));
}

void eagerFolios()
{
	qputenv("QET_EAGER_FOLIOS", "1");
}

bool hasTab(QWidget *widget, const QString &title)
{
	for (QTabWidget *tabs : widget->findChildren<QTabWidget *>())
		for (int i = 0 ; i < tabs->count() ; ++i)
			if (tabs->tabText(i) == title)
				return true;
	return false;
}

} // namespace

class tst_fluidpropertieswidget : public QObject
{
	Q_OBJECT

	QTemporaryDir m_dir;
	Diagram *m_diagram = nullptr;
	QHash<QString, Element *> m_elements;

	private slots:
		void initTestCase()
		{
			QETDiagramEditor *editor = QetAppTest::editor();
			QVERIFY(editor);
			const QString path = m_dir.filePath(QStringLiteral("fluid.qet"));
			QFile f(path);
			QVERIFY(f.open(QIODevice::WriteOnly));
			f.write(project().toUtf8());
			f.close();
			QVERIFY(editor->openAndAddProject(path, false));
			QETProject *project = editor->currentProjectView()->project();
			QCOMPARE(project->diagrams().size(), 1);
			m_diagram = project->diagrams().first();
			for (QGraphicsItem *item : m_diagram->items())
				if (item->type() == Element::Type) {
					auto *element = static_cast<Element *>(item);
					m_elements.insert(element->uuid().toString(), element);
				}
			QCOMPARE(m_elements.size(), 3);
		}

			// Only a symbol with fluid data gets the tab
		void tabOnlyForFluidSymbols()
		{
			ElementPropertiesWidget cylinder(m_elements.value(uuid_a));
			QVERIFY(hasTab(&cylinder, QStringLiteral("Fluid power")));
			ElementPropertiesWidget box(m_elements.value(uuid_box));
			QVERIFY(!hasTab(&box, QStringLiteral("Fluid power")));
		}

			// Without values of its own, a placed cylinder has the
			// symbol's, and nothing to undo comes of the tab
		void symbolValuesWithoutOverride()
		{
			Element *a = m_elements.value(uuid_a);
			QVERIFY(a->elementData().m_fluid_override.isNull());
			QCOMPARE(a->elementData().fluid(), a->elementData().m_fluid);
			QCOMPARE(a->elementData().fluid().m_bore_mm, 32.0);
			FluidPropertiesWidget widget(a);
			QVERIFY(!widget.associatedUndo());
		}

			// A value set on one cylinder is that cylinder's alone, one
			// step on the undo stack
		void setUndoRedo()
		{
			Element *a = m_elements.value(uuid_a);
			Element *b = m_elements.value(uuid_b);
			QUndoStack &stack = m_diagram->undoStack();
			const int steps = stack.count();

			FluidPropertiesWidget widget(a);
			const auto boxes = widget.findChildren<QDoubleSpinBox *>();
			QCOMPARE(boxes.size(), 3);
				// bore, rod, stroke, in the order of the form
			boxes.at(0)->setValue(50.0);
			boxes.at(2)->setValue(250.0);
			widget.apply();

			QCOMPARE(stack.count(), steps + 1);
			QCOMPARE(a->elementData().fluid().m_bore_mm, 50.0);
			QCOMPARE(a->elementData().fluid().m_rod_mm, 12.0);
			QCOMPARE(a->elementData().fluid().m_stroke_mm, 250.0);
				// the definition and the other cylinder are untouched
			QCOMPARE(a->elementData().m_fluid.m_bore_mm, 32.0);
			QCOMPARE(b->elementData().fluid().m_bore_mm, 32.0);
			QCOMPARE(b->elementData().fluid().m_stroke_mm, 100.0);

				// written into the placed element
			QDomDocument doc;
			QHash<Terminal *, int> table;
			const QDomElement own = a->toXml(doc, table)
					.firstChildElement(FluidOverride::xmlTagName());
			QVERIFY(!own.isNull());
			QCOMPARE(own.attribute(QStringLiteral("bore_mm")).toDouble(), 50.0);
			QVERIFY(b->toXml(doc, table)
					.firstChildElement(FluidOverride::xmlTagName()).isNull());

			stack.undo();
			QVERIFY(a->elementData().m_fluid_override.isNull());
			QCOMPARE(a->elementData().fluid().m_bore_mm, 32.0);
			stack.redo();
			QCOMPARE(a->elementData().fluid().m_bore_mm, 50.0);

				// back to the symbol's values: nothing of its own is left
			FluidPropertiesWidget again(a);
			for (QDoubleSpinBox *box : again.findChildren<QDoubleSpinBox *>())
				box->setValue(0.0);
			again.apply();
			QVERIFY(a->elementData().m_fluid_override.isNull());
			QVERIFY(a->toXml(doc, table)
					.firstChildElement(FluidOverride::xmlTagName()).isNull());
		}
};

QET_APP_TEST_MAIN_PREPARED(tst_fluidpropertieswidget, eagerFolios)

#include "tst_fluidpropertieswidget.moc"
