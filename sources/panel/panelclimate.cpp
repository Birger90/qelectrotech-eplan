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
#include "panelclimate.h"

#include "panelcommands.h"
#include "panellayout.h"
#include "panelmenu.h"
#include "../ElementsCollection/elementslocation.h"
#include "../diagram.h"
#include "../qetgraphicsitem/element.h"
#include "../qetgraphicsitem/qetshapeitem.h"
#include "../qetproject.h"
#include "../undocommand/changeelementinformationcommand.h"

#include <QComboBox>
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QHash>
#include <QJsonArray>
#include <QLabel>
#include <QTableWidget>
#include <QUndoStack>
#include <QVBoxLayout>
#include <QtMath>

namespace {

double number(QString text)
{
	return text.replace(QLatin1Char(','), QLatin1Char('.')).trimmed().toDouble();
}

QString fmt(double v, int decimals = 1)
{
	return QString::number(v, 'f', decimals).replace(QLatin1Char('.'), QLatin1Char(','));
}

	/// the enclosure drawn around the plate, if any
QetShapeItem *enclosureOf(QetShapeItem *plate)
{
	auto *d = plate ? qobject_cast<Diagram *>(plate->scene()) : nullptr;
	if (!d) return nullptr;
	const QPointF centre = plate->mapToScene(plate->rect().normalized()).boundingRect().center();
	for (QGraphicsItem *item : d->items())
		if (auto *shape = dynamic_cast<QetShapeItem *>(item))
			if (shape->panelData().role == PanelData::Enclosure
				&& shape->mapToScene(shape->rect().normalized()).boundingRect().contains(centre))
				return shape;
	return nullptr;
}

	/// materials of the enclosure wall and their heat transfer coefficient
struct Material {const char *name; double k;};
const Material kMaterials[] = {
	{QT_TRANSLATE_NOOP("PanelClimateDialog", "Sheet steel, painted"), 5.5},
	{QT_TRANSLATE_NOOP("PanelClimateDialog", "Stainless steel"), 4.5},
	{QT_TRANSLATE_NOOP("PanelClimateDialog", "Aluminium"), 12.0},
	{QT_TRANSLATE_NOOP("PanelClimateDialog", "Aluminium, double-walled"), 4.5},
	{QT_TRANSLATE_NOOP("PanelClimateDialog", "Plastic"), 3.5},
};

} // namespace

/* ------------------------------------------------------------------ */

double PanelClimate::effectiveArea(int installation, double w, double h, double d)
{
		// surface factor b of IEC/TR 60890: an exposed top 1.4, a covered
		// top 0.7, an exposed side 0.9, a covered side 0.5; the floor does
		// not count
	const bool wall = installation == 2 || installation == 4 || installation >= 6;
	const int row = installation <= 2 ? 0 : installation <= 4 ? 1 : 2;   // single, first/last, middle
	const double top = installation == 7 ? 0.7 : 1.4;
	const double front_back = 0.9 + (wall ? 0.5 : 0.9);
	const double sides = row == 0 ? 0.9 + 0.9 : row == 1 ? 0.9 + 0.5 : 0.5 + 0.5;
	return top * w * d + front_back * w * h + sides * d * h;
}

QString PanelClimate::installationName(int installation)
{
	switch (installation) {
		case 1: return QCoreApplication::translate("PanelClimate", "1 single enclosure, free on all sides");
		case 2: return QCoreApplication::translate("PanelClimate", "2 single enclosure against a wall");
		case 3: return QCoreApplication::translate("PanelClimate", "3 first or last of a row, free-standing");
		case 4: return QCoreApplication::translate("PanelClimate", "4 first or last of a row, against a wall");
		case 5: return QCoreApplication::translate("PanelClimate", "5 middle of a row, free-standing");
		case 6: return QCoreApplication::translate("PanelClimate", "6 middle of a row, against a wall");
		case 7: return QCoreApplication::translate("PanelClimate", "7 middle of a row, against a wall, roof covered");
		default: return QString();
	}
}

double PanelClimate::ownPowerLoss(Element *element)
{
	if (!element) return 0;
	const double own = number(element->elementInformations().value(QStringLiteral("power_loss_w")).toString());
	if (own > 0) return own;
		// placed before its definition knew it: the definition's value
	const QDomElement infos = element->location().xml().firstChildElement(QStringLiteral("elementInformations"));
	for (QDomElement i = infos.firstChildElement(QStringLiteral("elementInformation")); !i.isNull();
		 i = i.nextSiblingElement(QStringLiteral("elementInformation")))
		if (i.attribute(QStringLiteral("name")) == QLatin1String("power_loss_w")) return qMax(0.0, number(i.text()));
	return 0;
}

PanelClimate PanelClimate::compute(QetShapeItem *plate)
{
	return compute(plate, plate ? plate->panelData().climate : PanelClimateSettings());
}

PanelClimate PanelClimate::compute(QetShapeItem *plate, const PanelClimateSettings &settings)
{
	PanelClimate c;
	c.settings = settings;
	if (!plate) return c;
	const PanelLayout layout(plate);
	c.name = layout.name();

		// the size: the enclosure drawn around the plate, else the plate
		// with a margin and the deepest device
	double deepest = 0;
	for (const PanelDevice &d : layout.devices()) deepest = qMax(deepest, d.depth_mm);
	if (QetShapeItem *enclosure = enclosureOf(plate)) {
		const PanelData ed = enclosure->panelData();
		const double k = ed.mm_per_unit > 0 ? ed.mm_per_unit : layout.mmPerUnit();
		const QRectF r = enclosure->mapToScene(enclosure->rect().normalized()).boundingRect();
		c.width_mm = r.width() * k;
		c.height_mm = r.height() * k;
		c.depth_mm = ed.profile_depth;
		if (!ed.designation.isEmpty()) c.name = ed.designation;
		else if (!ed.article.isEmpty()) c.name = ed.article;
	} else {
		c.width_mm = layout.widthMm() + 100;
		c.height_mm = layout.heightMm() + 100;
		c.size_estimated = true;
	}
	if (c.depth_mm <= 0) {
		c.depth_mm = qMax(300.0, deepest + 100);
		c.size_estimated = true;
	}
	if (settings.width_mm > 0) c.width_mm = settings.width_mm;
	if (settings.height_mm > 0) c.height_mm = settings.height_mm;
	if (settings.depth_mm > 0) c.depth_mm = settings.depth_mm;
	if (settings.width_mm > 0 && settings.height_mm > 0 && settings.depth_mm > 0) c.size_estimated = false;

		// the devices of the schematic by their full designation, for the
		// power loss a layout device does not carry itself
	QHash<QString, double> wanted;   // panel_of -> power loss found in the schematic
	for (const PanelDevice &d : layout.devices()) {
		if (!d.element) continue;
		const DiagramContext info = d.element->elementInformations();
		const QString of = info.value(QStringLiteral("panel_of")).toString();
			// terminal strips: their power loss is left out (a few mW per terminal)
		if (!d.terminals.isEmpty() || of.startsWith(QLatin1String("strip:"))) continue;
		Device x;
		x.element = d.element;
		x.designation = d.designation;
		x.article = info.value(QStringLiteral("manufacturer_reference")).toString();
		x.watts = ownPowerLoss(d.element);
		if (x.watts > 0) x.source = QCoreApplication::translate("PanelClimate", "layout");
		else if (!of.isEmpty()) wanted.insert(of, 0);
		c.devices << x;
	}
	auto *diagram = qobject_cast<Diagram *>(plate->scene());
	if (!wanted.isEmpty() && diagram && diagram->project()) {
		for (Diagram *folio : diagram->project()->folios()) {
			if (!PanelLayout::surfaces(folio).isEmpty()) continue;
			for (Element *e : folio->elements()) {
				Element *holder = PanelMenu::partHolder(e);
				const auto it = wanted.find(PanelLayout::deviceTag(holder).key());
				if (it != wanted.end() && it.value() <= 0) it.value() = ownPowerLoss(holder);
			}
		}
		for (Device &x : c.devices) {
			if (x.watts > 0) continue;
			x.watts = wanted.value(x.element->elementInformations().value(QStringLiteral("panel_of")).toString());
			if (x.watts > 0) x.source = QCoreApplication::translate("PanelClimate", "schematic");
		}
	}
	c.evaluate();
	return c;
}

void PanelClimate::evaluate()
{
	device_w = 0;
	without_loss = 0;
	for (const Device &d : std::as_const(devices)) {
		device_w += d.watts;
		if (d.watts <= 0) ++without_loss;
	}
	const PanelClimateSettings &s = settings;
	power_w = s.simultaneity * device_w + s.extra_w;
	area_m2 = effectiveArea(s.installation, width_mm / 1000, height_mm / 1000, depth_mm / 1000);
	const double ka = s.k * area_m2;   // W/K through the walls
	rise_k = ka > 0 ? power_w / ka : 0;
	inside_c = s.ambient_c + rise_k;
	cooling_w = power_w - ka * (s.inside_c - s.ambient_c);
	fan_m3h = 0;
	exchanger_w_k = 0;
	if (cooling_w > 0 && s.inside_c > s.ambient_c) {
		const double dt = s.inside_c - s.ambient_c;
		fan_m3h = kAirFactor * cooling_w / dt;
		exchanger_w_k = cooling_w / dt;
	}
}

QString PanelClimate::summary() const
{
	if (devices.isEmpty() && power_w <= 0)
		return QStringLiteral("Klima %1: keine Geräte auf der Platte").arg(name);
	QString text = QStringLiteral("Klima %1: P = %2 W").arg(name, fmt(power_w));
	if (without_loss)
		text += QStringLiteral(" (%1 von %2 Geräten ohne Verlustleistung)").arg(without_loss).arg(devices.size());
	text += QStringLiteral(", A_e = %1 m², ΔT = %2 K → innen %3 °C bei %4 °C außen")
				.arg(fmt(area_m2, 2), fmt(rise_k), fmt(inside_c), fmt(settings.ambient_c));
	if (!needsCooling())
		return text + QStringLiteral(" – zulässig %1 °C, keine Kühlung nötig").arg(fmt(settings.inside_c));
	text += QStringLiteral(" – über %1 °C: %2 W abführen").arg(fmt(settings.inside_c), fmt(cooling_w, 0));
	if (fan_m3h > 0)
		text += QStringLiteral(" (Filterlüfter ≥ %1 m³/h, Wärmetauscher ≥ %2 W/K oder Kühlgerät)")
					.arg(fmt(fan_m3h, 0), fmt(exchanger_w_k, 1));
	else
		text += QStringLiteral(" (Kühlgerät, außen nicht kühler als innen)");
	return text;
}

QJsonObject PanelClimate::toJson() const
{
	QJsonObject o;
	auto round = [](double v, int d) {const double f = qPow(10, d); return qRound(v * f) / f;};
	o[QStringLiteral("name")] = name;
	o[QStringLiteral("width_mm")] = round(width_mm, 1);
	o[QStringLiteral("height_mm")] = round(height_mm, 1);
	o[QStringLiteral("depth_mm")] = round(depth_mm, 1);
	o[QStringLiteral("size_estimated")] = size_estimated;
	o[QStringLiteral("installation")] = settings.installation;
	o[QStringLiteral("ambient_c")] = settings.ambient_c;
	o[QStringLiteral("inside_max_c")] = settings.inside_c;
	o[QStringLiteral("k")] = settings.k;
	o[QStringLiteral("simultaneity")] = settings.simultaneity;
	o[QStringLiteral("extra_w")] = settings.extra_w;
	o[QStringLiteral("device_w")] = round(device_w, 2);
	o[QStringLiteral("power_w")] = round(power_w, 2);
	o[QStringLiteral("devices_without_loss")] = without_loss;
	o[QStringLiteral("area_m2")] = round(area_m2, 3);
	o[QStringLiteral("rise_k")] = round(rise_k, 2);
	o[QStringLiteral("inside_c")] = round(inside_c, 2);
	o[QStringLiteral("cooling_w")] = round(cooling_w, 1);
	o[QStringLiteral("needs_cooling")] = needsCooling();
	o[QStringLiteral("fan_m3h")] = round(fan_m3h, 1);
	o[QStringLiteral("exchanger_w_k")] = round(exchanger_w_k, 2);
	QJsonArray list;
	for (const Device &d : devices)
		list.append(QJsonObject{{QStringLiteral("designation"), d.designation},
								{QStringLiteral("article"), d.article},
								{QStringLiteral("power_loss_w"), d.watts},
								{QStringLiteral("source"), d.source}});
	o[QStringLiteral("devices")] = list;
	o[QStringLiteral("summary")] = summary();
	return o;
}

/* ------------------------------------------------------------------ */

PanelClimateDialog::PanelClimateDialog(QetShapeItem *plate, QWidget *parent) :
	QDialog(parent),
	m_plate(plate)
{
	setWindowTitle(tr("Climate calculation"));
	resize(820, 640);
	PanelClimateSettings own = plate ? plate->panelData().climate : PanelClimateSettings();
	PanelClimateSettings auto_size = own;
	auto_size.width_mm = auto_size.height_mm = auto_size.depth_mm = 0;
	m_base = PanelClimate::compute(plate, auto_size);

	auto spin = [this](double value, double lo, double hi, int decimals, const QString &suffix) {
		auto *b = new QDoubleSpinBox(this);
		b->setRange(lo, hi);
		b->setDecimals(decimals);
		b->setSuffix(suffix);
		b->setValue(value);
		connect(b, &QDoubleSpinBox::valueChanged, this, &PanelClimateDialog::recompute);
		return b;
	};
	m_installation = new QComboBox(this);
	for (int i = 1; i <= 7; ++i) m_installation->addItem(PanelClimate::installationName(i), i);
	m_installation->setCurrentIndex(qBound(1, own.installation, 7) - 1);
	m_installation->setToolTip(tr("How the enclosure stands: which of its sides give off heat (IEC/TR 60890)"));
	connect(m_installation, &QComboBox::currentIndexChanged, this, &PanelClimateDialog::recompute);
	m_ambient = spin(own.ambient_c, -40, 80, 1, QStringLiteral(" °C"));
	m_inside = spin(own.inside_c, -20, 90, 1, QStringLiteral(" °C"));
	m_inside->setToolTip(tr("The warmest air allowed inside, from the devices' data (often 35 to 40 °C)"));
	m_material = new QComboBox(this);
	for (const Material &m : kMaterials) m_material->addItem(tr(m.name), m.k);
	m_material->addItem(tr("Other"), 0.0);
	m_k = spin(own.k, 0.5, 50, 2, QStringLiteral(" W/(m²·K)"));
	m_material->setCurrentIndex(m_material->count() - 1);
	for (int i = 0; i < m_material->count() - 1; ++i)
		if (qAbs(m_material->itemData(i).toDouble() - own.k) < 1e-9) {m_material->setCurrentIndex(i); break;}
	connect(m_material, &QComboBox::currentIndexChanged, this, [this]() {
		const double k = m_material->currentData().toDouble();
		if (k > 0) m_k->setValue(k);
	});
	m_simultaneity = spin(own.simultaneity, 0, 1, 2, QString());
	m_simultaneity->setSingleStep(0.1);
	m_simultaneity->setToolTip(tr("The share of the power loss that is there at the same time (1: all devices at full load)"));
	m_extra = spin(own.extra_w, 0, 100000, 1, QStringLiteral(" W"));
	m_extra->setToolTip(tr("Power loss not in the device data: wiring, busbars, devices not on the layout"));
	const QString from_enclosure = tr("from the enclosure");
	auto size = [&](double value, double base) {
		QDoubleSpinBox *b = spin(value, 0, 10000, 0, QStringLiteral(" mm"));
		b->setSpecialValueText(from_enclosure + QStringLiteral(" (%1 mm)").arg(fmt(base, 0)));
		return b;
	};
	m_width = size(own.width_mm, m_base.width_mm);
	m_height = size(own.height_mm, m_base.height_mm);
	m_depth = size(own.depth_mm, m_base.depth_mm);

	auto *conditions = new QGroupBox(tr("Enclosure"), this);
	auto *form = new QFormLayout(conditions);
	form->addRow(tr("Installation"), m_installation);
	auto *material_row = new QHBoxLayout;
	material_row->addWidget(m_material, 1);
	material_row->addWidget(m_k);
	form->addRow(tr("Wall"), material_row);
	form->addRow(tr("Width"), m_width);
	form->addRow(tr("Height"), m_height);
	form->addRow(tr("Depth"), m_depth);
	form->addRow(tr("Ambient temperature"), m_ambient);
	form->addRow(tr("Inside temperature allowed"), m_inside);
	form->addRow(tr("Simultaneity factor"), m_simultaneity);
	form->addRow(tr("Further power loss"), m_extra);

	m_table = new QTableWidget(int(m_base.devices.size()), 4, this);
	m_table->setHorizontalHeaderLabels({tr("Device"), tr("Part"), tr("Power loss W"), tr("From")});
	m_table->verticalHeader()->hide();
	m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
	m_table->setToolTip(tr("Power loss of each device (EPLAN: max. power dissipation); a value typed here is kept by the device on the layout"));
	m_filling = true;
	for (int i = 0; i < m_base.devices.size(); ++i) {
		const PanelClimate::Device &d = m_base.devices.at(i);
		auto fixed = [](const QString &text) {
			auto *item = new QTableWidgetItem(text);
			item->setFlags(item->flags() & ~Qt::ItemIsEditable);
			return item;
		};
		m_table->setItem(i, 0, fixed(d.designation));
		m_table->setItem(i, 1, fixed(d.article));
		m_table->setItem(i, 2, new QTableWidgetItem(d.watts > 0 ? fmt(d.watts, 2) : QString()));
		m_table->setItem(i, 3, fixed(d.source));
	}
	m_filling = false;
	connect(m_table, &QTableWidget::itemChanged, this, [this]() {if (!m_filling) recompute();});

	m_result = new QLabel(this);
	m_result->setWordWrap(true);
	m_result->setTextFormat(Qt::RichText);
	m_result->setTextInteractionFlags(Qt::TextSelectableByMouse);

	auto *top = new QHBoxLayout;
	top->addWidget(conditions, 0);
	top->addWidget(m_result, 1);
	auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	connect(buttons, &QDialogButtonBox::accepted, this, &PanelClimateDialog::accept);
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
	auto *hint = new QLabel(tr("Guide values for an even power loss and mixed air inside (IEC/TR 60890 surface factors, "
							   "k·A·ΔT through the walls); formulas and sources in FORMELN.md."), this);
	hint->setWordWrap(true);
	hint->setStyleSheet(QStringLiteral("color: gray"));
	auto *box = new QVBoxLayout(this);
	box->addLayout(top);
	box->addWidget(m_table, 1);
	box->addWidget(hint);
	box->addWidget(buttons);
	recompute();
}

PanelClimateSettings PanelClimateDialog::settings() const
{
	PanelClimateSettings s;
	s.installation = m_installation->currentData().toInt();
	s.ambient_c = m_ambient->value();
	s.inside_c = m_inside->value();
	s.k = m_k->value();
	s.simultaneity = m_simultaneity->value();
	s.extra_w = m_extra->value();
	s.width_mm = m_width->value();
	s.height_mm = m_height->value();
	s.depth_mm = m_depth->value();
	return s;
}

void PanelClimateDialog::recompute()
{
	if (!m_result) return;
	PanelClimate c = m_base;
	c.settings = settings();
	if (c.settings.width_mm > 0) c.width_mm = c.settings.width_mm;
	if (c.settings.height_mm > 0) c.height_mm = c.settings.height_mm;
	if (c.settings.depth_mm > 0) c.depth_mm = c.settings.depth_mm;
	for (int i = 0; i < c.devices.size() && i < m_table->rowCount(); ++i)
		c.devices[i].watts = qMax(0.0, number(m_table->item(i, 2)->text()));
	c.evaluate();
	const PanelClimateSettings &s = c.settings;
	QString html = QStringLiteral("<p><b>%1</b>%2</p>").arg(c.name.toHtmlEscaped(),
		c.size_estimated ? QStringLiteral(" &nbsp;<span style=\"color:#b06000\">%1</span>")
								  .arg(tr("(no enclosure on the folio: size estimated from the plate)"))
						 : QString());
	html += QStringLiteral("<table cellspacing=\"2\">");
	auto row = [&](const QString &what, const QString &value) {
		html += QStringLiteral("<tr><td>%1</td><td align=\"right\">&nbsp;&nbsp;<b>%2</b></td></tr>").arg(what, value);
	};
	row(tr("Power loss of the devices"), fmt(c.device_w) + QStringLiteral(" W"));
	if (c.without_loss) row(tr("Devices without power loss data"), QString::number(c.without_loss));
	row(tr("Power loss counted P"), fmt(c.power_w) + QStringLiteral(" W"));
	row(tr("Effective surface A<sub>e</sub>"), fmt(c.area_m2, 2) + QStringLiteral(" m²"));
	row(tr("Heat through the walls k·A<sub>e</sub>"), fmt(s.k * c.area_m2, 2) + QStringLiteral(" W/K"));
	row(tr("Temperature rise ΔT = P/(k·A<sub>e</sub>)"), fmt(c.rise_k) + QStringLiteral(" K"));
	row(tr("Inside without cooling"), fmt(c.inside_c) + QStringLiteral(" °C"));
	html += QStringLiteral("</table>");
	if (c.power_w <= 0) {
		html += QStringLiteral("<p>%1</p>").arg(tr("No power loss known yet: type it into the table or import the parts from the EPLAN Data Portal (EDZ)."));
	} else if (!c.needsCooling()) {
		html += QStringLiteral("<p style=\"color:#2a7a2a\"><b>%1</b></p>")
					.arg(tr("No cooling needed: the walls give off the heat, %1 K below the %2 °C allowed.")
							 .arg(fmt(s.inside_c - c.inside_c), fmt(s.inside_c)));
	} else {
		html += QStringLiteral("<p style=\"color:#b02020\"><b>%1</b></p>")
					.arg(tr("Cooling needed: %1 W to take away to keep %2 °C inside.").arg(fmt(c.cooling_w, 0), fmt(s.inside_c)));
		html += QStringLiteral("<ul>");
		if (c.fan_m3h > 0) {
			html += QStringLiteral("<li>%1</li>").arg(tr("Filter fan: at least %1 m³/h (V = 3.1 · Q / ΔT, up to 100 m above sea level)").arg(fmt(c.fan_m3h, 0)));
			html += QStringLiteral("<li>%1</li>").arg(tr("Air/air heat exchanger: at least %1 W/K").arg(fmt(c.exchanger_w_k, 1)));
		}
		html += QStringLiteral("<li>%1</li>").arg(tr("Cooling unit: at least %1 W").arg(fmt(c.cooling_w, 0)));
		html += QStringLiteral("</ul>");
		if (c.fan_m3h <= 0)
			html += QStringLiteral("<p>%1</p>").arg(tr("Outside is not cooler than allowed inside: only a cooling unit helps."));
	}
	m_result->setText(html);
}

void PanelClimateDialog::accept()
{
	auto *d = m_plate ? qobject_cast<Diagram *>(m_plate->scene()) : nullptr;
		// a project open read-only keeps what it has
	if (m_plate && !(d && d->project() && d->project()->isReadOnly())) {
		auto *macro = new QUndoCommand(tr("climate calculation of %1").arg(m_base.name));
		PanelData data = m_plate->panelData();
		const PanelClimateSettings s = settings();
		bool changed = false;
		if (!(data.climate == s)) {
			data.climate = s;
			new ChangePanelDataCommand(m_plate, data, macro);
			changed = true;
		}
			// a power loss typed in: kept by the device on the layout
		for (int i = 0; i < m_base.devices.size() && i < m_table->rowCount(); ++i) {
			const PanelClimate::Device &dev = m_base.devices.at(i);
			const double typed = qMax(0.0, number(m_table->item(i, 2)->text()));
			if (!dev.element || qAbs(typed - dev.watts) < 1e-9) continue;
			const DiagramContext old_info = dev.element->elementInformations();
			DiagramContext new_info = old_info;
			new_info.addValue(QStringLiteral("power_loss_w"), typed > 0 ? fmt(typed, 2) : QString(), false);
			new ChangeElementInformationCommand(dev.element, old_info, new_info, macro);
			changed = true;
		}
		if (changed && d) d->undoStack().push(macro);
		else if (changed) {macro->redo(); delete macro;}
		else delete macro;
	}
	QDialog::accept();
}
