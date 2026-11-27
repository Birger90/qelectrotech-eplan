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
#include "fluidbom.h"

#include "../bomexport.h"

#include <QCoreApplication>
#include <QHash>
#include <QPair>
#include <QRegularExpression>
#include <QStringList>

namespace
{
		/// The words of the list, translated in the context "FluidBom"
	struct T
	{
		Q_DECLARE_TR_FUNCTIONS(FluidBom)
	};

	QString number(double d)
	{
		return QString::number(d, 'g', 6);
	}

		/// manual, solenoid... as words for the list
	QString actuationText(const FluidData::Actuation &a)
	{
		QStringList words;
		for (const QString &type : a.types) {
			if (type == QLatin1String("solenoid"))     words << T::tr("solenoid");
			else if (type == QLatin1String("manual"))  words << T::tr("manual");
			else if (type == QLatin1String("button"))  words << T::tr("push button");
			else if (type == QLatin1String("lever"))   words << T::tr("lever");
			else if (type == QLatin1String("pedal"))   words << T::tr("pedal");
			else if (type == QLatin1String("detent"))  words << T::tr("detent");
			else if (type == QLatin1String("plunger")) words << T::tr("plunger");
			else if (type == QLatin1String("roller"))  words << T::tr("roller");
			else if (type == QLatin1String("idle_roller")) words << T::tr("idle roller");
			else if (type == QLatin1String("pneumatic"))   words << T::tr("pneumatic");
			else words << type;
		}
		if (a.pilot)      words << T::tr("pilot");
		if (a.spring)     words << T::tr("spring");
		if (a.air_spring) words << T::tr("air spring");
		return words.join(QStringLiteral(", "));
	}

	QString lineKey(int kind, const QString &section)
	{
		return QString::number(kind) + QLatin1Char('|') + section;
	}

		/// What the lines of one kind and size add up to
	struct LineSum {
		int count = 0;
		double mm = 0;           ///< sum of the lengths that could be read
		int without = 0;         ///< lines with no length typed in
		QStringList unreadable;  ///< lengths typed in but not understood
	};

		/// The size, then the length: summed, partly summed, or to be
		/// measured when no line of this kind and size has one
	QString lineCharacteristics(const QString &section, const LineSum &sum)
	{
		QStringList words;
		words << (section.isEmpty() ? T::tr("size not given")
					    : T::tr("size %1").arg(section));
		const int known = sum.count - sum.without - sum.unreadable.size();
		if (known == 0) {
			words << T::tr("length to be measured");
		} else {
				// To the millimetre, in metres: 3200 mm is "3.2 m"
			const double metres = qRound64(sum.mm) / 1000.0;
			words << T::tr("total %1 m")
				 .arg(QString::number(metres, 'g', 12));
			if (sum.without > 0)
				words << T::tr("%n line(s) without length", nullptr,
					       sum.without);
		}
		if (!sum.unreadable.isEmpty())
			words << T::tr("%n length(s) not understood: %1", nullptr,
				       sum.unreadable.size())
					 .arg(sum.unreadable.join(QStringLiteral(", ")));
		return words.join(QStringLiteral("; "));
	}
}

bool FluidBom::lengthInMm(const QString &text, double *mm)
{
	static const QRegularExpression rx(
		QStringLiteral("^\\s*(\\d+(?:[.,]\\d+)?)\\s*(mm|cm|m)?\\s*$"),
		QRegularExpression::CaseInsensitiveOption);
	const QRegularExpressionMatch match = rx.match(text);
	if (!match.hasMatch())
		return false;
	bool ok = false;
	const double value = match.captured(1)
			.replace(QLatin1Char(','), QLatin1Char('.')).toDouble(&ok);
	if (!ok)
		return false;
	const QString unit = match.captured(2).toLower();
	const double factor = unit == QLatin1String("mm") ? 1.0
			    : unit == QLatin1String("cm") ? 10.0
			    : 1000.0;   // "m", or no unit
	if (mm)
		*mm = value * factor;
	return true;
}

QString FluidBom::kindText(FluidData::Kind kind)
{
	switch (kind) {
		case FluidData::Kind::None:        return QString();
		case FluidData::Kind::Valve:       return T::tr("Directional valve");
		case FluidData::Kind::Cylinder:    return T::tr("Cylinder");
		case FluidData::Kind::Source:      return T::tr("Supply");
		case FluidData::Kind::Treatment:   return T::tr("Air treatment");
		case FluidData::Kind::FlowControl: return T::tr("Flow or shut-off valve");
		case FluidData::Kind::LogicValve:  return T::tr("Logic valve");
		case FluidData::Kind::Sensor:      return T::tr("Sensor or gauge");
		case FluidData::Kind::Other:       return T::tr("Other fluid part");
		case FluidData::Kind::Solenoid:    return T::tr("Valve solenoid");
	}
	return QString();
}

QString FluidBom::lineKindText(Fluid::LineKind kind)
{
	switch (kind) {
		case Fluid::LineKind::None:    return QString();
		case Fluid::LineKind::Working: return T::tr("Working line");
		case Fluid::LineKind::Pilot:   return T::tr("Pilot line");
		case Fluid::LineKind::Exhaust: return T::tr("Exhaust line");
	}
	return QString();
}

/**
	@return what tells this part apart in a parts list: the positions and
	actuation of a valve, bore, rod, stroke and action of a cylinder.
	Empty for the other kinds, whose fluid data carries no figures.
*/
QString FluidBom::details(const FluidData &data)
{
	QStringList parts;
	if (data.m_kind == FluidData::Kind::Valve) {
		parts << T::tr("%n position(s)", nullptr, data.m_positions.size());
		const QString left = actuationText(data.m_left);
		const QString right = actuationText(data.m_right);
		if (!left.isEmpty())
			parts << T::tr("left: %1").arg(left);
		if (!right.isEmpty())
			parts << T::tr("right: %1").arg(right);
	}
	else if (data.m_kind == FluidData::Kind::Cylinder) {
		parts << (data.m_action == FluidData::CylinderAction::Single
				  ? T::tr("single acting") : T::tr("double acting"));
		if (data.m_bore_mm > 0)
			parts << T::tr("bore %1 mm").arg(number(data.m_bore_mm));
		if (data.m_rod_mm > 0)
			parts << T::tr("rod %1 mm").arg(number(data.m_rod_mm));
		if (data.m_stroke_mm > 0)
			parts << T::tr("stroke %1 mm").arg(number(data.m_stroke_mm));
		if (data.m_spring == FluidData::SpringSide::Rod)
			parts << T::tr("spring on the rod side");
		else if (data.m_spring == FluidData::SpringSide::Piston)
			parts << T::tr("spring on the piston side");
	}
	return parts.join(QStringLiteral("; "));
}

QByteArray FluidBom::csv(const QList<Part> &parts, const QList<Line> &lines,
						 int *rows)
{
	QByteArray out = BomExport::csvRecord({
		T::tr("Item"), T::tr("Quantity"), T::tr("Unit"), T::tr("Name"),
		T::tr("Kind"), T::tr("Characteristics"), T::tr("Manufacturer"),
		T::tr("Order number"), T::tr("Labels")});
	int item = 0;

		// Identical parts on one line, in the order they first appear
	QList<QStringList> keys;
	QHash<QStringList, QStringList> labels;
	QHash<QStringList, int> counts;
	for (const Part &p : parts) {
		const QStringList key {p.name, p.kind, p.details, p.manufacturer,
							   p.reference};
		if (!counts.contains(key))
			keys << key;
		++counts[key];
		if (!p.label.isEmpty())
			labels[key] << p.label;
	}
	for (const QStringList &key : keys) {
		out += BomExport::csvRecord({
			QString::number(++item), QString::number(counts.value(key)),
			T::tr("pcs"), key.at(0), key.at(1), key.at(2), key.at(3),
			key.at(4), labels.value(key).join(QStringLiteral(", "))});
	}

		// Lines by kind and size: counted, their lengths added up
	QList<QPair<int, QString>> line_keys;
	QHash<QString, LineSum> line_sums;
	for (const Line &l : lines) {
		if (l.kind == Fluid::LineKind::None)
			continue;
		const int kind = static_cast<int>(l.kind);
		const QString key = lineKey(kind, l.section);
		if (!line_sums.contains(key))
			line_keys << qMakePair(kind, l.section);
		LineSum &sum = line_sums[key];
		++sum.count;
		double mm = 0;
		const QString length = l.length.trimmed();
		if (length.isEmpty())
			++sum.without;
		else if (lengthInMm(length, &mm))
			sum.mm += mm;
		else
			sum.unreadable << length;
	}
	for (const auto &k : line_keys) {
		const LineSum sum = line_sums.value(lineKey(k.first, k.second));
		out += BomExport::csvRecord({
			QString::number(++item),
			QString::number(sum.count),
			T::tr("line(s)"),
			lineKindText(static_cast<Fluid::LineKind>(k.first)),
			T::tr("Hose or pipe"), lineCharacteristics(k.second, sum),
			QString(), QString(), QString()});
	}
	if (rows)
		*rows = item;
	return out;
}
