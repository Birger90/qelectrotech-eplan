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
#include "panelassistant.h"

#include "panellayout.h"
#include "panelmenu.h"
#include "../diagram.h"
#include "../qetgraphicsitem/element.h"
#include "../qetgraphicsitem/qetshapeitem.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QGraphicsView>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QTimer>
#include <QUndoStack>
#include <QVBoxLayout>

namespace {
	/// the mounting plate of the folio: the first surface inside the cabinet
QetShapeItem *mainSurface(Diagram *d)
{
	if (!d) return nullptr;
	for (QetShapeItem *s : PanelLayout::surfaces(d))
		if (s->panelData().face.isEmpty()) return s;
	return nullptr;
}

bool hasEnclosure(Diagram *d)
{
	if (!d) return false;
	for (QGraphicsItem *item : d->items())
		if (auto *s = dynamic_cast<QetShapeItem *>(item))
			if (s->panelData().role == PanelData::Enclosure) return true;
	return false;
}
}

/* ------------------------------------------------------------------ */

PanelCheckDock::PanelCheckDock(QWidget *parent) :
	QDockWidget(tr("Layout check"), parent)
{
	setObjectName(QStringLiteral("panel_check_dock"));
	auto *body = new QWidget(this);
	m_summary = new QLabel(body);
	m_summary->setWordWrap(true);
	m_list = new QListWidget(body);
	m_list->setWordWrap(true);
	m_list->setToolTip(tr("Click a finding to select the part on the folio"));
	auto *again = new QPushButton(tr("Check again"), body);
	auto *box = new QVBoxLayout(body);
	box->addWidget(m_summary);
	box->addWidget(m_list, 1);
	box->addWidget(again);
	setWidget(body);

	m_timer = new QTimer(this);
	m_timer->setSingleShot(true);
	m_timer->setInterval(400);
	connect(m_timer, &QTimer::timeout, this, &PanelCheckDock::refresh);
	connect(again, &QPushButton::clicked, this, &PanelCheckDock::refresh);
	connect(m_list, &QListWidget::itemClicked, this, &PanelCheckDock::showItem);
	connect(m_list, &QListWidget::itemActivated, this, &PanelCheckDock::showItem);
}

void PanelCheckDock::setDiagram(Diagram *diagram)
{
	if (m_diagram == diagram) return;
	if (m_diagram) disconnect(&m_diagram->undoStack(), nullptr, this, nullptr);
	m_diagram = diagram;
		// every change of the folio: check again a moment later
	if (m_diagram)
		connect(&m_diagram->undoStack(), &QUndoStack::indexChanged, this, [this]() {m_timer->start();});
	refresh();
}

void PanelCheckDock::refresh()
{
	m_list->clear();
	m_findings = 0;
	if (!m_diagram) {
		m_summary->setText(tr("No folio."));
		return;
	}
	const QList<QetShapeItem *> surfaces = PanelLayout::surfaces(m_diagram);
	if (surfaces.isEmpty()) {
		m_summary->setText(tr("This folio has no cabinet layout yet. Use \"Build cabinet...\" to start."));
		return;
	}
	for (QetShapeItem *surface : surfaces) {
		const PanelLayout layout(surface);
		for (const QString &w : layout.warnings()) {
			auto *item = new QListWidgetItem(w, m_list);
			item->setData(Qt::UserRole, w.section(QLatin1Char(':'), 0, 0).trimmed());
			item->setToolTip(layout.name());
			++m_findings;
		}
	}
	m_summary->setText(m_findings ? tr("%n finding(s) - click one to show the part.", nullptr, m_findings)
								   : tr("No findings: the layout is fine."));
}

void PanelCheckDock::showItem(QListWidgetItem *item)
{
	if (!item || !m_diagram) return;
	const QString tag = item->data(Qt::UserRole).toString();
	if (tag.isEmpty()) return;
	QList<QGraphicsItem *> hits;
	for (QGraphicsItem *gi : m_diagram->items()) {
		if (auto *e = dynamic_cast<Element *>(gi)) {
			if (e->parentItem()) continue;
			if (e->actualLabel() == tag
				|| e->elementInformations().value(QStringLiteral("label")).toString() == tag)
				hits << e;
		} else if (auto *s = dynamic_cast<QetShapeItem *>(gi)) {
			if (!s->panelData().isNull() && s->panelData().designation == tag) hits << s;
		}
	}
	if (hits.isEmpty()) return;
	m_diagram->clearSelection();
	QRectF area;
	for (QGraphicsItem *gi : std::as_const(hits)) {
		gi->setSelected(true);
		area |= gi->sceneBoundingRect();
	}
	for (QGraphicsView *view : m_diagram->views()) view->centerOn(area.center());
}

/* ------------------------------------------------------------------ */

int PanelAssistant::suggestedRows(double height_mm, double duct_mm)
{
		// a row of devices needs about 220 mm between two ducts
	return qMax(1, int((height_mm - duct_mm) / (duct_mm + 220)));
}

int PanelAssistant::addGrid(QetShapeItem *surface, int rows, double duct_mm)
{
	auto *d = surface ? qobject_cast<Diagram *>(surface->scene()) : nullptr;
	if (!d || rows < 1 || duct_mm <= 0) return 0;
	const PanelLayout layout(surface);
	const double W = layout.widthMm(), H = layout.heightMm(), dw = duct_mm;
	const double row_h = (H - 2 * dw - (rows - 1) * dw) / rows;
	if (row_h < 60 || W < 3 * dw) return 0;

	int added = 0, duct_no = 0, rail_no = 0;
	auto add = [&](PanelData::Role role, double x0, double y0, double x1, double y1, const QString &name) {
		PanelData data = PanelData::defaults(role);
		data.designation = name;
		const QRectF r = QRectF(layout.toScene(QPointF(x0, y0)), layout.toScene(QPointF(x1, y1))).normalized();
		PanelMenu::addShape(d, r, false, data, name);
		++added;
	};
	d->undoStack().beginMacro(QObject::tr("rails and ducts as a grid"));
		// ducts on the left and right edge over the full height
	add(PanelData::Duct, 0, 0, dw, H, QStringLiteral("-KK%1").arg(++duct_no));
	add(PanelData::Duct, W - dw, 0, W, H, QStringLiteral("-KK%1").arg(++duct_no));
		// ducts across: bottom, between the rows, top
	for (int i = 0; i <= rows; ++i) {
		const double y = i == 0 ? 0 : i == rows ? H - dw : dw + i * row_h + (i - 1) * dw;
		add(PanelData::Duct, dw, y, W - dw, y + dw, QStringLiteral("-KK%1").arg(++duct_no));
	}
		// one DIN rail in the middle of each row
	for (int i = 0; i < rows; ++i) {
		const double c = dw + i * (row_h + dw) + row_h / 2;
		add(PanelData::DinRail, dw + 10, c - 17.5, W - dw - 10, c + 17.5, QStringLiteral("-HS%1").arg(++rail_no));
	}
	d->undoStack().endMacro();
	return added;
}

PanelAssistant::PanelAssistant(Diagram *diagram, const Actions &actions, QWidget *parent) :
	QDialog(parent),
	m_diagram(diagram),
	m_actions(actions)
{
	setWindowTitle(tr("Build cabinet"));
	setAttribute(Qt::WA_DeleteOnClose);
	setModal(false);
	auto *grid = new QGridLayout;
	grid->setColumnStretch(2, 1);
	int row = 0;
	auto step = [&](const QString &title, const QString &help, QWidget *control) {
		auto *mark = new QLabel(this);
		mark->setMinimumWidth(22);
		auto *text = new QLabel(QStringLiteral("<b>%1</b><br><span style=\"color:gray\">%2</span>").arg(title, help), this);
		text->setWordWrap(true);
		auto *state = new QLabel(this);
		state->setWordWrap(true);
		auto *texts = new QVBoxLayout;
		texts->addWidget(text);
		texts->addWidget(state);
		grid->addWidget(mark, row, 0, Qt::AlignTop);
		grid->addWidget(new QLabel(QString::number(row + 1) + QLatin1Char('.'), this), row, 1, Qt::AlignTop);
		grid->addLayout(texts, row, 2);
		grid->addWidget(control, row, 3, Qt::AlignTop);
		m_marks << mark;
		m_states << state;
		++row;
	};
	auto button = [&](const QString &text, std::function<void()> run) {
		auto *b = new QPushButton(text, this);
		connect(b, &QPushButton::clicked, this, [this, run]() {
			if (run) run();
			update();
		});
		return b;
	};

	step(tr("Enclosure and mounting plate"),
		 tr("Choose the enclosure, e.g. a Rittal VX by its order number; its mounting plate comes with it."),
		 button(tr("Choose..."), m_actions.enclosure));

	auto *grid_box = new QWidget(this);
	auto *grid_row = new QHBoxLayout(grid_box);
	grid_row->setContentsMargins(0, 0, 0, 0);
	m_rows = new QSpinBox(grid_box);
	m_rows->setRange(1, 12);
	m_rows->setSuffix(tr(" rows"));
	m_duct = new QComboBox(grid_box);
	for (int w : {40, 60, 80, 100}) m_duct->addItem(tr("duct %1 mm").arg(w), w);
	m_duct->setCurrentIndex(1);
	auto *make = new QPushButton(tr("Create"), grid_box);
	grid_row->addWidget(m_rows);
	grid_row->addWidget(m_duct);
	grid_row->addWidget(make);
	connect(make, &QPushButton::clicked, this, [this]() {
		addGrid(mainSurface(m_diagram), m_rows->value(), m_duct->currentData().toDouble());
		update();
	});
	step(tr("DIN rails and cable ducts"),
		 tr("Ducts on the edges and between the rows, one DIN rail per row. Move or stretch them afterwards as you like."),
		 grid_box);

	step(tr("Devices from the schematic"),
		 tr("Every device with an order number goes onto the rails at its real size, with the manufacturer's drawing."),
		 button(tr("Place"), m_actions.devices));
	step(tr("Terminal strips"),
		 tr("The terminal strips of the project onto the rails, terminal by terminal as in the terminal plans."),
		 button(tr("Place"), m_actions.terminals));
	step(tr("Check"),
		 tr("Parts on each other, in a duct, beyond the plate, without holes. Click a finding to see the part."),
		 button(tr("Check list"), m_actions.checks));

	auto *done_box = new QWidget(this);
	auto *done_row = new QHBoxLayout(done_box);
	done_row->setContentsMargins(0, 0, 0, 0);
	done_row->addWidget(button(tr("3D view"), m_actions.view3d));
	done_row->addWidget(button(tr("Export"), m_actions.manufacturing));
	step(tr("3D and manufacturing"),
		 tr("Look at the cabinet in 3D, then export the drilling files (DXF), hole and cut lists."),
		 done_box);

	auto *close = new QDialogButtonBox(QDialogButtonBox::Close, this);
	connect(close, &QDialogButtonBox::rejected, this, &QDialog::close);
	auto *box = new QVBoxLayout(this);
	box->addLayout(grid);
	box->addStretch();
	box->addWidget(close);
	resize(660, 520);

	if (m_diagram) connect(&m_diagram->undoStack(), &QUndoStack::indexChanged, this, &PanelAssistant::update);
	update();
}

void PanelAssistant::update()
{
	QetShapeItem *plate = mainSurface(m_diagram);
	const bool enclosure = hasEnclosure(m_diagram) && plate;
	int profiles = 0, devices = 0, strips = 0, findings = 0;
	if (plate) {
		const PanelLayout layout(plate);
		profiles = layout.profiles().size();
		for (const PanelDevice &dev : layout.devices()) {
			if (!dev.terminals.isEmpty()
				|| (dev.element && dev.element->elementInformations().value(QStringLiteral("panel_of")).toString().startsWith(QLatin1String("strip:"))))
				++strips;
			else
				++devices;
		}
		if (profiles == 0 && m_rows->value() == 1)
			m_rows->setValue(suggestedRows(layout.heightMm(), m_duct->currentData().toDouble()));
	}
	if (m_diagram)
		for (QetShapeItem *s : PanelLayout::surfaces(m_diagram)) findings += PanelLayout(s).warnings().size();

	const bool done[] = {enclosure, profiles > 0, devices > 0, strips > 0, findings == 0 && devices > 0, false};
	const QString state[] = {
		enclosure ? tr("Done.") : plate ? tr("Mounting plate found, no enclosure yet.") : tr("Not yet."),
		profiles ? tr("%n rail(s) and duct(s) on the plate.", nullptr, profiles) : tr("Not yet."),
		devices ? tr("%n device(s) on the plate.", nullptr, devices) : tr("Not yet."),
		strips ? tr("%n terminal strip(s) on the plate.", nullptr, strips) : tr("Not yet."),
		!devices ? QString() : findings ? tr("%n finding(s) to look at.", nullptr, findings) : tr("No findings."),
		QString()};
	for (int i = 0; i < m_marks.size() && i < 6; ++i) {
		m_marks.at(i)->setText(done[i] ? QStringLiteral("<span style=\"color:#2a8a2a;font-size:16px\">&#10004;</span>")
									   : QStringLiteral("<span style=\"color:#b0b0b0;font-size:16px\">&#9679;</span>"));
		m_states.at(i)->setText(state[i]);
	}
}
