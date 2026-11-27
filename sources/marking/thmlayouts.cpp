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
#include "thmlayouts.h"

#include <QCoreApplication>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>

#include <algorithm>
#include <cmath>

namespace {

	/// A value the maker's file keeps as text or number
double num(const QJsonValue &v, double fallback = 0)
{
	if (v.isDouble()) return v.toDouble();
	bool ok = false;
	const double d = v.toString().trimmed().toDouble(&ok);
	return ok ? d : fallback;
}

	/// micrometres -> millimetres, to the micrometre
double mm(double um)
{
	return std::round(um) / 1000.0;
}

	/// The value of a property: {"value": "..."}
double prop(const QJsonObject &properties, const char *name)
{
	return num(properties.value(QLatin1String(name)).toObject().value(QLatin1String("value")));
}

bool mentions(const QJsonValue &v, const QLatin1String &word)
{
	const QByteArray text = v.isObject() ? QJsonDocument(v.toObject()).toJson(QJsonDocument::Compact)
							: v.isArray() ? QJsonDocument(v.toArray()).toJson(QJsonDocument::Compact)
							: v.toString().toUtf8();
	return text.contains(word.data());
}

void readLayout(const QJsonObject &l, ThmLayouts::Variant *v)
{
	const QJsonObject page = l.value(QLatin1String("page")).toObject();
	v->page_width = mm(num(page.value(QLatin1String("width_um"))));
	v->page_height = mm(num(page.value(QLatin1String("height_um"))));
	const QJsonValue pitched = l.value(QLatin1String("pitched_strip"));
	v->pitched_strip = pitched.isBool() ? pitched.toBool() : pitched.toString() == QLatin1String("true");
	v->support_plate = l.value(QLatin1String("support_plate")).toString();
	if (mentions(l.value(QLatin1String("layout_rules_raw")), QLatin1String("multi_rows"))
			|| mentions(l.value(QLatin1String("page_rules_raw")), QLatin1String("multi_rows"))) {
		v->refusal = QCoreApplication::translate("ThmLayouts", "Continuous material with layout rules (multi_rows) is not printed yet.");
		return;
	}

	struct Marker { double x, y, w, h; };
	QList<Marker> markers;
	for (const QJsonValue &m : l.value(QLatin1String("markers")).toArray()) {
		const QJsonObject o = m.toObject();
		markers << Marker{mm(num(o.value(QLatin1String("x_um")))), mm(num(o.value(QLatin1String("y_um")))),
						  mm(num(o.value(QLatin1String("width_um")))), mm(num(o.value(QLatin1String("height_um"))))};
	}
	std::sort(markers.begin(), markers.end(), [](const Marker &a, const Marker &b) { return a.x < b.x; });
	if (markers.isEmpty()) {
		v->refusal = QCoreApplication::translate("ThmLayouts", "The layout has no labels.");
		return;
	}
	for (const Marker &m : markers) {
		if (m.y != markers.first().y || m.w != markers.first().w || m.h != markers.first().h) {
			v->refusal = QCoreApplication::translate("ThmLayouts", "The labels of the layout are not in one even row.");
			return;
		}
	}
	v->columns = markers.size();
	v->first_x = markers.first().x;
	v->column_pitch = markers.size() > 1 ? markers.at(1).x - markers.first().x : 0;
	for (int i = 2; i < markers.size(); ++i) {
		if (std::abs(markers.at(i).x - markers.at(i - 1).x - v->column_pitch) > 0.001) {
			v->refusal = QCoreApplication::translate("ThmLayouts", "The labels of the layout are not evenly spaced.");
			return;
		}
	}
	v->marker_y = markers.first().y;
	v->marker_width = markers.first().w;
	v->marker_height = markers.first().h;

		//The text field: the one called MARKER (or Marker)
	bool found = false;
	for (const QJsonValue &f : l.value(QLatin1String("fields")).toArray()) {
		const QJsonObject o = f.toObject();
		if (o.value(QLatin1String("name")).toString().compare(QLatin1String("MARKER"), Qt::CaseInsensitive) != 0)
			continue;
		const QJsonObject p = o.value(QLatin1String("properties")).toObject();
		v->field_x0 = mm(prop(p, "xstart"));
		v->field_y0 = mm(prop(p, "ystart"));
		v->field_x1 = mm(prop(p, "xend"));
		v->field_y1 = mm(prop(p, "yend"));
		v->orientation = int(prop(p, "orientation"));
		v->font_points = num(p.value(QLatin1String("font")).toObject().value(QLatin1String("height")));
		found = true;
		break;
	}
	if (!found || v->field_x1 <= v->field_x0 || v->field_y1 <= v->field_y0)
		v->refusal = QCoreApplication::translate("ThmLayouts", "The layout has no text field for the marker.");
	else if (v->pitched_strip && v->columns > 1)
		v->refusal = QCoreApplication::translate("ThmLayouts", "Strips with several labels side by side are not printed yet.");
}

void readParameters(const QJsonObject &p, ThmLayouts::Variant *v)
{
	const auto text = [&p](const char *key) { return p.value(QLatin1String(key)).toString().trimmed(); };
	v->sensor = text("sensor_type_muma").isEmpty() ? text("sensor_type") : text("sensor_type_muma");
	v->print_mode = text("print_mode");
	v->position_y = mm(num(p.value(QLatin1String("print_position_mmp"))));
	v->speed_multimark = int(num(p.value(QLatin1String("print_speed_muma"))));
	v->speed_plus = int(num(p.value(QLatin1String("print_speed_muma_plus"))));
	v->darkness = int(num(p.value(QLatin1String("print_darkness_muma_plus"))));
	v->rotate = text("rotate_180_degrees") == QLatin1String("on");
	v->ribbon = text("ribbon_muma_plus");
	if (!v->refusal.isEmpty()) return;
	if (v->sensor == QLatin1String("continuous"))
		v->refusal = QCoreApplication::translate("ThmLayouts", "Continuous material is not printed yet.");
	else if (!v->sensor.isEmpty() && v->sensor != QLatin1String("gap"))
		v->refusal = QCoreApplication::translate("ThmLayouts", "The label sensor \"%1\" is not known.").arg(v->sensor);
	else if (text("cut_mode") == QLatin1String("on"))
		v->refusal = QCoreApplication::translate("ThmLayouts", "Cutting is not supported yet.");
}

}

QList<ThmLayouts::Variant> ThmLayouts::parse(const QByteArray &json, QString *error)
{
	QJsonParseError parse_error;
	const QJsonDocument doc = QJsonDocument::fromJson(json, &parse_error);
	const QJsonObject root = doc.object();
	if (!doc.isObject() || !root.value(QLatin1String("variants")).isArray()
			|| !root.value(QLatin1String("layouts")).isObject()) {
		if (error) *error = parse_error.error != QJsonParseError::NoError
				? QCoreApplication::translate("ThmLayouts", "The layout file cannot be read: %1").arg(parse_error.errorString())
				: QCoreApplication::translate("ThmLayouts", "The file holds no \"layouts\" and \"variants\".");
		return {};
	}
	const QJsonObject layouts = root.value(QLatin1String("layouts")).toObject();
	QList<Variant> out;
	for (const QJsonValue &value : root.value(QLatin1String("variants")).toArray()) {
		const QJsonObject o = value.toObject();
		Variant v;
		v.article = o.value(QLatin1String("article")).toString().trimmed();
		v.name = o.value(QLatin1String("name")).toString().trimmed();
		v.category = o.value(QLatin1String("category")).toString();
		v.layout = o.value(QLatin1String("layout")).toString();
		if (v.article.isEmpty()) continue;
		const QJsonValue layout = layouts.value(v.layout);
		if (layout.isObject()) readLayout(layout.toObject(), &v);
		else v.refusal = QCoreApplication::translate("ThmLayouts", "The layout %1 is not in the file.").arg(v.layout);
		readParameters(o.value(QLatin1String("printer_parameters_raw")).toObject(), &v);
		out << v;
	}
	if (out.isEmpty() && error) *error = QCoreApplication::translate("ThmLayouts", "The layout file holds no variants.");
	else if (error) error->clear();
	return out;
}

QList<ThmLayouts::Variant> ThmLayouts::read(const QString &path, QString *error)
{
	QFile file(path);
	if (!file.open(QIODevice::ReadOnly)) {
		if (error) *error = QCoreApplication::translate("ThmLayouts", "Cannot open %1: %2").arg(path, file.errorString());
		return {};
	}
	return parse(file.readAll(), error);
}

QList<int> ThmLayouts::find(const QList<Variant> &all, const QString &article_or_key)
{
	QString want = article_or_key.trimmed();
	want.remove(QLatin1Char(' '));
	QList<int> found;
	if (want.isEmpty()) return found;
	for (int i = 0; i < all.size(); ++i) {
		if (all.at(i).key().compare(want, Qt::CaseInsensitive) == 0) return {i};
	}
	for (int i = 0; i < all.size(); ++i) {
		if (all.at(i).article.compare(want, Qt::CaseInsensitive) == 0) found << i;
	}
	return found;
}

int ThmLayouts::best(const QList<Variant> &all, const QString &article_or_key)
{
	const QList<int> found = find(all, article_or_key);
	for (int i : found) {
		if (all.at(i).refusal.isEmpty()) return i;
	}
	return found.isEmpty() ? -1 : found.first();
}

QString ThmLayouts::title(const Variant &v)
{
	return QStringLiteral("%1 – %2").arg(v.name, v.article);
}

bool ThmLayouts::profile(const Variant &v, ThmJob::Family family, ThmJob::Profile *out, QString *error)
{
	if (!v.refusal.isEmpty()) {
		if (error) *error = QCoreApplication::translate("ThmLayouts", "%1: %2").arg(title(v), v.refusal);
		return false;
	}
	ThmJob::Profile p;
	p.family = family;
	p.sensor = ThmJob::Sensor::Gap;
	p.gap = 0;   //the layout's page is one pitch: label and gap together
	p.offset_y = v.position_y;
	if (v.pitched_strip) {
			//The maker's software writes a strip with its page axes turned:
			//the page width runs along the transport (its own .prn output)
		p.width = v.marker_height;
		p.height = v.page_width;
		p.page_width = v.page_height;
		p.first_x = v.marker_y;
		p.margin_left = v.field_y0;
		p.margin_right = v.marker_height - v.field_y1;
		p.field_top = v.marker_y + v.field_x0;
		p.field_bottom = v.marker_y + v.field_x1;
		p.marker_top = v.marker_y;
		p.marker_height = v.marker_width;
	} else {
		p.width = v.marker_width;
		p.height = v.page_height;
		p.page_width = v.page_width;
		p.columns = qMax(1, v.columns);
		p.column_pitch = v.columns > 1 ? v.column_pitch : 0;
		p.first_x = v.first_x;
		p.margin_left = v.field_x0;
		p.margin_right = v.marker_width - v.field_x1;
		p.field_top = v.marker_y + v.field_y0;
		p.field_bottom = v.marker_y + v.field_y1;
		p.marker_top = v.marker_y;
		p.marker_height = v.marker_height;
	}
	p.text_rotation = ((v.orientation % 360) + 360) % 360;
		//The font height of the layout (points) as text size, kept inside
		//the field across the text
	const double across = (p.text_rotation == 90 || p.text_rotation == 270)
			? p.width - p.margin_left - p.margin_right
			: (p.field_bottom > 0 ? p.field_bottom : p.height) - p.field_top;
	double size = v.font_points > 0 ? v.font_points * 25.4 / 72 : across * 0.6;
	size = qMin(size, across * 0.9);
	p.text_size = std::round(size * 100) / 100;
	p.set_heat = true;
	p.speed = family == ThmJob::Family::MultiMark
			? (v.speed_multimark ? v.speed_multimark : v.speed_plus)
			: (v.speed_plus ? v.speed_plus : v.speed_multimark);
	p.heat = v.darkness;
	p.transfer = true;
	const bool tear_off = v.print_mode == QLatin1String("tear_off");
	p.heat_options = tear_off ? QStringLiteral("B30") : QString();
	p.print_options = tear_off ? QStringList{QStringLiteral("T"), QStringLiteral("P")}
							   : QStringList{QStringLiteral("P")};
	p.rotate = v.rotate;
	p.plain_zero = true;
	QString why;
	if (!ThmJob::validate(p, &why)) {
		if (error) *error = QCoreApplication::translate("ThmLayouts", "%1: %2").arg(title(v), why);
		return false;
	}
	if (out) *out = p;
	if (error) error->clear();
	return true;
}

bool ThmLayouts::jobProfile(const QList<Variant> &layouts, const QString &article,
							const ThmPrinter *printer, ThmJob::Profile *out,
							QString *source, QString *error)
{
	const int i = best(layouts, article);
	if (i >= 0) {
		const ThmJob::Family family = printer ? printer->family() : ThmJob::Family::Mmp;
		if (!profile(layouts.at(i), family, out, error)) return false;
		if (source) *source = QCoreApplication::translate("ThmLayouts", "Layout %1 of %2 (%3)").arg(layouts.at(i).layout, title(layouts.at(i)),
														   ThmJob::familyKey(family));
		return true;
	}
	if (printer) {
		QString why;
		if (!ThmJob::validate(printer->profile, &why)) {
			if (error) *error = QCoreApplication::translate("ThmLayouts", "No layout for article %1, and the own profile of \"%2\": %3")
								.arg(article.trimmed(), printer->name, why);
			return false;
		}
		if (out) *out = printer->profile;
		if (source) *source = QCoreApplication::translate("ThmLayouts", "Own profile of \"%1\"").arg(printer->name);
		return true;
	}
	if (error) *error = QCoreApplication::translate("ThmLayouts", "No layout for article %1 and no printer profile.").arg(article.trimmed());
	return false;
}

const QList<ThmLayouts::Variant> &ThmLayouts::settingsLayouts(QString *path, QString *error)
{
	static QString cached_path;
	static QString cached_error;
	static QList<Variant> cached;
	QSettings settings;
	const QString file = ThmPrinters::layoutsFile(settings);
	if (file != cached_path) {
		cached_path = file;
		cached_error.clear();
		cached = file.isEmpty() ? QList<Variant>() : read(file, &cached_error);
	}
	if (path) *path = cached_path;
	if (error) *error = cached_error;
	return cached;
}
