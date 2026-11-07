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
#include "panelmenu.h"

#include "panel3dview.h"
#include "panelcommands.h"
#include "panelimport.h"
#include "articledialog.h"
#include "eplanproject.h"
#include "paneldxf.h"
#include "panellayout.h"
#include "panelstep.h"
#include "panelassistant.h"
#include <QCoreApplication>
#include <QStatusBar>
#include "../ElementsCollection/xmlelementcollection.h"
#include "../NameList/nameslist.h"
#include "../QPropertyUndoCommand/qpropertyundocommand.h"
#include "../diagram.h"
#include "../diagramview.h"
#include "../factory/elementfactory.h"
#include "../qet.h"
#include "../qetapp.h"
#include "../qetdiagrameditor.h"
#include "../qetgraphicsitem/element.h"
#include "../qetgraphicsitem/qetshapeitem.h"
#include "../qetgraphicsitem/terminalelement.h"
#include "../qetproject.h"
#include "../TerminalStrip/terminalstrip.h"
#include "../TerminalStrip/physicalterminal.h"
#include "../TerminalStrip/realterminal.h"
#include "../qetversion.h"
#include "../undocommand/addgraphicsobjectcommand.h"
#include "../undocommand/changeelementinformationcommand.h"
#include "../undocommand/deleteqgraphicsitemcommand.h"
#include "../diagramcontent.h"

#include <QPointer>
#include <QGuiApplication>
#include <limits>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGraphicsObject>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPlainTextEdit>
#include <QProgressDialog>
#include <QSettings>
#include <QPushButton>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSet>
#include <QSpinBox>
#include <QTableWidget>
#include <QTextStream>
#include <QUndoStack>
#include <QUuid>
#include <QVBoxLayout>

#include <algorithm>

namespace {

	/// paper millimetres per scene unit when a folio is printed on A4
	/// landscape, the reference for the nominal scale 1:N
const double kPaperMmPerUnit = 0.2658;

QRectF sceneRectOf(QetShapeItem *shape)
{
	return shape->mapToScene(shape->rect().normalized()).boundingRect();
}

QString num(double v, int decimals = 1)
{
	QString s = QString::number(v, 'f', decimals);
	if (s.contains(QLatin1Char('.'))) {
		while (s.endsWith(QLatin1Char('0'))) s.chop(1);
		if (s.endsWith(QLatin1Char('.'))) s.chop(1);
	}
	return s.replace(QLatin1Char('.'), QLatin1Char(','));
}

double infoNumber(const DiagramContext &info, const QString &key)
{
	return info.value(key).toString().replace(QLatin1Char(','), QLatin1Char('.')).toDouble();
}

double cellNumber(QTableWidget *table, int row, int column)
{
	QTableWidgetItem *item = table->item(row, column);
	return item ? item->text().replace(QLatin1Char(','), QLatin1Char('.')).toDouble() : 0;
}

QDoubleSpinBox *mmBox(double value, double max, int decimals, QWidget *parent)
{
	auto *b = new QDoubleSpinBox(parent);
	b->setRange(0, max);
	b->setDecimals(decimals);
	b->setSuffix(QStringLiteral(" mm"));
	b->setValue(value);
	return b;
}

	/// a line edit with a "..." button that picks a STEP file
QWidget *stepPicker(QLineEdit *edit, QWidget *parent)
{
	auto *w = new QWidget(parent);
	auto *l = new QHBoxLayout(w);
	l->setContentsMargins(0, 0, 0, 0);
	auto *b = new QPushButton(QStringLiteral("..."), w);
	b->setMaximumWidth(32);
	l->addWidget(edit, 1);
	l->addWidget(b);
	QObject::connect(b, &QPushButton::clicked, w, [edit, w]() {
		const QString file = QFileDialog::getOpenFileName(
					w, PanelMenu::tr("STEP model"), QFileInfo(edit->text()).absolutePath(),
					PanelMenu::tr("STEP files (*.stp *.step *.STP *.STEP)"));
		if (!file.isEmpty()) edit->setText(QDir::toNativeSeparators(file));
	});
	return w;
}

QDialogButtonBox *okCancel(QDialog *dialog)
{
	auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dialog);
	QObject::connect(buttons, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
	QObject::connect(buttons, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
	return buttons;
}

/* ------------------------------------------------------------------ */

/**
	@brief The drilling view: drawn over a folio, not saved with it (the
	folio writes only the items it knows). Shows every hole the
	manufacturing will get at its real size, the device outlines from
	their part sizes and the keep-out areas.
*/
class PanelHoleOverlay : public QGraphicsObject
{
	public:
		explicit PanelHoleOverlay(Diagram *diagram) : m_diagram(diagram)
		{
			setZValue(1e6);
			setAcceptedMouseButtons(Qt::NoButton);
			setFlag(QGraphicsItem::ItemIsSelectable, false);
		}

		void refresh()
		{
			prepareGeometryChange();
			m_holes.clear();
			m_outlines.clear();
			m_bad.clear();
			m_rect = QRectF();
			if (!m_diagram) return;
			for (QetShapeItem *surface : PanelLayout::surfaces(m_diagram))
			{
				const PanelLayout layout(surface);
				const double k = layout.mmPerUnit() > 0 ? layout.mmPerUnit() : 1;
				m_rect |= sceneRectOf(surface).adjusted(-20, -20, 20, 20);
				for (const PanelHole &h : layout.holes()) {
					Hole s;
					s.centre = layout.toScene(QPointF(h.x, h.y));
					s.w = h.d1 / k;
					s.h = (h.type == PanelHole::Rectangle || h.type == PanelHole::Slot ? h.d2 : h.d1) / k;
					s.round = h.type != PanelHole::Rectangle;
					s.thread = h.type == PanelHole::Thread;
					s.angle = h.angle;
					m_holes << s;
				}
				for (const PanelDevice &d : layout.devices()) {
					const QPointF a = layout.toScene(d.rect_mm.topLeft());
					const QPointF b = layout.toScene(d.rect_mm.bottomRight());
					m_outlines << QRectF(a, b).normalized();
				}
				for (const QRectF &r : layout.keepouts()) {
					const QPointF a = layout.toScene(r.topLeft());
					const QPointF b = layout.toScene(r.bottomRight());
					m_bad << QRectF(a, b).normalized();
				}
			}
			update();
		}

		QRectF boundingRect() const override {return m_rect;}
			// never under the mouse: the folio's own items stay clickable
		QPainterPath shape() const override {return QPainterPath();}

		void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *) override
		{
			p->setRenderHint(QPainter::Antialiasing, true);
			p->setPen(QPen(QColor(200, 0, 0, 160), 0, Qt::DashLine));
			p->setBrush(QBrush(QColor(255, 0, 0, 60), Qt::BDiagPattern));
			for (const QRectF &r : m_bad) p->drawRect(r);

			p->setPen(QPen(QColor(0, 90, 200), 0, Qt::DashLine));
			p->setBrush(Qt::NoBrush);
			for (const QRectF &r : m_outlines) p->drawRect(r);

			for (const Hole &h : m_holes)
			{
				p->save();
				p->translate(h.centre);
				p->rotate(-h.angle);
				const QRectF r(-h.w / 2, -h.h / 2, h.w, h.h);
				p->setPen(QPen(QColor(220, 0, 0), 0));
				p->setBrush(QColor(220, 0, 0, 60));
				if (h.round) p->drawEllipse(r); else p->drawRect(r);
				if (h.thread) {
						// thread: a second, open circle like the drawing symbol
					p->setBrush(Qt::NoBrush);
					p->drawEllipse(r.adjusted(h.w * 0.15, h.h * 0.15, -h.w * 0.15, -h.h * 0.15));
				}
				const double c = qMax(h.w, h.h) * 0.8;
				p->drawLine(QPointF(-c, 0), QPointF(c, 0));
				p->drawLine(QPointF(0, -c), QPointF(0, c));
				p->restore();
			}
		}

	private:
		struct Hole {QPointF centre; double w = 0, h = 0, angle = 0; bool round = true, thread = false;};
		QPointer<Diagram> m_diagram;
		QList<Hole> m_holes;
		QList<QRectF> m_outlines;
		QList<QRectF> m_bad;
		QRectF m_rect;
};

/**
	@brief The definition of the 2D panel representation of a device: a
	rectangle of the device's real size at the drawing scale, with its
	designation, and the device's drilling patterns, sizes and STEP model.
	A thumbnail element: it stays out of the bill of materials, the
	schematic device is the one that is ordered.
*/
QDomElement panelDeviceDefinition(QDomDocument &doc, const QString &label, const QString &title,
								  double w_mm, double h_mm, double d_mm, double k,
								  const DiagramContext &source_info, const QDomElement &patterns,
								  const PanelOutline &graphic = PanelOutline())
{
	const int w = qMax(4, qRound(w_mm / k));
	const int h = qMax(4, qRound(h_mm / k));
	QDomElement definition = doc.createElement(QStringLiteral("definition"));
	definition.setAttribute(QStringLiteral("version"), QetVersion::currentVersion().toString());
	definition.setAttribute(QStringLiteral("type"), QStringLiteral("element"));
	definition.setAttribute(QStringLiteral("link_type"), QStringLiteral("thumbnail"));
	definition.setAttribute(QStringLiteral("width"), w);
	definition.setAttribute(QStringLiteral("height"), h);
	definition.setAttribute(QStringLiteral("hotspot_x"), 0);
	definition.setAttribute(QStringLiteral("hotspot_y"), 0);
	doc.appendChild(definition);

	QDomElement uuid = doc.createElement(QStringLiteral("uuid"));
	uuid.setAttribute(QStringLiteral("uuid"), QUuid::createUuid().toString());
	definition.appendChild(uuid);

	NamesList names;
	names.addName(QStringLiteral("en"), title);
	names.addName(QStringLiteral("de"), title);
	definition.appendChild(names.toXml(doc));

	DiagramContext infos;
	infos.addValue(QStringLiteral("label"), label);
	for (const QString &key : {QStringLiteral("manufacturer"), QStringLiteral("manufacturer_reference"),
							   QStringLiteral("designation"), QStringLiteral("description"),
							   QStringLiteral("step_file"), QStringLiteral("mount_variant"),
							   QStringLiteral("snap_height_mm"), QStringLiteral("middle_offset_mm"),
							   QStringLiteral("panel_of")})
		if (!source_info.value(key).toString().isEmpty())
			infos.addValue(key, source_info.value(key), false);
	infos.addValue(QStringLiteral("width_mm"), num(w_mm), false);
	infos.addValue(QStringLiteral("height_mm"), num(h_mm), false);
	infos.addValue(QStringLiteral("depth_mm"), num(d_mm), false);
	QDomElement informations = doc.createElement(QStringLiteral("elementInformations"));
	infos.toXml(informations, QStringLiteral("elementInformation"));
	definition.appendChild(informations);

	if (!patterns.isNull()) definition.appendChild(doc.importNode(patterns, true));

	QDomElement description = doc.createElement(QStringLiteral("description"));
	QDomElement rect = doc.createElement(QStringLiteral("rect"));
	rect.setAttribute(QStringLiteral("x"), 0);
	rect.setAttribute(QStringLiteral("y"), 0);
	rect.setAttribute(QStringLiteral("width"), w);
	rect.setAttribute(QStringLiteral("height"), h);
	rect.setAttribute(QStringLiteral("rx"), 0);
	rect.setAttribute(QStringLiteral("ry"), 0);
	rect.setAttribute(QStringLiteral("style"), graphic.isEmpty()
					  ? QStringLiteral("line-style:normal;line-weight:normal;filling:white;color:black")
					  : QStringLiteral("line-style:normal;line-weight:none;filling:white;color:white"));
	rect.setAttribute(QStringLiteral("antialias"), QStringLiteral("false"));
	description.appendChild(rect);

		// the manufacturer's panel layout drawing, fitted into the outline
	if (!graphic.isEmpty() && graphic.width > 0 && graphic.height > 0) {
		const double sx = w / graphic.width, sy = h / graphic.height;
		auto px = [&](const QPointF &p) {return QPointF(p.x() * sx, (graphic.height - p.y()) * sy);};
		const QString style = QStringLiteral("line-style:normal;line-weight:thin;filling:none;color:black");
		int count = 0;
		for (const QLineF &l : graphic.lines) {
			const QPointF a = px(l.p1()), b = px(l.p2());
				// what is too small to see at this scale is left out
			if (QLineF(a, b).length() < 0.3) continue;
			if (++count > 12000) break;
			QDomElement e = doc.createElement(QStringLiteral("line"));
			e.setAttribute(QStringLiteral("x1"), QString::number(a.x(), 'f', 2));
			e.setAttribute(QStringLiteral("y1"), QString::number(a.y(), 'f', 2));
			e.setAttribute(QStringLiteral("x2"), QString::number(b.x(), 'f', 2));
			e.setAttribute(QStringLiteral("y2"), QString::number(b.y(), 'f', 2));
			e.setAttribute(QStringLiteral("end1"), QStringLiteral("none"));
			e.setAttribute(QStringLiteral("end2"), QStringLiteral("none"));
			e.setAttribute(QStringLiteral("length1"), 1.5);
			e.setAttribute(QStringLiteral("length2"), 1.5);
			e.setAttribute(QStringLiteral("style"), style);
			e.setAttribute(QStringLiteral("antialias"), QStringLiteral("true"));
			description.appendChild(e);
		}
		for (const PanelOutline::Arc &a : graphic.arcs) {
			if (a.r * sx < 0.15) continue;
			if (++count > 12000) break;
			const QPointF c = px(a.centre);
			const double rx = a.r * sx, ry = a.r * sy;
			QDomElement e = doc.createElement(a.isCircle() ? QStringLiteral("ellipse") : QStringLiteral("arc"));
			e.setAttribute(QStringLiteral("x"), QString::number(c.x() - rx, 'f', 2));
			e.setAttribute(QStringLiteral("y"), QString::number(c.y() - ry, 'f', 2));
			e.setAttribute(QStringLiteral("width"), QString::number(2 * rx, 'f', 2));
			e.setAttribute(QStringLiteral("height"), QString::number(2 * ry, 'f', 2));
			if (!a.isCircle()) {
				double span = a.a1 - a.a0;
				while (span <= 0) span += 360;
				e.setAttribute(QStringLiteral("start"), QString::number(a.a0, 'f', 2));
				e.setAttribute(QStringLiteral("angle"), QString::number(span, 'f', 2));
			}
			e.setAttribute(QStringLiteral("style"), style);
			e.setAttribute(QStringLiteral("antialias"), QStringLiteral("true"));
			description.appendChild(e);
		}
	}

		// the designation, turned upright when the device is narrow like
		// a terminal or a circuit breaker
	const bool upright = graphic.isEmpty() && h > w * 1.5 && label.size() * 6 + 4 > w;
		// with the manufacturer's drawing the designation stands above the
		// device, like in EPLAN's mounting layout; over a narrow device it
		// stands upright so neighbours do not run into each other
	const bool above = !graphic.isEmpty();
	const bool above_up = above && label.size() * 4 > w;
	QDomElement text = doc.createElement(QStringLiteral("dynamic_text"));
	text.setAttribute(QStringLiteral("x"), upright ? w / 2 + 4 : (above_up ? w / 2 - 4 : (above ? 0 : 2)));
	text.setAttribute(QStringLiteral("y"), upright ? 2 : (above_up ? -2 : (above ? -13 : h / 2 - 6)));
	text.setAttribute(QStringLiteral("z"), 1);
	text.setAttribute(QStringLiteral("rotation"), upright ? 90 : (above_up ? 270 : 0));
	text.setAttribute(QStringLiteral("text_width"), -1);
	text.setAttribute(QStringLiteral("Halignment"), QStringLiteral("AlignLeft"));
	text.setAttribute(QStringLiteral("Valignment"), QStringLiteral("AlignTop"));
	text.setAttribute(QStringLiteral("frame"), QStringLiteral("false"));
	text.setAttribute(QStringLiteral("keep_visual_rotation"), QStringLiteral("false"));
	text.setAttribute(QStringLiteral("text_from"), QStringLiteral("ElementInfo"));
	text.setAttribute(QStringLiteral("uuid"), QUuid::createUuid().toString());
		// above a device the text is no larger than the device is wide
		// (upright) or than fits its width (across), at most 6, at least 3
	int font_size = qBound(5, qMin(w, h) / 4, 9);
	if (above_up) font_size = qBound(3, int(w * 0.9), 6);
	else if (above) font_size = qBound(3, int(w / qMax(1.0, label.size() * 0.6)), 6);
	text.setAttribute(QStringLiteral("font"), QETApp::dynamicTextsItemFont(font_size).toString());
	QDomElement cache = doc.createElement(QStringLiteral("text"));
	cache.appendChild(doc.createTextNode(label));
	text.appendChild(cache);
	QDomElement info_name = doc.createElement(QStringLiteral("info_name"));
	info_name.appendChild(doc.createTextNode(QStringLiteral("label")));
	text.appendChild(info_name);
	description.appendChild(text);
	definition.appendChild(description);
	return definition;
}

QString reportText(QETProject *project)
{
	QString out;
	QTextStream s(&out);
	int n_surfaces = 0;
	const QList<Diagram *> folios = project->folios();
	for (int f = 0; f < folios.size(); ++f)
	{
		for (QetShapeItem *surface : PanelLayout::surfaces(folios.at(f)))
		{
			++n_surfaces;
			const PanelLayout layout(surface);
			s << PanelMenu::tr("Folio %1 -- %2: %3 x %4 mm, %5 mm thick, scale 1:%6")
				 .arg(f + 1).arg(layout.name(), num(layout.widthMm()), num(layout.heightMm()),
					  num(layout.surfaceData().thickness), num(PanelMenu::scaleForMmPerUnit(layout.mmPerUnit()), 2))
			  << "\n";
			QMap<QString, int> kinds;
			for (const PanelHole &h : layout.holes()) kinds[h.kindName()]++;
			s << "  " << PanelMenu::tr("Holes: %1").arg(layout.holes().size());
			QStringList parts;
			for (auto it = kinds.constBegin(); it != kinds.constEnd(); ++it)
				parts << QStringLiteral("%1 x %2").arg(it.value()).arg(it.key());
			if (!parts.isEmpty()) s << "  (" << parts.join(QStringLiteral(", ")) << ")";
			s << "\n";
			for (const PanelDevice &d : layout.devices())
				s << "  " << d.designation << ": "
				  << (d.on_rail ? PanelMenu::tr("on DIN rail") : PanelMenu::tr("screwed"))
				  << (d.variant.isEmpty() ? QString() : QStringLiteral(" -- ") + d.variant)
				  << QStringLiteral("  x=%1 y=%2").arg(num(d.pos_mm.x()), num(d.pos_mm.y())) << "\n";
			const QStringList cuts = layout.cutList();
			if (!cuts.isEmpty()) {
				s << "  " << PanelMenu::tr("Cut list:") << "\n";
				for (const QString &c : cuts) s << "    " << c << "\n";
			}
			const QStringList warnings = layout.warnings();
			if (!warnings.isEmpty()) {
				s << "  " << PanelMenu::tr("Warnings:") << "\n";
				for (const QString &w : warnings) s << "    ! " << w << "\n";
			}
			s << "\n";
		}
	}
	if (!n_surfaces)
		s << PanelMenu::tr("The project has no mounting surface. Cabinet layout > Insert mounting surface.") << "\n";
	return out;
}

void showText(QWidget *parent, const QString &title, const QString &text)
{
	QDialog dialog(parent);
	dialog.setWindowTitle(title);
	dialog.resize(760, 560);
	auto *edit = new QPlainTextEdit(text, &dialog);
	edit->setReadOnly(true);
	edit->setFont(QFont(QStringLiteral("Consolas"), 9));
	auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
	QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
	auto *box = new QVBoxLayout(&dialog);
	box->addWidget(edit, 1);
	box->addWidget(buttons);
	dialog.exec();
}

} // namespace

/* ------------------------------------------------------------------ */

double PanelMenu::mmPerUnitForScale(double n)
{
	return n * kPaperMmPerUnit;
}

double PanelMenu::scaleForMmPerUnit(double mm_per_unit)
{
	return mm_per_unit / kPaperMmPerUnit;
}

PanelMenu::PanelMenu(QETDiagramEditor *editor) :
	QMenu(tr("Ca&binet layout"), editor),
	m_editor(editor)
{
		// a double click on a duct, rail or surface opens its own editor
		// a device dragged on a mounting surface: no grid, it snaps onto a
		// DIN rail near it and against its neighbour while it moves
	QetGraphicsItem::drag_filter = [](QetGraphicsItem *item, QPointF &pos) {
		auto *element = qobject_cast<Element *>(item);
		auto *d = element ? qobject_cast<Diagram *>(element->scene()) : nullptr;
		if (!d) return false;
		const QList<QetShapeItem *> plates = PanelLayout::surfaces(d);
		if (plates.isEmpty()) return false;
		const QPointF centre = element->sceneBoundingRect().center() + (pos - element->pos());
		bool on_plate = false;
		for (QetShapeItem *s : plates) if (sceneRectOf(s).contains(centre)) on_plate = true;
		if (!on_plate) return false;
			// Shift: free, no rail and no grid
		if (QGuiApplication::keyboardModifiers() & Qt::ShiftModifier) return true;
		const QPointF old = element->pos();
		element->QGraphicsItem::setPos(pos);
		const QPointF snap = PanelLayout::railSnap(d, element);
		element->QGraphicsItem::setPos(old);
		pos += snap;
		return true;
	};
	QetShapeItem::panel_editor = [self = QPointer<PanelMenu>(this)](QetShapeItem *shape) {
		if (!self || shape->panelData().role == PanelData::Enclosure) return false;
		self->editShape(shape);
		return true;
	};
		// the everyday commands on top, the single steps below in a submenu
	QAction *assistant = addAction(tr("Build cabinet..."));
	assistant->setStatusTip(tr("The cabinet layout in a few guided steps: enclosure, rails and ducts, devices, terminal strips, check, 3D, export"));
	connect(assistant, &QAction::triggered, this, &PanelMenu::showAssistant);
	m_check_action = addAction(tr("Layout check"));
	m_check_action->setCheckable(true);
	m_check_action->setStatusTip(tr("A side panel with every finding of the layout; a click shows the part"));
	connect(m_check_action, &QAction::toggled, this, &PanelMenu::showChecks);
	QAction *view3d = addAction(tr("3D view..."));
	view3d->setStatusTip(tr("The cabinet layout of this folio in 3D, with the STEP models of the parts"));
	connect(view3d, &QAction::triggered, this, &PanelMenu::show3D);
	QAction *exp = addAction(tr("Export manufacturing data (DXF)..."));
	exp->setStatusTip(tr("One DXF per mounting surface with every hole, a hole list and a cut list"));
	connect(exp, &QAction::triggered, this, &PanelMenu::exportManufacturing);
	addSeparator();
	QMenu *steps = addMenu(tr("Single steps"));
	auto add = [this, steps](const QString &text, const QString &tip, auto slot) {
		QAction *a = steps->addAction(text);
		a->setStatusTip(tip);
		connect(a, &QAction::triggered, this, slot);
		m_edit_actions << a;
		return a;
	};
	add(tr("Insert enclosure..."),
		tr("An enclosure of a manufacturer (e.g. Rittal from the EPLAN Data Portal) with its mounting plate"),
		&PanelMenu::insertEnclosure);
	add(tr("Insert mounting surface..."),
		tr("A mounting plate, door or wall of the given size, drawn to scale on this folio"),
		&PanelMenu::insertSurface);
	steps->addSeparator();
	add(tr("Insert DIN rail..."), tr("A DIN rail (TS 35) on the mounting surface, with its fixing holes"),
		[this]() {insertProfile(PanelData::DinRail);});
	add(tr("Insert cable duct..."), tr("A wiring duct on the mounting surface, with its fixing holes"),
		[this]() {insertProfile(PanelData::Duct);});
	add(tr("Insert C profile..."), tr("A C mounting profile on the mounting surface"),
		[this]() {insertProfile(PanelData::CProfile);});
	add(tr("Place devices from the schematic..."),
		tr("Put the devices of the project on the mounting surface at their real size, snapped onto the DIN rails"),
		&PanelMenu::placeDevices);
	add(tr("Take the layout from an EPLAN project..."),
		tr("Move the devices and terminal strips to where an EPLAN project (.zw1 archive or project folder) has them on its panel layout page"),
		&PanelMenu::importEplanLayout);
	add(tr("Assign part..."),
		tr("Give the selected devices or terminals their part (manufacturer and order number) from the EPLAN Data Portal"),
		&PanelMenu::assignArticle);
	add(tr("Devices without part..."),
		tr("The devices and terminals of the project that have no order number yet: the cabinet layout knows nothing of their size"),
		&PanelMenu::missingArticles);
	add(tr("Place terminal strips..."),
		tr("The terminal strips of the project as rows of terminals on the DIN rails, with the manufacturer's drawing of the terminal"),
		&PanelMenu::placeTerminalStrips);
	add(tr("Insert cut-out..."), tr("A hole or cut-out in the mounting surface (e.g. 22.5 mm for a push button)"),
		&PanelMenu::insertCutout);
	add(tr("Insert keep-out area..."), tr("An area of the mounting surface where no hole may be drilled"),
		&PanelMenu::insertKeepout);
	steps->addSeparator();
	add(tr("Manufacturing data of the selection..."),
		tr("Role and data of the selected shape, or mounting variant, size and STEP model of the selected device"),
		&PanelMenu::editSelected);
	steps->addSeparator();
	m_hole_view = steps->addAction(tr("Drilling view"));
	m_hole_view->setCheckable(true);
	m_hole_view->setStatusTip(tr("Show the holes and the device outlines over the cabinet layout"));
	connect(m_hole_view, &QAction::toggled, this, &PanelMenu::toggleHoleView);
	QAction *report = steps->addAction(tr("Manufacturing report..."));
	report->setStatusTip(tr("Holes, cut list and warnings of every mounting surface"));
	connect(report, &QAction::triggered, this, &PanelMenu::showReport);
	m_edit_actions << assistant;
	updateActions();
}

/**
	@brief PanelMenu::showAssistant
	"Build cabinet...": the guided steps, each one running the command of
	this menu that does it.
*/
void PanelMenu::showAssistant()
{
	Diagram *d = diagram();
	if (!d) return;
	PanelAssistant::Actions a;
	a.enclosure = [this]() {insertEnclosure();};
	a.devices = [this]() {placeDevices();};
	a.terminals = [this]() {placeTerminalStrips();};
	a.checks = [this]() {if (m_check_action) m_check_action->setChecked(true); showChecks(true);};
	a.view3d = [this]() {show3D();};
	a.manufacturing = [this]() {exportManufacturing();};
	auto *assistant = new PanelAssistant(d, a, m_editor);
	assistant->show();
}

/**
	@brief PanelMenu::showChecks
	The check list as a side panel of the editor, following the folio
	that is open.
*/
void PanelMenu::showChecks(bool on)
{
	if (!m_check_dock) {
		if (!on) return;
		m_check_dock = new PanelCheckDock(m_editor);
		m_editor->addDockWidget(Qt::RightDockWidgetArea, m_check_dock);
		connect(m_check_dock, &QDockWidget::visibilityChanged, this, [this](bool visible) {
			if (m_check_action && m_check_action->isChecked() != visible && !visible) m_check_action->setChecked(false);
		});
	}
	m_check_dock->setVisible(on);
	if (on) {
		m_check_dock->setDiagram(diagram());
		m_check_dock->raise();
	}
}

void PanelMenu::updateActions()
{
	Diagram *d = diagram();
	if (m_check_dock && m_check_dock->isVisible()) m_check_dock->setDiagram(d);
	const bool editable = d && d->project() && !d->project()->isReadOnly();
	for (QAction *a : std::as_const(m_edit_actions)) a->setEnabled(editable);
	if (m_hole_view && m_hole_view->isChecked()) refreshHoleView();
}

Diagram *PanelMenu::diagram() const
{
	if (!m_editor) return nullptr;
	DiagramView *view = m_editor->currentDiagramView();
	return view ? view->diagram() : nullptr;
}

/**
	The surface a new part goes on: the selected one, the one under a
	selected item, else the first surface of the folio.
*/
QetShapeItem *PanelMenu::targetSurface(Diagram *d)
{
	const QList<QetShapeItem *> surfaces = PanelLayout::surfaces(d);
	if (surfaces.isEmpty()) return nullptr;
	for (QGraphicsItem *item : d->selectedItems()) {
		if (auto *s = dynamic_cast<QetShapeItem *>(item))
			if (surfaces.contains(s)) return s;
		const QPointF c = item->sceneBoundingRect().center();
		for (QetShapeItem *s : surfaces)
			if (sceneRectOf(s).contains(c)) return s;
	}
	return surfaces.first();
}

QetShapeItem *PanelMenu::addShape(Diagram *d, const QRectF &r, bool ellipse,
								  const PanelData &data, const QString &undo_text)
{
	auto *shape = new QetShapeItem(r.topLeft(), r.bottomRight(),
								   ellipse ? QetShapeItem::Ellipse : QetShapeItem::Rectangle);
	QPen pen = shape->pen();
	if (data.role == PanelData::Keepout) pen.setStyle(Qt::DashDotLine);
	shape->setPen(pen);
		//colours of the EPLAN Pro Panel mounting layout: ducts teal, DIN
		//rails orange, the plate light grey
	if (data.role == PanelData::Surface)
		shape->setBrush(QColor(228, 228, 228));
	else if (data.role == PanelData::Duct)
		shape->setBrush(QColor(72, 160, 160));
	else if (data.role == PanelData::DinRail)
		shape->setBrush(QColor(240, 140, 50));
	else if (data.role == PanelData::CProfile)
		shape->setBrush(QColor(170, 170, 170));
	else if (data.role == PanelData::Keepout)
		shape->setBrush(QBrush(QColor(200, 0, 0), Qt::DiagCrossPattern));
	else if (data.role == PanelData::Cutout)
		shape->setBrush(QColor(255, 255, 255));
	shape->setPanelData(data);
	d->addItem(shape);
	auto *cmd = new QUndoCommand(undo_text);
	new AddGraphicsObjectCommand(shape, d, QPointF(), cmd);
	d->undoStack().push(cmd);
	d->clearSelection();
	shape->setSelected(true);
	return shape;
}

/* ------------------------------------------------------------------ */

/**
	@brief PanelMenu::insertEnclosure
	EPLAN Pro Panel "Insert > Enclosure": the enclosure of a manufacturer
	with its mounting plate. The enclosure comes from the EPLAN Data
	Portal (its panel layout views: front A, mounting plate B, ...); on
	the folio it is drawn as its front view at a scale that fits, the
	mounting plate inside it, ready for rails, ducts and devices. A
	bought plate: its holes are there already and keep drills away.
*/
void PanelMenu::insertEnclosure()
{
	Diagram *d = diagram();
	if (!d) return;
	QDialog dialog(m_editor);
	dialog.setWindowTitle(tr("Insert enclosure"));
	auto *form = new QFormLayout;
	auto *list = new QComboBox(&dialog);
	auto fill = [list](const QString &select) {
		list->clear();
		for (const QString &f : PanelImport::enclosureFolders()) list->addItem(QFileInfo(f).fileName(), f);
		const int i = list->findData(select);
		if (i >= 0) list->setCurrentIndex(i);
	};
	fill(QString());
	auto *portal = new QPushButton(tr("Data Portal…"), &dialog);
	portal->setToolTip(tr("Choose an enclosure (e.g. Rittal TS 8, VX 25, AX) in the EPLAN Data Portal"));
	auto *row = new QHBoxLayout;
	row->addWidget(list, 1);
	row->addWidget(portal);
	auto *info = new QLabel(&dialog);
	info->setWordWrap(true);
	auto *designation = new QLineEdit(QStringLiteral("+S1"), &dialog);
	auto *plate_name = new QLineEdit(QStringLiteral("-MP1"), &dialog);
	auto *with_plate = new QCheckBox(tr("With its mounting plate"), &dialog);
	with_plate->setChecked(true);
	auto *machined = new QCheckBox(tr("Make the holes of the mounting plate too (plate made in house)"), &dialog);
	auto *plate_w = mmBox(0, 5000, 1, &dialog);
	auto *plate_h = mmBox(0, 5000, 1, &dialog);
	auto *plate_size = new QHBoxLayout;
	plate_size->addWidget(plate_w);
	plate_size->addWidget(new QLabel(QStringLiteral("x"), &dialog));
	plate_size->addWidget(plate_h);
	form->addRow(tr("Enclosure"), row);
	form->addRow(QString(), info);
	form->addRow(tr("Designation"), designation);
	form->addRow(QString(), with_plate);
	form->addRow(tr("Mounting plate"), plate_name);
	form->addRow(tr("Plate size"), plate_size);
	form->addRow(QString(), machined);

	PanelImport::Enclosure enclosure;
	auto load = [&]() {
		enclosure = PanelImport::Enclosure();
		const QString folder = list->currentData().toString();
		if (folder.isEmpty()) {
			info->setText(tr("No enclosure yet: choose one in the Data Portal."));
		} else {
			enclosure = PanelImport::readEnclosure(folder);
			QString text = enclosure.description;
			text += QStringLiteral("\n") + tr("W x H x D: %1 x %2 x %3 mm").arg(num(enclosure.width), num(enclosure.height), num(enclosure.depth));
			if (!enclosure.plate.isEmpty()) {
				text += QStringLiteral("\n") + tr("Mounting plate %1 x %2 mm").arg(num(enclosure.plate.width), num(enclosure.plate.height));
				plate_w->setValue(enclosure.plate.width);
				plate_h->setValue(enclosure.plate.height);
			} else {
					//Rittal: the plate of a TS 8 / VX enclosure is 101 mm narrower
					//and 104 mm lower than the enclosure -- an estimate to check
				text += QStringLiteral("\n") + tr("The data hold no mounting plate: its size is estimated "
												  "(width - 101, height - 104 mm). Please check.");
				plate_w->setValue(qMax(0.0, enclosure.width - 101));
				plate_h->setValue(qMax(0.0, enclosure.height - 104));
			}
			info->setText(text);
		}
		plate_w->setEnabled(enclosure.plate.isEmpty());
		plate_h->setEnabled(enclosure.plate.isEmpty());
		machined->setEnabled(!enclosure.plate.isEmpty());
	};
	connect(list, &QComboBox::currentIndexChanged, &dialog, load);
	connect(portal, &QPushButton::clicked, &dialog, [&]() {
		PanelTemplateDialog dp(&dialog, true);
		if (dp.exec() == QDialog::Accepted && !dp.templateFile().isEmpty()) {
			fill(dp.templateFile());
			load();
		}
	});
	load();
	auto *box = new QVBoxLayout(&dialog);
	box->addLayout(form);
	box->addWidget(okCancel(&dialog));
	if (dialog.exec() != QDialog::Accepted || enclosure.isNull()) return;

		// a scale at which the front view fits the folio
	const QRectF area = d->border_and_titleblock.insideBorderRect().adjusted(20, 20, -20, -20);
	const QList<double> scales = {1.0, 2.0, 2.5, 5.0, 7.5, 10.0, 12.5, 15.0, 20.0, 25.0, 50.0};
	double n = scales.last();
	for (double cand : scales) {
		const double kk = mmPerUnitForScale(cand);
		if (enclosure.front.width / kk <= area.width() && enclosure.front.height / kk <= area.height()) {n = cand; break;}
	}
	const double k = mmPerUnitForScale(n);
	const QString type = enclosure.name.section(QStringLiteral(" - "), 0, 0);

	PanelData enc;
	enc.role = PanelData::Enclosure;
	enc.designation = designation->text().trimmed();
	enc.manufacturer = QStringLiteral("Rittal");
	enc.article = type.contains(QLatin1Char('.')) ? type.section(QLatin1Char('.'), 1) : type;
	enc.mm_per_unit = k;
	enc.profile_depth = enclosure.depth;
	enc.outline = enclosure.front;
	const QRectF enc_rect(area.topLeft(), QSizeF(enclosure.front.width / k, enclosure.front.height / k));
	QetShapeItem *enc_shape = addShape(d, enc_rect, false, enc, tr("insert enclosure %1").arg(enc.designation));
	enc_shape->setBrush(Qt::NoBrush);

	if (with_plate->isChecked() && plate_w->value() > 0 && plate_h->value() > 0) {
		PanelData plate = PanelData::defaults(PanelData::Surface);
		plate.designation = plate_name->text().trimmed();
		plate.manufacturer = enc.manufacturer;
		plate.mm_per_unit = k;
		if (!enclosure.plate.isEmpty()) {
			plate.outline = enclosure.plate;
			plate.outline_machined = machined->isChecked();
		}
			// in the middle of the enclosure's front
		const QSizeF size(plate_w->value() / k, plate_h->value() / k);
		const QRectF r(enc_rect.center() - QPointF(size.width() / 2, size.height() / 2), size);
		addShape(d, r, false, plate, tr("insert mounting surface %1").arg(plate.designation));
	}
	refreshHoleView();
}

void PanelMenu::insertSurface()
{
	Diagram *d = diagram();
	if (!d) return;
	QDialog dialog(m_editor);
	dialog.setWindowTitle(tr("Insert mounting surface"));
	auto *form = new QFormLayout;

		// what the plate is made from: sizes only, sizes with the corners
		// of a template (holes, radii, notches), or the template as it is
	auto *basis = new QComboBox(&dialog);
	basis->addItem(tr("Sizes only (plain plate)"));
	basis->addItem(tr("Sizes, holes and edges like the template"));
	basis->addItem(tr("Template plate as it is"));
	auto *templ = new QComboBox(&dialog);
	auto fill_templates = [templ](const QString &select) {
		templ->clear();
		for (const QString &f : PanelImport::templateFiles()) templ->addItem(PanelImport::templateName(f), f);
		const int i = templ->findData(select);
		if (i >= 0) templ->setCurrentIndex(i);
	};
	fill_templates(QString());
	auto *from_portal = new QPushButton(tr("Data Portal…"), &dialog);
	from_portal->setToolTip(tr("Load a manufacturer's mounting plate (e.g. Rittal) as template"));
	auto *from_file = new QPushButton(tr("DXF file…"), &dialog);
	from_file->setToolTip(tr("Use the DXF of a plate as template"));
	auto *templ_row = new QHBoxLayout;
	templ_row->addWidget(templ, 1);
	templ_row->addWidget(from_portal);
	templ_row->addWidget(from_file);
	auto *templ_info = new QLabel(&dialog);
	auto *machined = new QCheckBox(tr("The plate is made in house: make the holes and edges of the template too"), &dialog);
	machined->setChecked(true);
	machined->setToolTip(tr("Off: the plate is bought with its holes; they become keep-out areas"));

	auto *designation = new QLineEdit(QStringLiteral("-MP1"), &dialog);
	auto *w = mmBox(500, 5000, 1, &dialog);
	auto *h = mmBox(700, 5000, 1, &dialog);
	auto *t = mmBox(3, 50, 1, &dialog);
	auto *scale = new QComboBox(&dialog);
	scale->addItem(tr("Fit to the folio"), 0.0);
	const QList<double> scales = {1.0, 2.0, 2.5, 5.0, 7.5, 10.0, 12.5, 15.0, 20.0, 25.0, 50.0};
	for (double n : scales) scale->addItem(QStringLiteral("1:%1").arg(num(n)), n);
	scale->setToolTip(tr("Nominal scale when the folio is printed on A4"));
	auto *origin = new QComboBox(&dialog);
	origin->addItems({tr("bottom left"), tr("top left"), tr("top right"), tr("bottom right")});
	auto *article = new QLineEdit(&dialog);
	auto *manufacturer = new QLineEdit(&dialog);
	auto *step = new QLineEdit(&dialog);
	form->addRow(tr("Made from"), basis);
	form->addRow(tr("Template"), templ_row);
	form->addRow(QString(), templ_info);
	form->addRow(QString(), machined);
	form->addRow(tr("Designation"), designation);
	form->addRow(tr("Width"), w);
	form->addRow(tr("Height"), h);
	form->addRow(tr("Sheet thickness"), t);
	form->addRow(tr("Scale"), scale);
	form->addRow(tr("Zero point"), origin);
	form->addRow(tr("Manufacturer"), manufacturer);
	form->addRow(tr("Article number"), article);
	form->addRow(tr("STEP model"), stepPicker(step, &dialog));

	PanelOutline outline;
	auto load_template = [&]() {
		outline = PanelOutline();
		const QString file = templ->currentData().toString();
		if (!file.isEmpty()) {
			QString error;
			outline = PanelImport::readDxfFile(file, &error);
			if (outline.isEmpty()) templ_info->setText(error);
			else templ_info->setText(tr("%1 x %2 mm, %n holes", nullptr, int(outline.circles().size()))
									 .arg(num(outline.width), num(outline.height)));
		} else {
			templ_info->setText(tr("No template yet: load one from the Data Portal or a DXF file."));
		}
		const bool use = basis->currentIndex() > 0;
		templ->setEnabled(use);
		from_portal->setEnabled(use);
		from_file->setEnabled(use);
		machined->setEnabled(use);
		templ_info->setEnabled(use);
		const bool as_is = basis->currentIndex() == 2 && !outline.isEmpty();
		w->setEnabled(!as_is);
		h->setEnabled(!as_is);
		if (use && !outline.isEmpty() && (as_is || (w->value() == 500 && h->value() == 700))) {
			w->setValue(outline.width);
			h->setValue(outline.height);
		}
			// a Data Portal template is named "<type> - <description>"
		if (use && !outline.isEmpty() && article->text().isEmpty()) {
			const QString type = outline.name.section(QStringLiteral(" - "), 0, 0);
			if (type.contains(QLatin1Char('.'))) article->setText(type.section(QLatin1Char('.'), 1));
		}
	};
	connect(basis, &QComboBox::currentIndexChanged, &dialog, load_template);
	connect(templ, &QComboBox::currentIndexChanged, &dialog, load_template);
	connect(from_portal, &QPushButton::clicked, &dialog, [&]() {
		PanelTemplateDialog portal(&dialog);
		if (portal.exec() == QDialog::Accepted && !portal.templateFile().isEmpty()) {
			fill_templates(portal.templateFile());
			if (manufacturer->text().isEmpty()) manufacturer->setText(QStringLiteral("Rittal"));
			load_template();
		}
	});
	connect(from_file, &QPushButton::clicked, &dialog, [&]() {
		const QString file = QFileDialog::getOpenFileName(&dialog, tr("DXF of a mounting plate"), QString(),
														  tr("DXF files (*.dxf *.DXF)"));
		if (file.isEmpty()) return;
		QFile f(file);
		if (!f.open(QIODevice::ReadOnly)) return;
		QString error;
		const QString stored = PanelImport::saveTemplate(QFileInfo(file).completeBaseName(), f.readAll(), &error);
		if (stored.isEmpty()) {templ_info->setText(error); return;}
		fill_templates(stored);
		load_template();
	});
	load_template();

	auto *layout = new QVBoxLayout(&dialog);
	layout->addLayout(form);
	layout->addWidget(okCancel(&dialog));
	if (dialog.exec() != QDialog::Accepted) return;

	const QRectF area = d->border_and_titleblock.insideBorderRect().adjusted(20, 20, -20, -20);
	double n = scale->currentData().toDouble();
	if (n <= 0) {
			// the smallest standard scale at which the surface fits
		n = scales.last();
		for (double c : scales) {
			const double k = mmPerUnitForScale(c);
			if (w->value() / k <= area.width() && h->value() / k <= area.height()) {n = c; break;}
		}
	}
	PanelData data = PanelData::defaults(PanelData::Surface);
	data.designation = designation->text().trimmed();
	data.mm_per_unit = mmPerUnitForScale(n);
	data.thickness = t->value();
	data.origin = origin->currentIndex();
	data.article = article->text().trimmed();
	data.manufacturer = manufacturer->text().trimmed();
	data.step_file = step->text().trimmed();
	if (basis->currentIndex() > 0 && !outline.isEmpty()) {
		data.outline = outline;
		data.outline_machined = machined->isChecked();
	}
	const QSizeF size(w->value() / data.mm_per_unit, h->value() / data.mm_per_unit);
	addShape(d, QRectF(area.topLeft(), size), false, data,
			 tr("insert mounting surface %1").arg(data.designation));
	refreshHoleView();
}

void PanelMenu::insertProfile(PanelData::Role role)
{
	Diagram *d = diagram();
	if (!d) return;
	QetShapeItem *surface = targetSurface(d);
	if (!surface) {
		QMessageBox::information(m_editor, QString(title()).remove(QLatin1Char('&')), tr("Insert a mounting surface on this folio first."));
		return;
	}
	const PanelLayout layout(surface);
	PanelData data = PanelData::defaults(role);
	const QString what = role == PanelData::DinRail ? tr("DIN rail")
					   : role == PanelData::Duct ? tr("cable duct") : tr("C profile");

	QDialog dialog(m_editor);
	dialog.setWindowTitle(tr("Insert %1").arg(what));
	auto *form = new QFormLayout;
	auto *designation = new QLineEdit(&dialog);
	auto *orientation = new QComboBox(&dialog);
	orientation->addItems({tr("horizontal"), tr("vertical")});
	auto *length = mmBox(qMax(10.0, layout.widthMm() - (role == PanelData::Duct ? 20 : 60)), 5000, 1, &dialog);
	auto *width = mmBox(role == PanelData::DinRail ? 35 : role == PanelData::Duct ? 60 : 30, 500, 1, &dialog);
	auto *depth = mmBox(data.profile_depth, 500, 1, &dialog);
	auto *pos = mmBox(role == PanelData::Duct ? layout.heightMm() - 40 : layout.heightMm() / 2, 5000, 1, &dialog);
	pos->setToolTip(tr("Distance of the centre line from the zero edge of the surface"));
	auto *fixing = new QComboBox(&dialog);
	fixing->addItem(tr("Tapped hole"), int(PanelHole::Thread));
	fixing->addItem(tr("Through hole"), int(PanelHole::Drill));
	fixing->setCurrentIndex(data.fixing.type == PanelHole::Drill ? 1 : 0);
	auto *size = mmBox(data.fixing.d1, 30, 1, &dialog);
	size->setToolTip(tr("Thread size (M5: 5) or drill diameter"));
	auto *spacing = mmBox(data.spacing, 2000, 1, &dialog);
	auto *end = mmBox(data.end_distance, 500, 1, &dialog);
	auto *rows = new QSpinBox(&dialog);
	rows->setRange(1, 2);
	rows->setValue(data.rows);
	auto *article = new QLineEdit(&dialog);
	auto *step = new QLineEdit(&dialog);
	form->addRow(tr("Designation"), designation);
	form->addRow(tr("Orientation"), orientation);
	form->addRow(tr("Length"), length);
	form->addRow(tr("Width"), width);
	form->addRow(tr("Height above the surface"), depth);
	form->addRow(tr("Position of the centre line"), pos);
	form->addRow(tr("Fixing"), fixing);
	form->addRow(tr("Fixing size"), size);
	form->addRow(tr("Hole spacing"), spacing);
	form->addRow(tr("End distance"), end);
	form->addRow(tr("Rows of holes"), rows);
	form->addRow(tr("Article number"), article);
	form->addRow(tr("STEP model"), stepPicker(step, &dialog));
	const double sw = layout.widthMm(), sh = layout.heightMm();
	connect(orientation, &QComboBox::currentIndexChanged, &dialog, [=](int i) {
		length->setValue(qMax(10.0, (i == 0 ? sw : sh) - 60));
		pos->setValue(i == 0 ? sh / 2 : sw / 2);
	});
	auto *box = new QVBoxLayout(&dialog);
	box->addLayout(form);
	box->addWidget(okCancel(&dialog));
	if (dialog.exec() != QDialog::Accepted) return;

	data.designation = designation->text().trimmed();
	data.profile_depth = depth->value();
	data.fixing.type = PanelHole::Type(fixing->currentData().toInt());
	data.fixing.d1 = size->value();
	data.spacing = spacing->value();
	data.end_distance = end->value();
	data.rows = rows->value();
	data.article = article->text().trimmed();
	data.step_file = step->text().trimmed();

	const bool horizontal = orientation->currentIndex() == 0;
	const double L = length->value(), W = width->value();
		// centred along the surface, centre line at the given distance
	QPointF a_mm, b_mm;
	if (horizontal) {
		const double x0 = (sw - L) / 2;
		a_mm = QPointF(x0, pos->value() - W / 2);
		b_mm = QPointF(x0 + L, pos->value() + W / 2);
	} else {
		const double y0 = (sh - L) / 2;
		a_mm = QPointF(pos->value() - W / 2, y0);
		b_mm = QPointF(pos->value() + W / 2, y0 + L);
	}
	const QRectF r = QRectF(layout.toScene(a_mm), layout.toScene(b_mm)).normalized();
	addShape(d, r, false, data, tr("insert %1").arg(what));
	refreshHoleView();
}

void PanelMenu::insertCutout()
{
	Diagram *d = diagram();
	if (!d) return;
	QetShapeItem *surface = targetSurface(d);
	if (!surface) {
		QMessageBox::information(m_editor, QString(title()).remove(QLatin1Char('&')), tr("Insert a mounting surface on this folio first."));
		return;
	}
	const PanelLayout layout(surface);
	QDialog dialog(m_editor);
	dialog.setWindowTitle(tr("Insert cut-out"));
	auto *form = new QFormLayout;
	auto *type = new QComboBox(&dialog);
	type->addItem(tr("Round hole"), int(PanelHole::Drill));
	type->addItem(tr("Tapped hole"), int(PanelHole::Thread));
	type->addItem(tr("Rectangle"), int(PanelHole::Rectangle));
	type->addItem(tr("Slot"), int(PanelHole::Slot));
	type->addItem(tr("Hexagon"), int(PanelHole::Hexagon));
	type->addItem(tr("Octagon"), int(PanelHole::Octagon));
	auto *d1 = mmBox(22.5, 2000, 2, &dialog);
	auto *d2 = mmBox(0, 2000, 2, &dialog);
	d2->setToolTip(tr("0: as wide as high"));
	auto *d3 = mmBox(0, 200, 2, &dialog);
	auto *x = mmBox(layout.widthMm() / 2, 5000, 1, &dialog);
	auto *y = mmBox(layout.heightMm() / 2, 5000, 1, &dialog);
	auto *designation = new QLineEdit(&dialog);
	form->addRow(tr("Kind"), type);
	form->addRow(tr("Diameter / width"), d1);
	form->addRow(tr("Height"), d2);
	form->addRow(tr("Corner radius"), d3);
	form->addRow(tr("Centre x"), x);
	form->addRow(tr("Centre y"), y);
	form->addRow(tr("Designation"), designation);
	auto *box = new QVBoxLayout(&dialog);
	box->addLayout(form);
	box->addWidget(okCancel(&dialog));
	if (dialog.exec() != QDialog::Accepted) return;

	PanelData data = PanelData::defaults(PanelData::Cutout);
	data.designation = designation->text().trimmed();
	data.cutout.type = PanelHole::Type(type->currentData().toInt());
	data.cutout.d1 = d1->value();
	data.cutout.d2 = d2->value() > 0 ? d2->value() : d1->value();
	data.cutout.d3 = d3->value();
	data.cutout.subtype = d3->value() > 0 ? PanelHole::Rounded : PanelHole::Plain;
	const QPointF c = layout.toScene(QPointF(x->value(), y->value()));
	const double k = layout.mmPerUnit();
	const QSizeF s(data.cutout.d1 / k, data.cutout.d2 / k);
	const bool round = data.cutout.type != PanelHole::Rectangle && data.cutout.type != PanelHole::Slot;
	addShape(d, QRectF(c - QPointF(s.width() / 2, s.height() / 2), s), round, data, tr("insert cut-out"));
	refreshHoleView();
}

void PanelMenu::insertKeepout()
{
	Diagram *d = diagram();
	if (!d) return;
	QetShapeItem *surface = targetSurface(d);
	if (!surface) {
		QMessageBox::information(m_editor, QString(title()).remove(QLatin1Char('&')), tr("Insert a mounting surface on this folio first."));
		return;
	}
	const PanelLayout layout(surface);
	const PanelData data = PanelData::defaults(PanelData::Keepout);
	const QPointF a = layout.toScene(QPointF(layout.widthMm() * 0.4, layout.heightMm() * 0.4));
	const QPointF b = layout.toScene(QPointF(layout.widthMm() * 0.6, layout.heightMm() * 0.6));
	addShape(d, QRectF(a, b).normalized(), false, data, tr("insert keep-out area"));
	refreshHoleView();
}

/* ------------------------------------------------------------------ */

namespace {

	/// the first place on a horizontal DIN rail of @p surface with room for
	/// a device of ws x hs scene units: after what is on the rail already,
	/// between the ducts above and below it
bool freeRailSpot(QetShapeItem *surface, double ws, double hs, QPointF *pos)
{
	const PanelLayout layout(surface);
	const double k = layout.mmPerUnit() > 0 ? layout.mmPerUnit() : 1;
	const QRectF plate = sceneRectOf(surface);
	QList<QRectF> rails;
	for (const PanelProfile &p : layout.profiles())
		if (p.data.role == PanelData::DinRail && p.shape) {
			const QRectF r = sceneRectOf(p.shape);
			if (r.width() >= r.height()) rails << r;
		}
	std::sort(rails.begin(), rails.end(), [](const QRectF &a, const QRectF &b) {return a.top() < b.top();});
	for (const QRectF &r : std::as_const(rails)) {
		double above = r.top() - plate.top(), below = plate.bottom() - r.bottom();
		for (const PanelProfile &o : layout.profiles()) {
			if (!o.shape) continue;
			const QRectF q = sceneRectOf(o.shape);
			if (q == r || q.right() <= r.left() || q.left() >= r.right()) continue;
			if (q.bottom() <= r.top()) above = qMin(above, r.top() - q.bottom());
			else if (q.top() >= r.bottom()) below = qMin(below, q.top() - r.bottom());
		}
		if (hs > r.height() + 2 * qMin(above, below)) continue;
		double cursor = r.left() + 5 / k;
		for (const PanelDevice &dev : layout.devices()) {
			const QRectF dr = QRectF(layout.toScene(dev.rect_mm.topLeft()), layout.toScene(dev.rect_mm.bottomRight())).normalized();
			if (dr.intersects(r)) cursor = qMax(cursor, dr.right());
		}
		if (cursor + ws <= r.right() - 2 / k) {
			*pos = QPointF(cursor, r.center().y() - hs / 2);
			return true;
		}
	}
	return false;
}

} // namespace

QString PanelMenu::placeTerminalStrip(Diagram *d, const QString &designation, int count,
									  const QString &order, const QString &manufacturer,
									  double width_mm, double height_mm)
{
	Q_UNUSED(width_mm) Q_UNUSED(height_mm)
	QList<QPair<QString, QString>> terminals;
	for (int i = 0; i < count; ++i) terminals << qMakePair(order, manufacturer);
	return placeTerminalStrip(d, designation, terminals);
}

QString PanelMenu::placeTerminalStrip(Diagram *d, const QString &designation,
									  const QList<QPair<QString, QString>> &terminals)
{
	const int count = int(terminals.size());
	if (!d || !d->project() || count <= 0) return QString();
	QList<QetShapeItem *> surfaces = PanelLayout::surfaces(d);
	if (surfaces.isEmpty()) return tr("%1: no mounting surface on the folio").arg(designation);
	QetShapeItem *surface = surfaces.first();
	const PanelLayout layout(surface);
	const double k = layout.mmPerUnit() > 0 ? layout.mmPerUnit() : 1;

		// each terminal: the manufacturer's drawing of its part, else a
		// plain one 5.2 x 60 mm; in a row only the lines that carry the
		// shape, the small details run together into a black bar
	QHash<QString, PanelOutline> views;
	auto viewOf = [&views](const QString &order, const QString &maker) {
		const QString key = order + QLatin1Char('|') + maker;
		if (!views.contains(key)) {
			PanelOutline one;
			if (!order.isEmpty()) one = PanelImport::fetchDeviceView(order, maker);
			if (one.isEmpty() || one.width <= 0 || one.height <= 0 || one.width >= 40) {
				one = PanelOutline();
				one.width = 5.2;
				one.height = 60;
				one.lines << QLineF(0, 0, 5.2, 0) << QLineF(5.2, 0, 5.2, 60) << QLineF(5.2, 60, 0, 60) << QLineF(0, 60, 0, 0)
						  << QLineF(0, 15, 5.2, 15) << QLineF(0, 45, 5.2, 45);
			}
			views.insert(key, one);
		}
		return views.value(key);
	};
	PanelOutline strip;
	QStringList kinds;
	double x = 0, th = 0;
	for (const auto &t : terminals) th = qMax(th, viewOf(t.first, t.second).height);
	for (const auto &t : terminals) {
		const PanelOutline one = viewOf(t.first, t.second);
		const double detail = qMin(2.0, one.height * 0.04);
			// terminals of different heights stand on one rail centre line
		const QPointF shift(x, (th - one.height) / 2);
		for (const QLineF &l : one.lines)
			if (l.length() >= detail) strip.lines << l.translated(shift);
		for (PanelOutline::Arc a : one.arcs)
			if (a.r >= 0.8) {a.centre += shift; strip.arcs << a;}
		x += one.width;
		if (!kinds.contains(t.first)) kinds << t.first;
	}
	strip.width = x;
	strip.height = th;
	const QString order = kinds.size() == 1 ? kinds.first() : QString();
	const QString manufacturer = kinds.size() == 1 ? terminals.first().second : QString();

	QPointF pos;
	const double ws = strip.width / k, hs = strip.height / k;
	if (!freeRailSpot(surface, ws, hs, &pos))
		return tr("%1: no DIN rail with %2 mm free").arg(designation, num(strip.width));

	XmlElementCollection *collection = d->project()->embeddedElementCollection();
	if (!collection) return QString();
	const QString dir_name = QStringLiteral("Cabinet layout");
	const QString dir_path = QStringLiteral("import/") + dir_name;
	if (!collection->exist(dir_path)) {
		NamesList names;
		names.addName(QStringLiteral("en"), dir_name);
		names.addName(QStringLiteral("de"), QStringLiteral("Schaltschrankaufbau"));
		if (!collection->createDir(QStringLiteral("import"), dir_name, names)) return QString();
	}
	DiagramContext info;
	info.addValue(QStringLiteral("manufacturer_reference"), order, false);
	info.addValue(QStringLiteral("manufacturer"), manufacturer, false);
	info.addValue(QStringLiteral("panel_of"), QStringLiteral("strip:") + designation, false);
	info.addValue(QStringLiteral("terminal_count"), QString::number(count), false);
	QDomDocument doc;
	const QDomElement def = panelDeviceDefinition(doc, designation,
												  tr("Terminal strip %1 (%2 terminals)").arg(designation).arg(count),
												  strip.width, strip.height, 0, k, info, QDomElement(), strip);
	const QString file_name = QET::stringToFileName(designation) + QLatin1Char('-')
							  + QUuid::createUuid().toString(QUuid::Id128).left(8) + QStringLiteral(".elmt");
	if (!collection->addElementDefinition(dir_path, file_name, def)) return QString();
	const ElementsLocation location(dir_path + QLatin1Char('/') + file_name, d->project());
	int state = 0;
	Element *element = ElementFactory::Instance()->createElement(location, nullptr, &state);
	if (state || !element) {delete element; return QString();}
	element->setPos(pos);
	d->addItem(element);
	auto *cmd = new QUndoCommand(tr("place terminal strip %1").arg(designation));
	new AddGraphicsObjectCommand(element, d, pos, cmd);
	d->undoStack().push(cmd);
	d->project()->setModified(true);
	return tr("%1: %2 terminals, %3 mm long, %4 kinds of terminal").arg(designation).arg(count).arg(num(strip.width)).arg(kinds.size());
}

Element *PanelMenu::replaceDevice(Element *device, const DiagramContext &info)
{
	auto *d = device ? qobject_cast<Diagram *>(device->scene()) : nullptr;
	if (!d || !d->project()) return nullptr;
	const double w = infoNumber(info, QStringLiteral("width_mm")), h = infoNumber(info, QStringLiteral("height_mm"));
	if (w <= 0 || h <= 0) return nullptr;
		// the scale of the surface the device lies on
	double k = 0;
	const QPointF at = device->sceneBoundingRect().center();
	for (QetShapeItem *s : PanelLayout::surfaces(d))
		if (sceneRectOf(s).contains(at)) {k = s->panelData().mm_per_unit; break;}
	if (k <= 0 && !PanelLayout::surfaces(d).isEmpty()) k = PanelLayout::surfaces(d).first()->panelData().mm_per_unit;
	if (k <= 0) return nullptr;

	PanelOutline graphic;
	const QString order = info.value(QStringLiteral("manufacturer_reference")).toString();
	if (!order.isEmpty() && !info.value(QStringLiteral("panel_of")).toString().startsWith(QStringLiteral("strip:")))
		graphic = PanelImport::fetchDeviceView(order, info.value(QStringLiteral("manufacturer")).toString());

	XmlElementCollection *collection = d->project()->embeddedElementCollection();
	if (!collection) return nullptr;
	const QString dir_path = QStringLiteral("import/Cabinet layout");
	if (!collection->exist(dir_path)) {
		NamesList names;
		names.addName(QStringLiteral("en"), QStringLiteral("Cabinet layout"));
		names.addName(QStringLiteral("de"), QStringLiteral("Schaltschrankaufbau"));
		if (!collection->createDir(QStringLiteral("import"), QStringLiteral("Cabinet layout"), names)) return nullptr;
	}
	QString label = info.value(QStringLiteral("label")).toString();
	if (label.isEmpty()) label = device->actualLabel();
	const QDomElement patterns = device->location().xml().firstChildElement(QStringLiteral("drillingpatterns"));
	QDomDocument doc;
	const QDomElement def = panelDeviceDefinition(doc, label, device->name(), w, h,
												  infoNumber(info, QStringLiteral("depth_mm")), k, info, patterns, graphic);
	const QString file_name = QET::stringToFileName(label) + QLatin1Char('-')
							  + QUuid::createUuid().toString(QUuid::Id128).left(8) + QStringLiteral(".elmt");
	if (!collection->addElementDefinition(dir_path, file_name, def)) return nullptr;
	int state = 0;
	Element *element = ElementFactory::Instance()->createElement(
		ElementsLocation(dir_path + QLatin1Char('/') + file_name, d->project()), nullptr, &state);
	if (state || !element) {delete element; return nullptr;}
	const QPointF pos = device->pos();
	element->setPos(pos);
	d->addItem(element);
	auto *macro = new QUndoCommand(tr("change the size of %1").arg(label));
	new AddGraphicsObjectCommand(element, d, pos, macro);
	DiagramContent content;
	content.m_elements << device;
	new DeleteQGraphicsItemCommand(d, content, macro);
	d->undoStack().push(macro);
	d->project()->setModified(true);
	return element;
}

QString PanelMenu::applyEplanLayout(Diagram *d, const QString &eplan_project)
{
	if (!d) return QString();
	QetShapeItem *surface = targetSurface(d);
	if (!surface) return tr("Insert a mounting surface on this folio first.");
	const EplanProject::Layout layout = EplanProject::readLayout(eplan_project);
	if (!layout.error.isEmpty()) return layout.error;
	if (layout.items.isEmpty()) return tr("The panel layout page of the EPLAN project holds no devices.");
	const double k = surface->panelData().mm_per_unit > 0 ? surface->panelData().mm_per_unit : 1;
	const QRectF plate = sceneRectOf(surface);

	static const QRegularExpression key_re(QStringLiteral("^=([^+]*)\\+([^-]*)(-.*)$"));
	auto same = [](const QString &a, const QString &b) {return a.isEmpty() || b.isEmpty() || a.compare(b, Qt::CaseInsensitive) == 0;};
	auto *macro = new QUndoCommand(tr("take the layout from %1").arg(QFileInfo(eplan_project).fileName()));
	int moved = 0;
	QStringList off_plate;
	double park_y = plate.top();
	for (Element *e : d->elements()) {
		const DiagramContext info = e->elementInformations();
		QString key = info.value(QStringLiteral("panel_of")).toString();
		if (key.isEmpty()) continue;
		QString plant, location, bmk;
		if (key.startsWith(QLatin1String("strip:"))) bmk = key.mid(6);
		else {
			const QRegularExpressionMatch m = key_re.match(key);
			if (!m.hasMatch()) continue;
			plant = m.captured(1); location = m.captured(2); bmk = m.captured(3);
		}
		const double w = infoNumber(info, QStringLiteral("width_mm")), h = infoNumber(info, QStringLiteral("height_mm"));
			// the placement of that device; of several (main part, auxiliary
			// switch) the one of its size
		const EplanProject::Placement *best = nullptr;
		double best_cost = 1e18;
		for (const EplanProject::Placement &p : layout.items) {
			if (p.bmk.compare(bmk, Qt::CaseInsensitive) != 0 || !same(p.location, location) || !same(p.plant, plant)) continue;
			const double cost = p.centre_only ? 1e6 : qAbs(p.rect_mm.width() - w) + qAbs(p.rect_mm.height() - h);
			if (cost < best_cost) {best_cost = cost; best = &p;}
		}
		QPointF target;
		bool on_plate = false;
			// a device drawn on the page as a plain box with its type
			// written in it, without a function there: the text is its centre
		EplanProject::Placement by_text;
		if (!best) {
			const QString type = info.value(QStringLiteral("manufacturer_reference")).toString().trimmed().toUpper();
			for (const auto &t : layout.texts)
				if (!type.isEmpty() && t.first == type) {
					by_text.centre_only = true;
					by_text.rect_mm = QRectF(t.second, QSizeF(0, 0));
					best = &by_text;
					break;
				}
		}
		if (best) {
			double x = best->rect_mm.left(), top = best->rect_mm.bottom();
			if (best->centre_only) {x -= w / 2; top += h / 2;}
			on_plate = x > -w && x < plate.width() * k && top > 0 && top - h < plate.height() * k + h;
			target = QPointF(plate.left() + x / k, plate.bottom() - top / k);
		}
		if (!on_plate) {
				// not on the plate in EPLAN (door, roof, floor): beside the plate
			if (!plate.intersects(e->sceneBoundingRect())) continue;
			target = QPointF(plate.right() + 120 / k, park_y);
			park_y += qMax(h / k, 20.0) + 10;
			off_plate << key;
		}
		if (QLineF(e->pos(), target).length() < 0.01) continue;
		new QPropertyUndoCommand(e, "pos", e->pos(), target, macro);
		++moved;
	}
	if (moved) d->undoStack().push(macro);
	else delete macro;
	QString report = tr("%1 devices and terminal strips moved to their place in the EPLAN project.").arg(moved - off_plate.size());
	if (!off_plate.isEmpty())
		report += QLatin1Char('\n') + tr("Not on the plate in the EPLAN project, put beside it: %1").arg(off_plate.join(QStringLiteral(", ")));
	return report;
}

void PanelMenu::importEplanLayout()
{
	Diagram *d = diagram();
	if (!d) return;
	const QString path = QFileDialog::getOpenFileName(m_editor, tr("EPLAN project"), QString(),
		tr("EPLAN project (*.zw1 Function.eod);;All files (*)"));
	if (path.isEmpty()) return;
	QGuiApplication::setOverrideCursor(Qt::WaitCursor);
	const QString report = applyEplanLayout(d, path);
	QGuiApplication::restoreOverrideCursor();
	refreshHoleView();
	QMessageBox::information(m_editor, tr("Cabinet layout"), report);
}

Element *PanelMenu::partHolder(Element *e)
{
	if (e && e->linkType() == Element::Slave && !e->linkedElements().isEmpty())
		return e->linkedElements().first();
	return e;
}

/**
	@brief PanelMenu::assignArticle
	EPLAN's part selection for the selected devices (or terminals): one
	part from the Data Portal, its manufacturer and order number written
	into the device. A contact gives it to its coil (the main function).
*/
void PanelMenu::assignArticle()
{
	Diagram *d = diagram();
	if (!d) return;
	QList<Element *> targets;
	for (QGraphicsItem *item : d->selectedItems())
		if (auto *e = dynamic_cast<Element *>(item))
			if (Element *h = partHolder(e))
				if (!targets.contains(h)) targets << h;
	if (targets.isEmpty()) {
		QMessageBox::information(m_editor, QString(title()).remove(QLatin1Char('&')),
								 tr("Select the devices or terminals in the schematic first."));
		return;
	}
	assignArticleTo(targets);
}

bool PanelMenu::assignArticleTo(const QList<Element *> &targets)
{
	if (targets.isEmpty()) return false;
	const DiagramContext first = targets.first()->elementInformations();
	QString search = first.value(QStringLiteral("manufacturer_reference")).toString();
	if (search.isEmpty()) search = first.value(QStringLiteral("designation")).toString();
	const QString what = targets.size() == 1 ? targets.first()->actualLabel()
											 : tr("%n devices", nullptr, int(targets.size()));
	ArticleDialog dialog(m_editor, what, search);
	if (dialog.exec() != QDialog::Accepted) return false;
	const DataPortalPart part = dialog.part();
	if (part.order_number.isEmpty()) return false;

	QHash<Diagram *, QUndoCommand *> macros;
	for (Element *e : targets) {
		auto *folio = qobject_cast<Diagram *>(e->scene());
		const DiagramContext old_info = e->elementInformations();
		DiagramContext new_info = old_info;
		new_info.addValue(QStringLiteral("manufacturer"), part.manufacturer, false);
		new_info.addValue(QStringLiteral("manufacturer_reference"), part.order_number, false);
		if (!part.designation.isEmpty())
			new_info.addValue(QStringLiteral("designation"), part.designation, false);
		new_info.addValue(QStringLiteral("eplan_part_number"), part.part_number, false);
			// sizes typed for the old part do not fit the new one
		if (old_info.value(QStringLiteral("manufacturer_reference")).toString() != part.order_number)
			for (const QString &key : {QStringLiteral("width_mm"), QStringLiteral("height_mm"), QStringLiteral("depth_mm")})
				new_info.addValue(key, QString(), false);
		if (new_info == old_info) continue;
		if (!folio) {e->setElementInformations(new_info); continue;}
		if (!macros.contains(folio)) macros.insert(folio, new QUndoCommand(tr("assign the part %1").arg(part.order_number)));
		new ChangeElementInformationCommand(e, old_info, new_info, macros.value(folio));
	}
	for (auto it = macros.constBegin(); it != macros.constEnd(); ++it) it.key()->undoStack().push(it.value());
	return true;
}

/**
	@brief PanelMenu::missingArticles
	What the cabinet layout cannot know: the devices and terminals of the
	project without an order number. A double click shows the device, the
	button gives the selected ones their part.
*/
void PanelMenu::missingArticles()
{
	Diagram *d = diagram();
	if (!d || !d->project()) return;
	QDialog dialog(m_editor);
	dialog.setWindowTitle(tr("Devices without part"));
	dialog.resize(760, 520);
	auto *table = new QTableWidget(0, 4, &dialog);
	table->setHorizontalHeaderLabels({tr("Device"), tr("Kind"), tr("Folio"), tr("Designation")});
	table->horizontalHeader()->setStretchLastSection(true);
	table->verticalHeader()->hide();
	table->setSelectionBehavior(QAbstractItemView::SelectRows);
	table->setEditTriggers(QAbstractItemView::NoEditTriggers);
	auto *status = new QLabel(&dialog);
	QList<QPointer<Element>> elements;
	auto fill = [&]() {
		table->setRowCount(0);
		elements.clear();
		int with = 0;
		QSet<Element *> seen;
		for (Diagram *folio : d->project()->folios()) {
			if (!PanelLayout::surfaces(folio).isEmpty()) continue;
			for (Element *e : folio->elements()) {
				const int t = e->linkType();
				if (!(t == Element::Simple || t == Element::Master || t == Element::Terminale)) continue;
				if (e->actualLabel().trimmed().isEmpty() || seen.contains(e)) continue;
				seen << e;
				if (!e->elementInformations().value(QStringLiteral("manufacturer_reference")).toString().trimmed().isEmpty()) {++with; continue;}
				const int r = table->rowCount();
				table->insertRow(r);
				table->setItem(r, 0, new QTableWidgetItem(e->actualLabel()));
				table->setItem(r, 1, new QTableWidgetItem(t == Element::Terminale ? tr("Terminal") : tr("Device")));
				table->setItem(r, 2, new QTableWidgetItem(QString::number(folio->folioIndex() + 1)));
				table->setItem(r, 3, new QTableWidgetItem(e->name()));
				elements << e;
			}
		}
		status->setText(tr("%1 without part, %2 with part.").arg(table->rowCount()).arg(with));
	};
	fill();
	connect(table, &QTableWidget::cellDoubleClicked, &dialog, [&](int row, int) {
		Element *e = elements.value(row);
		auto *folio = e ? qobject_cast<Diagram *>(e->scene()) : nullptr;
		if (!folio) return;
		emit folio->showDiagram(folio);
		folio->clearSelection();
		e->setSelected(true);
		for (QGraphicsView *v : folio->views()) v->centerOn(e);
	});
	auto *assign = new QPushButton(tr("Assign part to the selected..."), &dialog);
	connect(assign, &QPushButton::clicked, &dialog, [&]() {
		QList<Element *> chosen;
		for (const QModelIndex &i : table->selectionModel()->selectedRows())
			if (Element *e = elements.value(i.row())) chosen << e;
		if (assignArticleTo(chosen)) fill();
	});
	auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
	connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
	auto *box = new QVBoxLayout(&dialog);
	box->addWidget(table, 1);
	box->addWidget(status);
	box->addWidget(assign);
	box->addWidget(buttons);
	dialog.exec();
}

/**
	@brief PanelMenu::placeTerminalStrips
	The terminal strips (EPLAN: terminal strip definition and its terminals)
	go onto the DIN rails as rows of terminals. Offered: the terminal strips
	of the project; more rows can be added by hand.
*/
void PanelMenu::placeTerminalStrips()
{
	Diagram *d = diagram();
	if (!d || !d->project()) return;
	if (PanelLayout::surfaces(d).isEmpty()) {
		QMessageBox::information(m_editor, QString(title()).remove(QLatin1Char('&')),
								 tr("Insert a mounting surface on this folio first."));
		return;
	}
	QDialog dialog(m_editor);
	dialog.setWindowTitle(tr("Place terminal strips"));
	dialog.resize(640, 460);
	auto *table = new QTableWidget(0, 4, &dialog);
	table->setHorizontalHeaderLabels({tr("Terminal strip"), tr("Terminals"), tr("Parts of the terminals"), tr("Manufacturer")});
	table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
	table->horizontalHeader()->setStretchLastSection(false);
	table->horizontalHeader()->setStretchLastSection(true);
	table->verticalHeader()->hide();
	auto addRow = [table](const QString &name, int count, const QString &order, const QString &maker) {
		const int r = table->rowCount();
		table->insertRow(r);
		auto *c0 = new QTableWidgetItem(name);
		c0->setFlags(c0->flags() | Qt::ItemIsUserCheckable);
		c0->setCheckState(Qt::Checked);
		table->setItem(r, 0, c0);
		table->setItem(r, 1, new QTableWidgetItem(QString::number(count)));
		table->setItem(r, 2, new QTableWidgetItem(order));
		table->setItem(r, 3, new QTableWidgetItem(maker));
	};
		// the strips of the project: each terminal keeps the part it was
		// given (terminal catalog or "Assign part")
	QHash<int, QList<QPair<QString, QString>>> parts_of_row;
	for (TerminalStrip *strip : d->project()->terminalStrip())
		if (strip && strip->physicalTerminalCount() > 0) {
			QString name = strip->name();
			if (!name.startsWith(QLatin1Char('-'))) name.prepend(QLatin1Char('-'));
			QList<QPair<QString, QString>> parts;
			QMap<QString, int> kinds;
			for (const auto &phy : strip->physicalTerminal()) {
				QString order, maker;
				for (const auto &real : phy->realTerminals()) {
					const DiagramContext info = real->terminalData().m_informations;
					order = info.value(QStringLiteral("manufacturer_reference")).toString().trimmed();
					maker = info.value(QStringLiteral("manufacturer")).toString().trimmed();
					if (!order.isEmpty()) break;
				}
				parts << qMakePair(order, maker);
				kinds[order.isEmpty() ? tr("without part") : order] += 1;
			}
			QStringList summary;
			for (auto it = kinds.constBegin(); it != kinds.constEnd(); ++it)
				summary << QStringLiteral("%1 x %2").arg(it.value()).arg(it.key());
			parts_of_row.insert(table->rowCount(), parts);
			addRow(name, strip->physicalTerminalCount(), QString(), QString());
			const int last = table->rowCount() - 1;
			auto *info = new QTableWidgetItem(summary.join(QStringLiteral(", ")));
			info->setFlags(info->flags() & ~Qt::ItemIsEditable);
			table->setItem(last, 2, info);
			table->item(last, 1)->setFlags(table->item(last, 1)->flags() & ~Qt::ItemIsEditable);
		}
	auto *add_row = new QPushButton(tr("Add a strip"), &dialog);
	connect(add_row, &QPushButton::clicked, &dialog, [addRow]() {addRow(QStringLiteral("-X"), 10, QString(), QString());});
	auto *hint = new QLabel(tr("The terminals of the project are drawn with the part each one has (Assign part); "
							   "a terminal without part is drawn 5.2 x 60 mm. For a strip added by hand, type "
							   "the order number of its terminals."), &dialog);
	hint->setWordWrap(true);
	auto *box = new QVBoxLayout(&dialog);
	box->addWidget(table, 1);
	box->addWidget(add_row);
	box->addWidget(hint);
	auto *buttons = okCancel(&dialog);
	buttons->button(QDialogButtonBox::Ok)->setText(tr("Place"));
	box->addWidget(buttons);
	if (table->rowCount() == 0) addRow(QStringLiteral("-X1"), 10, QString(), QString());
	if (dialog.exec() != QDialog::Accepted) return;

	QStringList report;
	for (int r = 0; r < table->rowCount(); ++r) {
		if (table->item(r, 0)->checkState() != Qt::Checked) continue;
		const QString line = parts_of_row.contains(r)
			? placeTerminalStrip(d, table->item(r, 0)->text().trimmed(), parts_of_row.value(r))
			: placeTerminalStrip(d, table->item(r, 0)->text().trimmed(),
								 table->item(r, 1)->text().toInt(),
								 table->item(r, 2) ? table->item(r, 2)->text().trimmed() : QString(),
								 table->item(r, 3) ? table->item(r, 3)->text().trimmed() : QString());
		if (!line.isEmpty()) report << line;
	}
	refreshHoleView();
	if (!report.isEmpty())
		showText(m_editor, tr("Place terminal strips"), report.join(QLatin1Char('\n')));
}

/**
	@brief PanelMenu::placeDevices
	EPLAN "place from the device navigator": every device of the project
	(main functions of the schematic, not yet on a layout) can be put on
	the mounting surface as a rectangle of its real size, carrying its
	drilling patterns and STEP model. They are snapped onto the DIN rails
	from left to right; what does not fit goes into the free area.
*/
void PanelMenu::placeDevices()
{
	placeDevicesOn(diagram(), m_editor, true);
	refreshHoleView();
}

QString PanelMenu::placeDevicesOn(Diagram *d, QWidget *parent, bool interactive)
{
	QStringList report;
	auto say = [&](bool warning, const QString &text) {
		report << text;
		if (!interactive) return;
		if (warning) QMessageBox::warning(parent, tr("Cabinet layout"), text);
		else QMessageBox::information(parent, tr("Cabinet layout"), text);
	};
	if (!d || !d->project()) return report.join(QLatin1Char('\n'));
	QetShapeItem *surface = targetSurface(d);
	if (!surface) {
		say(false, tr("Insert a mounting surface on this folio first."));
		return report.join(QLatin1Char('\n'));
	}
	QETProject *project = d->project();

			// One device is one full designation =plant+location-BMK, however it
		// is written on its folios ("-A20" on a folio of =SB+CA, "=SB-A20",
		// "=SB+CA-A20"): the parts of it a label leaves out come from the
		// folio. Only the devices of the enclosure's location are offered.
	struct Tag {QString plant, location, bmk;};
	auto tagOf = [](Element *e, Diagram *folio) {
		Tag t;
		t.plant = folio->border_and_titleblock.plant().trimmed();
		t.location = folio->border_and_titleblock.locmach().trimmed();
		if (t.plant.startsWith(QLatin1Char('='))) t.plant.remove(0, 1);
		if (t.location.startsWith(QLatin1Char('+'))) t.location.remove(0, 1);
		static const QRegularExpression re(QStringLiteral("^(?:=([^+\\-]+))?(?:\\+([^\\-]+))?(-.*)?$"));
		const QString label = e->actualLabel().trimmed();
		const QRegularExpressionMatch m = re.match(label);
		if (!m.hasMatch()) {t.bmk = label; return t;}
		if (!m.captured(1).isEmpty()) t.plant = m.captured(1);
		if (!m.captured(2).isEmpty()) t.location = m.captured(2);
		t.bmk = m.captured(3);
		return t;
	};
	auto keyOf = [](const Tag &t) {
		return QStringLiteral("=%1+%2%3").arg(t.plant, t.location, t.bmk);
	};
		// the location of the enclosure around the plate
	QString enclosure_location;
	const QRectF plate_rect = sceneRectOf(surface);
	for (QGraphicsItem *item : d->items())
		if (auto *shape = dynamic_cast<QetShapeItem *>(item))
			if (shape->panelData().role == PanelData::Enclosure
				&& sceneRectOf(shape).contains(plate_rect.center())) {
				QString loc = shape->panelData().designation.trimmed();
				if (loc.contains(QLatin1Char('+'))) loc = loc.section(QLatin1Char('+'), 1).section(QLatin1Char('-'), 0, 0);
				else loc.clear();
				enclosure_location = loc;
			}

		// devices already on a layout: by their full designation
	QSet<QString> placed;
	for (Diagram *folio : project->folios())
		if (!PanelLayout::surfaces(folio).isEmpty())
			for (Element *e : folio->elements()) {
				const QString of = e->elementInformations().value(QStringLiteral("panel_of")).toString();
				if (!of.isEmpty()) placed << of;
			}

	struct Row {Element *e; QString label; QString key; double w, h, dep;};
	QList<Row> rows;
	QSet<QString> seen, elsewhere;
	int duplicates = 0;
	for (Diagram *folio : project->folios()) {
		if (!PanelLayout::surfaces(folio).isEmpty()) continue;
		for (Element *e : folio->elements()) {
			const int t = e->linkType();
			if (!(t == Element::Simple || t == Element::Master || t == Element::Terminale)) continue;
			if (e->actualLabel().trimmed().isEmpty()) continue;
				// a terminal of a terminal strip goes onto the rail with its
				// strip ("Place terminal strips"), not on its own
			if (auto *te = qobject_cast<TerminalElement *>(e))
				if (te->realTerminal() && te->realTerminal()->parentStrip()) continue;
			const Tag tag = tagOf(e, folio);
			if (tag.bmk.isEmpty()) continue;
			if (!enclosure_location.isEmpty() && !tag.location.isEmpty()
				&& tag.location.compare(enclosure_location, Qt::CaseInsensitive) != 0) {
				elsewhere << keyOf(tag);
				continue;
			}
			const QString key = keyOf(tag);
			if (seen.contains(key)) {++duplicates; continue;}
			seen << key;
				// on the plate: plant and BMK, the location is the enclosure's
			const QString label = tag.plant.isEmpty() ? tag.bmk : QStringLiteral("=%1%2").arg(tag.plant, tag.bmk);
			const DiagramContext info = e->elementInformations();
			rows << Row{e, label, key, infoNumber(info, QStringLiteral("width_mm")),
						infoNumber(info, QStringLiteral("height_mm")),
						infoNumber(info, QStringLiteral("depth_mm"))};
		}
	}
	std::sort(rows.begin(), rows.end(), [](const Row &a, const Row &b) {
		return QString::localeAwareCompare(a.label, b.label) < 0;});
	if (rows.isEmpty()) {
		say(false, tr("The project has no devices with a designation."));
		return report.join(QLatin1Char('\n'));
	}

	const bool has_token = !QSettings().value(QStringLiteral("eplanimport/dataportal_token")).toString().isEmpty();
	QDialog dialog(parent);
	dialog.setWindowTitle(tr("Place devices on %1").arg(PanelLayout(surface).name()));
	dialog.resize(780, 560);
	auto *table = new QTableWidget(int(rows.size()), 6, &dialog);
	table->setHorizontalHeaderLabels({tr("Designation"), tr("Part"), tr("Width mm"), tr("Height mm"),
									  tr("Depth mm"), tr("Mounting variants")});
	table->verticalHeader()->hide();
	for (int i = 0; i < rows.size(); ++i) {
		const Row &r = rows.at(i);
		auto *c0 = new QTableWidgetItem(r.label);
		c0->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
			// ready to place: the size is known, or comes with the
			// manufacturer's drawing (an order number and a token)
		const bool sized = (r.w > 0 && r.h > 0)
				|| (has_token && !r.e->elementInformations().value(QStringLiteral("manufacturer_reference")).toString().isEmpty());
		c0->setCheckState(!placed.contains(r.key) && sized ? Qt::Checked : Qt::Unchecked);
		c0->setToolTip(r.key);
		table->setItem(i, 0, c0);
		const DiagramContext info = r.e->elementInformations();
		QString part = info.value(QStringLiteral("manufacturer_reference")).toString();
		if (part.isEmpty()) part = r.e->name();
		if (placed.contains(r.key)) part += QStringLiteral("  ") + tr("(already placed)");
		auto *c1 = new QTableWidgetItem(part);
		c1->setFlags(Qt::ItemIsEnabled);
		table->setItem(i, 1, c1);
		table->setItem(i, 2, new QTableWidgetItem(r.w > 0 ? num(r.w) : QStringLiteral("45")));
		table->setItem(i, 3, new QTableWidgetItem(r.h > 0 ? num(r.h) : QStringLiteral("90")));
		table->setItem(i, 4, new QTableWidgetItem(r.dep > 0 ? num(r.dep) : QStringLiteral("70")));
		QStringList variants;
		for (const PanelPattern &p : PanelLayout::patternsOf(r.e)) variants << p.variant;
		auto *c5 = new QTableWidgetItem(variants.isEmpty() ? tr("none") : variants.join(QStringLiteral(", ")));
		c5->setFlags(Qt::ItemIsEnabled);
		table->setItem(i, 5, c5);
	}
	table->resizeColumnsToContents();
	table->horizontalHeader()->setStretchLastSection(true);
	int without_order = 0;
	for (const Row &r : std::as_const(rows))
		if (r.e->elementInformations().value(QStringLiteral("manufacturer_reference")).toString().isEmpty()) ++without_order;
	auto *hint = new QLabel(tr("%1 devices at %2; %3 devices of other locations left out; %4 further mentions "
							   "of the same devices (other spelling or folio) merged; %5 without order number.")
								.arg(rows.size())
								.arg(enclosure_location.isEmpty() ? tr("all locations") : QStringLiteral("+") + enclosure_location)
								.arg(elsewhere.size()).arg(duplicates).arg(without_order)
							+ QStringLiteral("\n")
							+ tr("Devices without a size are proposed at 45 x 90 x 70 mm; correct the sizes here "
							   "or in the part. They are snapped onto the DIN rails from left to right."), &dialog);
	hint->setWordWrap(true);
	auto *graphics = new QCheckBox(tr("Manufacturer's panel layout drawing from the EPLAN Data Portal "
									  "(by order number; also gives the size)"), &dialog);
	graphics->setChecked(has_token);
	graphics->setEnabled(has_token);
	if (!has_token)
		graphics->setToolTip(tr("Needs your Data Portal token: Part import > From the EPLAN Data Portal"));
	auto *buttons = okCancel(&dialog);
	buttons->button(QDialogButtonBox::Ok)->setText(tr("Place"));
	auto *all = new QPushButton(tr("All"), &dialog);
	auto *none = new QPushButton(tr("None"), &dialog);
	auto check = [table](Qt::CheckState state) {
		for (int i = 0; i < table->rowCount(); ++i) table->item(i, 0)->setCheckState(state);
	};
	connect(all, &QPushButton::clicked, &dialog, [check]() {check(Qt::Checked);});
	connect(none, &QPushButton::clicked, &dialog, [check]() {check(Qt::Unchecked);});
	auto *select_row = new QHBoxLayout;
	select_row->addWidget(all);
	select_row->addWidget(none);
	select_row->addStretch();
	auto *box = new QVBoxLayout(&dialog);
	box->addWidget(table, 1);
	box->addLayout(select_row);
	box->addWidget(hint);
	box->addWidget(graphics);
	box->addWidget(buttons);
	if (interactive && dialog.exec() != QDialog::Accepted) return QString();

	XmlElementCollection *collection = project->embeddedElementCollection();
	if (!collection) return report.join(QLatin1Char('\n'));
	const QString dir_name = QStringLiteral("Cabinet layout");
	const QString dir_path = QStringLiteral("import/") + dir_name;
	if (!collection->exist(dir_path)) {
		NamesList names;
		names.addName(QStringLiteral("en"), dir_name);
		names.addName(QStringLiteral("de"), QStringLiteral("Schaltschrankaufbau"));
		if (!collection->createDir(QStringLiteral("import"), dir_name, names)) return report.join(QLatin1Char('\n'));
	}

	const PanelLayout layout(surface);
	const double k = layout.mmPerUnit();
	const QRectF plate = sceneRectOf(surface);

		// free run on every horizontal rail, after what is already on it
	struct Rail {QRectF r; double cursor; double room;};
	QList<Rail> rails;
		// the room around a rail: up to the next duct, rail or the plate's
		// edge above and below it, over the rail's length
	auto roomAround = [&](const QRectF &r) {
		double above = r.top() - plate.top(), below = plate.bottom() - r.bottom();
		for (const PanelProfile &o : layout.profiles()) {
			if (!o.shape) continue;
			const QRectF q = sceneRectOf(o.shape);
			if (q == r || q.right() <= r.left() || q.left() >= r.right()) continue;
			if (q.bottom() <= r.top()) above = qMin(above, r.top() - q.bottom());
			else if (q.top() >= r.bottom()) below = qMin(below, q.top() - r.bottom());
		}
			// a device sits centred on its rail
		return r.height() + 2 * qMin(above, below);
	};
	for (const PanelProfile &p : layout.profiles()) {
		if (p.data.role != PanelData::DinRail || !p.shape) continue;
		const QRectF r = sceneRectOf(p.shape);
		if (r.width() < r.height()) continue;   // vertical rails are left to the user
		double cursor = r.left() + 5 / k;
		for (const PanelDevice &dev : layout.devices()) {
			const QPointF a = layout.toScene(dev.rect_mm.topLeft()), b = layout.toScene(dev.rect_mm.bottomRight());
			const QRectF dr = QRectF(a, b).normalized();
			if (dr.intersects(r)) cursor = qMax(cursor, dr.right());
		}
		rails << Rail{r, cursor, roomAround(r)};
	}
	std::sort(rails.begin(), rails.end(), [](const Rail &a, const Rail &b) {return a.r.top() < b.r.top();});
	QPointF free_pos = plate.topLeft() + QPointF(20 / k, 20 / k);
	double free_row_h = 0;

	auto *macro = new QUndoCommand(tr("place devices on %1").arg(layout.name()));
	int created = 0;
	QStringList skipped, no_graphic, too_big;
		// the enclosure and the plate themselves are parts of the project
		// too: they are not placed on the plate
	QSet<QString> housing;
	housing << layout.surfaceData().article;
	for (QGraphicsItem *item : d->items())
		if (auto *shape = dynamic_cast<QetShapeItem *>(item))
			if (shape->panelData().role == PanelData::Enclosure) housing << shape->panelData().article;
	housing.remove(QString());
	QProgressDialog progress(tr("Placing devices…"), tr("Cancel"), 0, int(rows.size()), parent);
	if (!interactive) progress.setMinimumDuration(std::numeric_limits<int>::max());
	progress.setWindowModality(Qt::WindowModal);
	progress.setMinimumDuration(500);
	for (int i = 0; i < rows.size(); ++i)
	{
		progress.setValue(i);
		if (progress.wasCanceled()) break;
		if (table->item(i, 0)->checkState() != Qt::Checked) continue;
		const Row &r = rows.at(i);
		double w = cellNumber(table, i, 2), h = cellNumber(table, i, 3);
		const double dep = cellNumber(table, i, 4);
		const DiagramContext info = r.e->elementInformations();

			// the drawing of the panel layout from the manufacturer's data;
			// a device without sizes takes them from it
		PanelOutline graphic;
		const QString order = info.value(QStringLiteral("manufacturer_reference")).toString();
		if (housing.contains(order)) continue;
		if (graphics->isChecked() && !order.isEmpty()) {
			progress.setLabelText(tr("Drawing of %1 (%2)…").arg(r.label, order));
			QString why;
			graphic = PanelImport::fetchDeviceView(order, info.value(QStringLiteral("manufacturer")).toString(), &why);
			if (graphic.isEmpty()) no_graphic << QStringLiteral("%1 (%2)").arg(r.label, order);
			else if (r.w <= 0 || r.h <= 0) {w = graphic.width; h = graphic.height;}
		}
		if (w <= 0 || h <= 0) {skipped << r.label; continue;}
			// what does not fit the plate is no device on it (an enclosure,
			// a long busbar, a wall bracket)
		if (w > layout.widthMm() || h > layout.heightMm()) {
			too_big << QStringLiteral("%1 (%2 x %3 mm)").arg(r.label, num(w), num(h));
			continue;
		}

		const QDomElement patterns = r.e->location().xml().firstChildElement(QStringLiteral("drillingpatterns"));
		QDomDocument doc;
		QString title_text = r.label;
		const QString ref = info.value(QStringLiteral("manufacturer_reference")).toString();
		if (!ref.isEmpty()) title_text += QStringLiteral(" ") + ref;
		DiagramContext with_key = info;
		with_key.addValue(QStringLiteral("panel_of"), r.key, false);
		const QDomElement def = panelDeviceDefinition(doc, r.label, title_text, w, h, dep, k, with_key, patterns, graphic);
		const QString file_name = QET::stringToFileName(r.label) + QLatin1Char('-')
								  + QUuid::createUuid().toString(QUuid::Id128).left(8) + QStringLiteral(".elmt");
		if (!collection->addElementDefinition(dir_path, file_name, def)) {skipped << r.label; continue;}
		const ElementsLocation location(dir_path + QLatin1Char('/') + file_name, project);
		int state = 0;
		Element *element = ElementFactory::Instance()->createElement(location, nullptr, &state);
		if (state || !element) {delete element; skipped << r.label; continue;}

		const double ws = w / k, hs = h / k;
		QPointF pos;
		bool done = false;
		for (Rail &rail : rails) {
			if (rail.cursor + ws <= rail.r.right() - 2 / k && hs <= rail.room) {
					// EPLAN's middle offset: the device centre above the rail centre line
				const double offset = infoNumber(info, QStringLiteral("middle_offset_mm")) / k;
				pos = QPointF(rail.cursor, rail.r.center().y() - hs / 2 - offset);
				rail.cursor += ws;
				done = true;
				break;
			}
		}
		if (!done) {
			if (free_pos.x() + ws > plate.right() - 20 / k) {
				free_pos = QPointF(plate.left() + 20 / k, free_pos.y() + free_row_h + 10 / k);
				free_row_h = 0;
			}
			pos = free_pos;
			free_pos.rx() += ws + 10 / k;
			free_row_h = qMax(free_row_h, hs);
		}
		element->setPos(pos);
		d->addItem(element);
		new AddGraphicsObjectCommand(element, d, pos, macro);
		++created;
	}
	if (!created) {
		delete macro;
	} else {
		d->undoStack().push(macro);
		project->setModified(true);
	}
	progress.setValue(int(rows.size()));
	if (!skipped.isEmpty())
		say(true, tr("Not placed (no size or not readable): %1")
							 .arg(skipped.join(QStringLiteral(", "))));
	if (!too_big.isEmpty())
		say(false, tr("Larger than the mounting plate, not placed: %1")
								 .arg(too_big.join(QStringLiteral(", "))));
	if (!no_graphic.isEmpty())
		say(false, tr("No vector drawing in the Data Portal (shown as a rectangle of its size): %1")
								 .arg(no_graphic.join(QStringLiteral(", "))));
	return report.join(QLatin1Char('\n'));
}

/* ------------------------------------------------------------------ */

void PanelMenu::editSelected()
{
	Diagram *d = diagram();
	if (!d) return;
	const QList<QGraphicsItem *> selection = d->selectedItems();
	for (QGraphicsItem *item : selection)
		if (auto *shape = dynamic_cast<QetShapeItem *>(item)) {editShape(shape); return;}
	for (QGraphicsItem *item : selection)
		if (auto *element = dynamic_cast<Element *>(item)) {editDevice(element); return;}
	QMessageBox::information(m_editor, QString(title()).remove(QLatin1Char('&')),
							 tr("Select a shape (surface, rail, duct, cut-out) or a device first."));
}

void PanelMenu::editShape(QetShapeItem *shape)
{
	const PanelData data = shape->panelData();
	QDialog dialog(m_editor);
	dialog.setWindowTitle(tr("Manufacturing data"));
	auto *form = new QFormLayout;
	auto *role = new QComboBox(&dialog);
	const QList<QPair<PanelData::Role, QString>> roles = {
		{PanelData::None, tr("none (ordinary shape)")}, {PanelData::Surface, tr("Mounting surface")},
		{PanelData::DinRail, tr("DIN rail")}, {PanelData::Duct, tr("Cable duct")},
		{PanelData::CProfile, tr("C profile")}, {PanelData::Cutout, tr("Cut-out")},
		{PanelData::Keepout, tr("Keep-out area")}};
	for (const auto &r : roles) role->addItem(r.second, int(r.first));
	role->setCurrentIndex(qMax(0, role->findData(int(data.role))));

		// start a new role from its defaults, keep the values of the old one
	const PanelData base = data.isNull() ? PanelData::defaults(PanelData::DinRail) : data;
	auto *designation = new QLineEdit(data.designation, &dialog);
	auto *manufacturer = new QLineEdit(data.manufacturer, &dialog);
	auto *article = new QLineEdit(data.article, &dialog);
	auto *step = new QLineEdit(data.step_file, &dialog);

	auto *scale = new QDoubleSpinBox(&dialog);
	scale->setRange(0.1, 1000);
	scale->setDecimals(2);
	scale->setPrefix(QStringLiteral("1:"));
	scale->setValue(scaleForMmPerUnit(data.mm_per_unit));
	scale->setToolTip(tr("A new scale keeps the size of the surface in millimetres; move the parts on it to fit again"));
	auto *thickness = mmBox(data.thickness, 50, 1, &dialog);
	auto *machined = new QCheckBox(tr("Holes are drilled into this surface"), &dialog);
	machined->setChecked(data.machined);
	auto *origin = new QComboBox(&dialog);
	origin->addItems({tr("bottom left"), tr("top left"), tr("top right"), tr("bottom right")});
	origin->setCurrentIndex(data.origin);

	auto *depth = mmBox(base.profile_depth, 500, 1, &dialog);
	auto *fixing = new QComboBox(&dialog);
	fixing->addItem(tr("Tapped hole"), int(PanelHole::Thread));
	fixing->addItem(tr("Through hole"), int(PanelHole::Drill));
	fixing->setCurrentIndex(base.fixing.type == PanelHole::Drill ? 1 : 0);
	auto *fsize = mmBox(base.fixing.d1, 30, 1, &dialog);
	auto *spacing = mmBox(base.spacing, 2000, 1, &dialog);
	auto *end = mmBox(base.end_distance, 500, 1, &dialog);
	auto *rows = new QSpinBox(&dialog);
	rows->setRange(1, 2);
	rows->setValue(base.rows);
	auto *row_offset = mmBox(base.row_offset, 500, 1, &dialog);
	row_offset->setToolTip(tr("0: a quarter of the width"));
	auto *zero_end = new QCheckBox(tr("Measure the rows from the other end (zero cut)"), &dialog);
	zero_end->setChecked(base.zero_from_end);

	const PanelData cut_base = data.role == PanelData::Cutout ? data : PanelData::defaults(PanelData::Cutout);
	auto *ctype = new QComboBox(&dialog);
	ctype->addItem(tr("Round hole"), int(PanelHole::Drill));
	ctype->addItem(tr("Tapped hole"), int(PanelHole::Thread));
	ctype->addItem(tr("Rectangle"), int(PanelHole::Rectangle));
	ctype->addItem(tr("Slot"), int(PanelHole::Slot));
	ctype->addItem(tr("Hexagon"), int(PanelHole::Hexagon));
	ctype->addItem(tr("Octagon"), int(PanelHole::Octagon));
	ctype->setCurrentIndex(qMax(0, ctype->findData(int(cut_base.cutout.type))));
	auto *cd1 = mmBox(cut_base.cutout.d1, 2000, 2, &dialog);
	auto *cd2 = mmBox(cut_base.cutout.d2, 2000, 2, &dialog);
	auto *cd3 = mmBox(cut_base.cutout.d3, 200, 2, &dialog);

		// a part takes the scale of the surface it lies on
	const QRectF scene = sceneRectOf(shape);
	double k_part = data.mm_per_unit;
	if (data.role != PanelData::Surface)
		if (auto *d = qobject_cast<Diagram *>(shape->scene()))
			for (QetShapeItem *s : PanelLayout::surfaces(d))
				if (s != shape && sceneRectOf(s).contains(scene.center())) {k_part = s->panelData().mm_per_unit; break;}
		// a rail or duct: its length along the long side and its width;
		// a rail has the width of its profile (TS 35)
	const bool profile = data.role == PanelData::DinRail || data.role == PanelData::Duct
		|| data.role == PanelData::CProfile;
	const bool upright = profile && scene.height() > scene.width();
	auto *len_w = mmBox((upright ? scene.height() : scene.width()) * k_part, 10000, 1, &dialog);
	auto *len_h = mmBox((upright ? scene.width() : scene.height()) * k_part, 10000, 1, &dialog);
	if (data.role == PanelData::DinRail) len_h->setEnabled(false);

	form->addRow(tr("Role"), role);
	form->addRow(tr("Designation"), designation);
	form->addRow(tr("Manufacturer"), manufacturer);
	form->addRow(tr("Article number"), article);
	form->addRow(tr("STEP model"), stepPicker(step, &dialog));
	form->addRow(profile ? tr("Length") : tr("Width"), len_w);
	form->addRow(profile ? tr("Width") : tr("Height"), len_h);
	auto *g_surface = new QGroupBox(tr("Mounting surface"), &dialog);
	auto *fs = new QFormLayout(g_surface);
	fs->addRow(tr("Scale"), scale);
	fs->addRow(tr("Sheet thickness"), thickness);
	fs->addRow(QString(), machined);
	fs->addRow(tr("Zero point"), origin);
	auto *g_profile = new QGroupBox(tr("Fixing row"), &dialog);
	auto *fp = new QFormLayout(g_profile);
	fp->addRow(tr("Height above the surface"), depth);
	fp->addRow(tr("Fixing"), fixing);
	fp->addRow(tr("Fixing size"), fsize);
	fp->addRow(tr("Hole spacing"), spacing);
	fp->addRow(tr("End distance"), end);
	fp->addRow(tr("Rows of holes"), rows);
	fp->addRow(tr("Row distance from the centre"), row_offset);
	fp->addRow(QString(), zero_end);
	auto *g_cut = new QGroupBox(tr("Cut-out"), &dialog);
	auto *fc = new QFormLayout(g_cut);
	fc->addRow(tr("Kind"), ctype);
	fc->addRow(tr("Diameter / width"), cd1);
	fc->addRow(tr("Height"), cd2);
	fc->addRow(tr("Corner radius"), cd3);

	auto update_groups = [=]() {
		const auto r = PanelData::Role(role->currentData().toInt());
		g_surface->setVisible(r == PanelData::Surface);
		g_profile->setVisible(r == PanelData::DinRail || r == PanelData::Duct || r == PanelData::CProfile);
		g_cut->setVisible(r == PanelData::Cutout);
	};
	connect(role, &QComboBox::currentIndexChanged, &dialog, update_groups);
		// a new profile role: its own defaults
	connect(role, &QComboBox::currentIndexChanged, &dialog, [=]() {
		const auto r = PanelData::Role(role->currentData().toInt());
		if (r == data.role) return;
		if (r == PanelData::DinRail || r == PanelData::Duct || r == PanelData::CProfile) {
			const PanelData def = PanelData::defaults(r);
			depth->setValue(def.profile_depth);
			fixing->setCurrentIndex(def.fixing.type == PanelHole::Drill ? 1 : 0);
			fsize->setValue(def.fixing.d1);
			spacing->setValue(def.spacing);
			end->setValue(def.end_distance);
			rows->setValue(def.rows);
		}
	});
	update_groups();

	auto *box = new QVBoxLayout(&dialog);
	box->addLayout(form);
	box->addWidget(g_surface);
	box->addWidget(g_profile);
	box->addWidget(g_cut);
	box->addWidget(okCancel(&dialog));
	if (dialog.exec() != QDialog::Accepted) return;

	const PanelData::Role new_role = PanelData::Role(role->currentData().toInt());
	PanelData nd;
	if (new_role != PanelData::None) {
		nd = data.role == new_role ? data : PanelData::defaults(new_role);
		nd.role = new_role;
		nd.designation = designation->text().trimmed();
		nd.manufacturer = manufacturer->text().trimmed();
		nd.article = article->text().trimmed();
		nd.step_file = step->text().trimmed();
		if (new_role == PanelData::Surface) {
			nd.mm_per_unit = mmPerUnitForScale(scale->value());
			nd.thickness = thickness->value();
			nd.machined = machined->isChecked();
			nd.origin = origin->currentIndex();
		}
		if (new_role == PanelData::DinRail || new_role == PanelData::Duct || new_role == PanelData::CProfile) {
			nd.profile_depth = depth->value();
			nd.fixing.type = PanelHole::Type(fixing->currentData().toInt());
			nd.fixing.d1 = fsize->value();
			nd.spacing = spacing->value();
			nd.end_distance = end->value();
			nd.rows = rows->value();
			nd.row_offset = row_offset->value();
			nd.zero_from_end = zero_end->isChecked();
		}
		if (new_role == PanelData::Cutout) {
			nd.cutout.type = PanelHole::Type(ctype->currentData().toInt());
			nd.cutout.d1 = cd1->value();
			nd.cutout.d2 = cd2->value();
			nd.cutout.d3 = cd3->value();
			nd.cutout.subtype = cd3->value() > 0 ? PanelHole::Rounded : PanelHole::Plain;
		}
	}

	auto *macro = new QUndoCommand(tr("Edit cabinet layout data"));
	new ChangePanelDataCommand(shape, nd, macro);
	const double k = new_role == PanelData::Surface ? nd.mm_per_unit : k_part;
	const QRectF old_rect = shape->rect();
	const QRectF local = old_rect.normalized();
	const double w_mm = upright ? len_h->value() : len_w->value();
	const double h_mm = upright ? len_w->value() : len_h->value();
	const QRectF new_rect(local.topLeft(), QSizeF(w_mm / k, h_mm / k));
	const double old_w_mm = scene.width() * k_part, old_h_mm = scene.height() * k_part;
		// a new size, or a new scale of a surface which keeps its size in mm
	if (qAbs(w_mm - old_w_mm) > 0.05 || qAbs(h_mm - old_h_mm) > 0.05
		|| qAbs(k - k_part) > 1e-9)
		new QPropertyUndoCommand(shape, "rect", old_rect, new_rect, macro);
	if (Diagram *d = qobject_cast<Diagram *>(shape->scene())) d->undoStack().push(macro);
	else {macro->redo(); delete macro;}
	refreshHoleView();
}

void PanelMenu::editDevice(Element *element)
{
	const DiagramContext old_info = element->elementInformations();
	const QList<PanelPattern> patterns = PanelLayout::patternsOf(element);
	QDialog dialog(m_editor);
	dialog.setWindowTitle(tr("Mounting of %1").arg(element->actualLabel()));
	auto *form = new QFormLayout;
	auto *variant = new QComboBox(&dialog);
	variant->addItem(tr("automatic (DIN rail when on a rail)"), QString());
	for (const PanelPattern &p : patterns)
		variant->addItem(p.din_rail ? p.variant + QStringLiteral(" [") + tr("DIN rail") + QStringLiteral("]")
									: p.variant + QStringLiteral(" (") + tr("%n holes", nullptr, int(p.holes.size())) + QStringLiteral(")"),
						 p.variant);
	variant->setCurrentIndex(qMax(0, variant->findData(old_info.value(QStringLiteral("mount_variant")).toString())));
	auto *w = mmBox(infoNumber(old_info, QStringLiteral("width_mm")), 5000, 1, &dialog);
	auto *h = mmBox(infoNumber(old_info, QStringLiteral("height_mm")), 5000, 1, &dialog);
	auto *dep = mmBox(infoNumber(old_info, QStringLiteral("depth_mm")), 5000, 1, &dialog);
	auto *step = new QLineEdit(old_info.value(QStringLiteral("step_file")).toString(), &dialog);
	form->addRow(tr("Mounting variant"), variant);
	form->addRow(tr("Width"), w);
	form->addRow(tr("Height"), h);
	form->addRow(tr("Depth"), dep);
	form->addRow(tr("STEP model"), stepPicker(step, &dialog));
	if (patterns.isEmpty()) {
		auto *note = new QLabel(tr("The part has no drilling patterns: it leaves no holes unless it is "
								   "on a DIN rail. Import the part from the manufacturer's data to get them."), &dialog);
		note->setWordWrap(true);
		form->addRow(note);
	}
	auto *box = new QVBoxLayout(&dialog);
	box->addLayout(form);
	box->addWidget(okCancel(&dialog));
	if (dialog.exec() != QDialog::Accepted) return;

	DiagramContext new_info = old_info;
	new_info.addValue(QStringLiteral("mount_variant"), variant->currentData().toString(), false);
	new_info.addValue(QStringLiteral("width_mm"), w->value() > 0 ? num(w->value()) : QString(), false);
	new_info.addValue(QStringLiteral("height_mm"), h->value() > 0 ? num(h->value()) : QString(), false);
	new_info.addValue(QStringLiteral("depth_mm"), dep->value() > 0 ? num(dep->value()) : QString(), false);
	new_info.addValue(QStringLiteral("step_file"), step->text().trimmed(), false);
	if (new_info == old_info) return;
		// a new size: the device is drawn anew in it
	if ((infoNumber(new_info, QStringLiteral("width_mm")) != infoNumber(old_info, QStringLiteral("width_mm"))
		 || infoNumber(new_info, QStringLiteral("height_mm")) != infoNumber(old_info, QStringLiteral("height_mm")))
		&& replaceDevice(element, new_info)) {
		refreshHoleView();
		return;
	}
	if (auto *d = qobject_cast<Diagram *>(element->scene()))
		d->undoStack().push(new ChangeElementInformationCommand(element, old_info, new_info));
	else
		element->setElementInformations(new_info);
	refreshHoleView();
}

/* ------------------------------------------------------------------ */

void PanelMenu::toggleHoleView(bool on)
{
	for (const QPointer<QObject> &o : std::as_const(m_overlays)) delete o.data();
	m_overlays.clear();
	if (!on) return;
	Diagram *d = diagram();
	if (!d || !d->project()) return;
	for (Diagram *folio : d->project()->folios()) {
		if (PanelLayout::surfaces(folio).isEmpty()) continue;
		auto *overlay = new PanelHoleOverlay(folio);
		folio->addItem(overlay);
		overlay->refresh();
		m_overlays << overlay;
	}
	connect(d->project()->undoStack(), &QUndoStack::indexChanged, this,
			&PanelMenu::refreshHoleView, Qt::UniqueConnection);
}

void PanelMenu::refreshHoleView()
{
	if (!m_hole_view || !m_hole_view->isChecked()) return;
	Diagram *d = diagram();
	if (d && !PanelLayout::surfaces(d).isEmpty()) {
		bool have = false;
		for (const QPointer<QObject> &o : std::as_const(m_overlays))
			if (auto *ov = dynamic_cast<PanelHoleOverlay *>(o.data()))
				if (ov->scene() == d) have = true;
		if (!have) {toggleHoleView(true); return;}
	}
	for (const QPointer<QObject> &o : std::as_const(m_overlays))
		if (auto *ov = dynamic_cast<PanelHoleOverlay *>(o.data())) ov->refresh();
}

/* ------------------------------------------------------------------ */

void PanelMenu::showReport()
{
	Diagram *d = diagram();
	if (!d || !d->project()) return;
	showText(m_editor, tr("Manufacturing report"), reportText(d->project()));
}

/**
	@brief PanelMenu::exportManufacturing
	Like EPLAN's NC export for a drilling machine or a laser: one DXF per
	mounting surface (layers per hole kind), plus a hole list and a cut
	list as CSV, all in one folder.
*/
void PanelMenu::exportManufacturing()
{
	Diagram *d = diagram();
	if (!d || !d->project()) return;
	QETProject *project = d->project();

	QDialog dialog(m_editor);
	dialog.setWindowTitle(tr("Export manufacturing data"));
	auto *form = new QFormLayout;
	auto *dir = new QLineEdit(&dialog);
	const QString base = project->filePath().isEmpty() ? QDir::homePath()
														: QFileInfo(project->filePath()).absolutePath();
	dir->setText(QDir::toNativeSeparators(QDir(base).filePath(QStringLiteral("Fertigung"))));
	auto *browse = new QPushButton(QStringLiteral("..."), &dialog);
	connect(browse, &QPushButton::clicked, &dialog, [dir, &dialog]() {
		const QString p = QFileDialog::getExistingDirectory(&dialog, QString(), dir->text());
		if (!p.isEmpty()) dir->setText(QDir::toNativeSeparators(p));
	});
	auto *row = new QHBoxLayout;
	row->addWidget(dir, 1);
	row->addWidget(browse);
	auto *labels = new QCheckBox(tr("Write the designations into the DXF (layer BESCHRIFTUNG)"), &dialog);
	auto *core = new QCheckBox(tr("Tapped holes as core hole diameter (for drilling, not for a tapping head)"), &dialog);
	form->addRow(tr("Folder"), row);
	form->addRow(QString(), labels);
	form->addRow(QString(), core);
	auto *box = new QVBoxLayout(&dialog);
	box->addLayout(form);
	box->addWidget(okCancel(&dialog));
	if (dialog.exec() != QDialog::Accepted) return;

	const QString out_dir = dir->text();
	QDir().mkpath(out_dir);
	PanelDxf::Options options;
	options.labels = labels->isChecked();
	options.thread_core = core->isChecked();

	QStringList written, errors;
	QString holes_csv = tr("Folio;Surface;Kind;X mm;Y mm;Source") + QStringLiteral("\n");
	QString cut_csv = tr("Folio;Surface;Part") + QStringLiteral("\n");
	const QList<Diagram *> folios = project->folios();
	QSet<QString> names;
	for (int f = 0; f < folios.size(); ++f)
	{
		for (QetShapeItem *surface : PanelLayout::surfaces(folios.at(f)))
		{
			const PanelLayout layout(surface);
			QString name = layout.name();
			name.replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9_+=.-]")), QStringLiteral("_"));
			name = QStringLiteral("%1_%2").arg(f + 1, 2, 10, QLatin1Char('0')).arg(name);
			while (names.contains(name)) name += QStringLiteral("_");
			names << name;
			const QString path = QDir(out_dir).filePath(name + QStringLiteral(".dxf"));
			QString error;
			if (PanelDxf::write(layout, path, options, &error)) written << path;
			else errors << error;
			for (const PanelHole &h : layout.holes())
				holes_csv += QStringLiteral("%1;%2;%3;%4;%5;%6\n").arg(f + 1).arg(layout.name(), h.kindName(),
								num(h.x, 2), num(h.y, 2), h.source);
			for (const QString &c : layout.cutList())
				cut_csv += QStringLiteral("%1;%2;%3\n").arg(f + 1).arg(layout.name(), c);
		}
	}
	auto write_csv = [&](const QString &file, const QString &text) {
		QSaveFile out(QDir(out_dir).filePath(file));
		if (out.open(QIODevice::WriteOnly)) {
			out.write("\xEF\xBB\xBF");
			out.write(text.toUtf8());
			if (out.commit()) written << out.fileName();
		}
	};
	if (!names.isEmpty()) {
		write_csv(QStringLiteral("Bohrliste.csv"), holes_csv);
		write_csv(QStringLiteral("Schnittliste.csv"), cut_csv);
	}

	QString text;
	if (names.isEmpty()) text = tr("The project has no mounting surface: nothing written.") + QStringLiteral("\n");
	else {
		text = tr("Written:") + QStringLiteral("\n");
		for (const QString &w : std::as_const(written))
			text += QStringLiteral("  ") + QDir::toNativeSeparators(w) + QStringLiteral("\n");
	}
	if (!errors.isEmpty())
		text += QStringLiteral("\n") + tr("Errors:") + QStringLiteral("\n  ") + errors.join(QStringLiteral("\n  ")) + QStringLiteral("\n");
	text += QStringLiteral("\n") + reportText(project);
	showText(m_editor, tr("Export manufacturing data"), text);
}

void PanelMenu::show3D()
{
	Diagram *d = diagram();
	if (!d) return;
		// the first view reads every STEP model: say so instead of a frozen
		// window
	m_editor->statusBar()->showMessage(tr("Loading the 3D models..."));
	QCoreApplication::processEvents();
	auto *dialog = new Panel3DDialog(d, m_editor);
	m_editor->statusBar()->clearMessage();
	dialog->show();
}
