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
#include "thmjob.h"

#include <QCoreApplication>
#include <QHash>
#include <QtMath>

namespace {

	/// Line end of every command: the printers take CR, LF or CR/LF
	/// (manual p. 7-8)
const char *cr = "\r";

bool isAscii(const QString &text)
{
	for (const QChar c : text) {
		if (c.unicode() > 0x7E) return false;
	}
	return true;
}

	/// One row of labels: the texts side by side, and how often the row
struct Row
{
	QStringList texts;
	int count = 1;
};

	/// The columns of the profile filled label by label, one row after
	/// the other; equal rows in a row are one row printed several times
QList<Row> rows(const QList<ThmJob::Label> &labels, int columns)
{
	QList<Row> out;
	if (columns <= 1) {
		for (const ThmJob::Label &l : labels) {
			if (l.count > 0) out << Row{{l.text}, l.count};
		}
		return out;
	}
	QStringList cells;
	for (const ThmJob::Label &l : labels) {
		for (int n = 0; n < l.count; ++n) cells << l.text;
	}
	for (int i = 0; i < cells.size(); i += columns) {
		const QStringList texts = cells.mid(i, columns);
		if (!out.isEmpty() && out.last().texts == texts) {
			++out.last().count;
		} else {
			out << Row{texts, 1};
		}
	}
	return out;
}

} // namespace

QString ThmJob::number(double value)
{
	QString s = QString::number(value, 'f', 2);
	while (s.contains(QLatin1Char('.')) && (s.endsWith(QLatin1Char('0')) || s.endsWith(QLatin1Char('.')))) {
		s.chop(1);
	}
	if (s == QLatin1String("-0")) s = QStringLiteral("0");
	return s;
}

QList<int> ThmJob::speedSteps()
{
	return speedSteps(Family::Mmp);
}

QList<int> ThmJob::speedSteps(Family family)
{
	if (family == Family::MultiMark) return {30, 40, 50, 75, 100, 125, 150};
	return {30, 40, 50, 75, 100, 125};
}

int ThmJob::heatMin(Family family)
{
	return family == Family::MultiMark ? -20 : kHeatMin;
}

int ThmJob::heatMax(Family)
{
	return kHeatMax;
}

double ThmJob::printWidth(Family family)
{
	return family == Family::MultiMark ? 106.0 : kPrintWidthMm;
}

QString ThmJob::familyKey(Family family)
{
	return family == Family::MultiMark ? QStringLiteral("multimark") : QStringLiteral("mmp");
}

bool ThmJob::familyFromKey(const QString &key, Family *family)
{
	const QString k = key.trimmed().toLower().remove(QLatin1Char(' '));
	Family f;
	if (k == QLatin1String("mmp") || k == QLatin1String("thmmmp")) f = Family::Mmp;
	else if (k == QLatin1String("multimark") || k == QLatin1String("thmmultimark")) f = Family::MultiMark;
	else return false;
	if (family) *family = f;
	return true;
}

ThmJob::Family ThmJob::familyFromModel(const QString &model)
{
	return model.contains(QLatin1String("MultiMark"), Qt::CaseInsensitive)
			&& !model.contains(QLatin1String("MMP"), Qt::CaseInsensitive)
		   ? Family::MultiMark : Family::Mmp;
}

double ThmJob::totalWidth(const Profile &p)
{
	const double pitch = p.columns > 1 && p.column_pitch > 0 ? p.column_pitch : p.width;
	return (qMax(1, p.columns) - 1) * pitch + p.width;
}

double ThmJob::rowWidth(const Profile &p)
{
	return p.page_width > 0 ? p.page_width : p.first_x + totalWidth(p);
}

bool ThmJob::validate(const Profile &p, QString *error)
{
	QString e;
	if (p.width <= 0 || p.height <= 0) {
		e = QCoreApplication::translate("ThmJob", "The label width and height are not set.");
	} else if (p.gap < 0) {
		e = QCoreApplication::translate("ThmJob", "The gap must not be negative.");
	} else if (p.columns < 1) {
		e = QCoreApplication::translate("ThmJob", "At least one column is needed.");
	} else if (p.columns > 1 && p.column_pitch > 0 && p.column_pitch < p.width) {
		e = QCoreApplication::translate("ThmJob", "The column pitch is smaller than the label width.");
	} else if (p.margin_left < 0 || p.margin_right < 0
			   || p.margin_left + p.margin_right >= p.width) {
		e = QCoreApplication::translate("ThmJob", "The margins leave no room for the text.");
	} else if (p.text_size <= 0) {
		e = QCoreApplication::translate("ThmJob", "The text size is not set.");
	} else if (p.baseline < 0 || p.baseline > p.height) {
		e = QCoreApplication::translate("ThmJob", "The text baseline lies outside the label.");
	} else if (p.field_top < 0 || p.field_bottom < 0 || p.field_bottom > p.height + 1e-9
			   || (p.field_bottom > 0 && p.field_bottom <= p.field_top)) {
		e = QCoreApplication::translate("ThmJob", "The text field lies outside the label.");
	} else if (p.text_rotation != 0 && p.text_rotation != 90 && p.text_rotation != 180
			   && p.text_rotation != 270) {
		e = QCoreApplication::translate("ThmJob", "The text can only be turned by 0, 90, 180 or 270 degrees.");
	} else if (p.first_x < 0 || p.page_width < 0
			   || (p.page_width > 0 && p.first_x + totalWidth(p) > p.page_width + 1e-9)) {
		e = QCoreApplication::translate("ThmJob", "The columns do not fit the row width.");
	} else if (p.set_heat && p.speed <= 0) {
		e = QCoreApplication::translate("ThmJob", "The print speed is not set.");
	} else if (p.set_heat && !speedSteps(p.family).contains(p.speed)) {
		QStringList steps;
		for (int s : speedSteps(p.family)) steps << QString::number(s);
		e = QCoreApplication::translate("ThmJob", "This printer only knows the speeds %1 mm/s.")
			.arg(steps.join(QStringLiteral(", ")));
	} else if (p.set_heat && (p.heat < heatMin(p.family) || p.heat > heatMax(p.family))) {
		e = QCoreApplication::translate("ThmJob", "The heat correction must lie between %1 and +%2.")
			.arg(heatMin(p.family)).arg(heatMax(p.family));
	} else if (rowWidth(p) > printWidth(p.family) + 1e-9) {
		e = QCoreApplication::translate("ThmJob", "The labels are wider than the print head (%1 mm).")
			.arg(number(printWidth(p.family)));
	}
	if (error) *error = e;
	return e.isEmpty();
}

QString ThmJob::escapeText(const QString &text)
{
	QString out;
	out.reserve(text.size());
	for (const QChar c : text) {
		const ushort u = c.unicode();
		if (u < 0x20 || u == 0x7F) {
			out += QLatin1Char(' ');
		} else if (c == QLatin1Char('[')) {
			out += QStringLiteral("[U:$5B]");
		} else {
			out += c;
		}
	}
	return out.trimmed();
}

QByteArray ThmJob::build(const QList<Label> &labels, const Profile &p, QString *error)
{
	if (!validate(p, error)) return QByteArray();
	if (labelCount(labels) <= 0) {
		if (error) *error = QCoreApplication::translate("ThmJob", "There is nothing to print.");
		return QByteArray();
	}
	if (error) error->clear();

	const double pitch = p.columns > 1 && p.column_pitch > 0 ? p.column_pitch : p.width;
		//The text field of a label: across from the margins, along the
		//transport from field_top to field_bottom
	const double field_left = p.margin_left;
	const double field_right = p.width - p.margin_right;
	const double field_top = p.field_top;
	const double field_bottom = p.field_bottom > 0 ? p.field_bottom : p.height;
	const double centre_y = (field_top + field_bottom) / 2;

		//S [ptype;]xo,yo,ho,dy,wd (manual p. 220-221): dy is the label
		//height plus the gap. Several columns are one label of the whole
		//width with one text field per column, so only the documented
		//meaning of S and T is used, not dx/col. M-Print PRO writes its
		//THM jobs the same way (one S over the whole row).
	const QString size_line = QStringLiteral("S %1;%2,%3,%4,%5,%6")
			.arg(sensorKey(p.sensor), number(p.offset_x), number(p.offset_y),
				 number(p.height), number(p.height + p.gap), number(rowWidth(p)));
		//H speed[,h][,t] (manual p. 196): T thermal transfer, D direct;
		//further options as the maker's software writes them
	QString heat_line = QStringLiteral("H %1,%2,%3")
			.arg(p.speed).arg(p.heat).arg(p.transfer ? QStringLiteral("T") : QStringLiteral("D"));
	if (!p.heat_options.isEmpty()) heat_line += QLatin1Char(',') + p.heat_options;
	QStringList options = p.print_options;
	if (p.rotate && !options.contains(QStringLiteral("R"))) options.prepend(QStringLiteral("R"));

		//T x,y,r,...: x,y is where the text's baseline begins; [J:cW]
		//centres it over W in its own direction. Turned by r degrees
		//anticlockwise, the start moves to the matching side of the field.
	struct TextPlace { double x, y, width; };
	const auto place = [&](int column) {
		const double left = p.first_x + column * pitch;
		const double centre_x = left + (field_left + field_right) / 2;
		switch (p.text_rotation) {
			case 90:
				return TextPlace{centre_x + p.text_size / 2, field_bottom, field_bottom - field_top};
			case 180:
				return TextPlace{left + field_right, centre_y - p.text_size / 2, field_right - field_left};
			case 270:
				return TextPlace{centre_x - p.text_size / 2, field_top, field_bottom - field_top};
			default:
				return TextPlace{left + field_left,
								 p.baseline > 0 ? p.baseline : centre_y + p.text_size / 2,
								 field_right - field_left};
		}
	};

	QByteArray out;
	for (const Row &row : rows(labels, p.columns)) {
		QStringList texts;
		for (const QString &t : row.texts) texts << escapeText(t);
		const bool ascii = isAscii(texts.join(QString()));
			//ESCo<codepage>; before the label data (manual p. 25)
		if (!ascii) out += "\x1boUTF-8;";
		QStringList lines;
		lines << QStringLiteral("m m");      //millimetres (manual p. 53)
		if (p.plain_zero) lines << QStringLiteral("zO");  //zero without slash
		lines << QStringLiteral("J");        //job start (manual p. 202)
		if (p.set_heat) lines << heat_line;
		lines << size_line;
			//O directly after S (manual p. 212); R turns 180 degrees (p. 213)
		if (!options.isEmpty()) lines << QStringLiteral("O ") + options.join(QLatin1Char(','));
		for (int c = 0; c < texts.size(); ++c) {
			if (texts.at(c).isEmpty()) continue;
				//T x,y,r,font,size;text (manual p. 224); [J:cW] centres
				//the text in a field W wide from x (manual p. 329)
			const TextPlace t = place(c);
			lines << QStringLiteral("T %1,%2,%3,%4,%5;[J:c%6]%7")
					 .arg(number(t.x), number(t.y))
					 .arg(p.text_rotation)
					 .arg(p.font)
					 .arg(number(p.text_size), number(t.width), texts.at(c));
		}
			//A <num>: end of the label and its amount (manual p. 72)
		lines << QStringLiteral("A %1").arg(row.count);
		for (const QString &l : lines) {
			out += (ascii ? l.toLatin1() : l.toUtf8()) + cr;
		}
	}
	return out;
}

QList<ThmJob::Label> ThmJob::toLabels(const QList<MarkingExport::Item> &items,
									  int copies, bool merge)
{
	QList<Label> labels;
	QHash<QString, int> index;
	copies = qMax(1, copies);
	for (const MarkingExport::Item &item : items) {
		const QString text = item.text.trimmed();
		if (text.isEmpty() || item.count <= 0) continue;
		const int count = item.count * copies;
		if (merge) {
			auto it = index.constFind(text);
			if (it != index.constEnd()) {
				labels[it.value()].count += count;
				continue;
			}
			index.insert(text, labels.size());
		}
		labels << Label{text, count};
	}
	return labels;
}

int ThmJob::labelCount(const QList<Label> &labels)
{
	int n = 0;
	for (const Label &l : labels) n += qMax(0, l.count);
	return n;
}

QString ThmJob::sensorKey(Sensor sensor)
{
	switch (sensor) {
		case Sensor::Endless: return QStringLiteral("e");
		case Sensor::ReflectTop: return QStringLiteral("l0");
		case Sensor::ReflectBottom: return QStringLiteral("l2");
		default: return QStringLiteral("l1");
	}
}

bool ThmJob::sensorFromKey(const QString &key, Sensor *sensor)
{
	const QString k = key.trimmed().toLower();
	Sensor s;
	if (k == QLatin1String("e") || k == QLatin1String("endless")
			|| k == QLatin1String("endlos")) s = Sensor::Endless;
	else if (k == QLatin1String("l1") || k == QLatin1String("gap")
			 || k == QLatin1String("luecke") || k == QString::fromUtf8("lücke")) s = Sensor::Gap;
	else if (k == QLatin1String("l0") || k == QLatin1String("reflect")
			 || k == QLatin1String("reflex")) s = Sensor::ReflectTop;
	else if (k == QLatin1String("l2")) s = Sensor::ReflectBottom;
	else return false;
	if (sensor) *sensor = s;
	return true;
}

QString ThmJob::sensorTitle(Sensor sensor)
{
	switch (sensor) {
		case Sensor::Endless: return QCoreApplication::translate("ThmJob", "Endless material (e)");
		case Sensor::ReflectTop: return QCoreApplication::translate("ThmJob", "Reflective mark, top (l0)");
		case Sensor::ReflectBottom: return QCoreApplication::translate("ThmJob", "Reflective mark, bottom (l2)");
		default: return QCoreApplication::translate("ThmJob", "Labels with gap (l1)");
	}
}

ThmJob::Status ThmJob::parseStatus(const QByteArray &reply)
{
	Status s;
	s.raw = reply.trimmed();
	const QByteArray &r = s.raw;
	if (r.size() != 9) return s;
	if ((r.at(0) != 'Y' && r.at(0) != 'N') || (r.at(8) != 'Y' && r.at(8) != 'N')) return s;
	for (int i = 2; i < 8; ++i) {
		if (r.at(i) < '0' || r.at(i) > '9') return s;
	}
	const char e = r.at(1);
	if (e != '-' && !((e >= 'a' && e <= 'z') || (e >= 'A' && e <= 'Z'))) return s;
	s.online = r.at(0) == 'Y';
	s.error = e;
	s.pending = r.mid(2, 6).toInt();
	s.interpreter_active = r.at(8) == 'Y';
	s.valid = true;
	return s;
}

QString ThmJob::errorText(char code)
{
		//The table of ESC s, manual p. 29-30
	switch (code) {
		case '-': return QCoreApplication::translate("ThmJob", "No error");
		case 'a': return QCoreApplication::translate("ThmJob", "Applicator did not reach the upper position");
		case 'b': return QCoreApplication::translate("ThmJob", "Applicator did not reach the lower position");
		case 'c': return QCoreApplication::translate("ThmJob", "Vacuum plate is empty");
		case 'd': return QCoreApplication::translate("ThmJob", "Label not deposited");
		case 'e': return QCoreApplication::translate("ThmJob", "Host stop/error");
		case 'f': return QCoreApplication::translate("ThmJob", "Reflective sensor blocked");
		case 'g': return QCoreApplication::translate("ThmJob", "Tamp pad 90° error");
		case 'h': return QCoreApplication::translate("ThmJob", "Tamp pad 0° error");
		case 'i': return QCoreApplication::translate("ThmJob", "Table not in front position");
		case 'j': return QCoreApplication::translate("ThmJob", "Table not in rear position");
		case 'k': return QCoreApplication::translate("ThmJob", "Head lifted");
		case 'l': return QCoreApplication::translate("ThmJob", "Head down");
		case 'm': return QCoreApplication::translate("ThmJob", "Scan result negative");
		case 'n': return QCoreApplication::translate("ThmJob", "Global network error");
		case 'o': return QCoreApplication::translate("ThmJob", "Compressed air error");
		case 'r': return QCoreApplication::translate("ThmJob", "RFID error");
		case 's': return QCoreApplication::translate("ThmJob", "System fault");
		case 'u': return QCoreApplication::translate("ThmJob", "USB error");
		case 'x': return QCoreApplication::translate("ThmJob", "Stacker full, printer goes on pause");
		case 'A': return QCoreApplication::translate("ThmJob", "Applicator error");
		case 'B': return QCoreApplication::translate("ThmJob", "Protocol error / invalid barcode data");
		case 'C': return QCoreApplication::translate("ThmJob", "Memory card error");
		case 'D': return QCoreApplication::translate("ThmJob", "Printhead or pinch roller open");
		case 'E': return QCoreApplication::translate("ThmJob", "Synchronization error (no label found)");
		case 'F': return QCoreApplication::translate("ThmJob", "Out of ribbon");
		case 'G': return QCoreApplication::translate("ThmJob", "PPP reload required");
		case 'H': return QCoreApplication::translate("ThmJob", "Heating voltage problem");
		case 'I': return QCoreApplication::translate("ThmJob", "Cutter jammed");
		case 'N': return QCoreApplication::translate("ThmJob", "Label material too thick (cutter)");
		case 'O': return QCoreApplication::translate("ThmJob", "Out of memory");
		case 'P': return QCoreApplication::translate("ThmJob", "Out of paper");
		case 'R': return QCoreApplication::translate("ThmJob", "Ribbon detected in thermal direct mode");
		case 'S': return QCoreApplication::translate("ThmJob", "Ribbon saver malfunction");
		case 'U': return QCoreApplication::translate("ThmJob", "User error");
		case 'V': return QCoreApplication::translate("ThmJob", "Input buffer overflow");
		case 'W': return QCoreApplication::translate("ThmJob", "Print head overheated");
		case 'X': return QCoreApplication::translate("ThmJob", "External I/O error");
		case 'Y': return QCoreApplication::translate("ThmJob", "Printhead error");
		case 'Z': return QCoreApplication::translate("ThmJob", "Printhead damaged");
		default: return QCoreApplication::translate("ThmJob", "Error code %1 (not in the manual's table)").arg(QLatin1Char(code));
	}
}

QString ThmJob::statusText(const Status &s)
{
	if (!s.valid) {
		return QCoreApplication::translate("ThmJob", "Status reply not understood: \"%1\"").arg(QString::fromLatin1(s.raw));
	}
	QString text = s.online ? QCoreApplication::translate("ThmJob", "online") : QCoreApplication::translate("ThmJob", "offline");
	text += QStringLiteral(", ") + errorText(s.error);
	text += QStringLiteral(", ") + QCoreApplication::translate("ThmJob", "%n label(s) to print", nullptr, s.pending);
	text += QStringLiteral(", ") + (s.interpreter_active ? QCoreApplication::translate("ThmJob", "interpreter active")
														 : QCoreApplication::translate("ThmJob", "standby"));
	return text;
}

ThmJob::Material ThmJob::parseMaterial(const QByteArray &reply)
{
	Material m;
	m.raw = reply.trimmed();
	const QByteArray &r = m.raw;
	if (r.size() != 5 || (r.at(0) != 'Y' && r.at(0) != 'N')) return m;
	for (int i = 1; i < 5; ++i) {
		if (r.at(i) < '0' || r.at(i) > '9') return m;
	}
	m.synchronized = r.at(0) == 'Y';
	m.pitch = r.mid(1).toInt();
	m.valid = true;
	return m;
}

QString ThmJob::materialText(const Material &m)
{
	if (!m.valid) {
		return QCoreApplication::translate("ThmJob", "Material reply not understood: \"%1\"").arg(QString::fromLatin1(m.raw));
	}
	const QString sync = m.synchronized ? QCoreApplication::translate("ThmJob", "synchronized") : QCoreApplication::translate("ThmJob", "not synchronized");
	if (m.pitch <= 0) return QCoreApplication::translate("ThmJob", "%1, label distance unknown").arg(sync);
	return QCoreApplication::translate("ThmJob", "%1, label distance %2 mm").arg(sync).arg(m.pitch);
}

ThmJob::Fit ThmJob::materialFit(const Material &m, const Profile &p)
{
	if (!m.valid || m.pitch <= 0 || p.height <= 0) return Fit::Unknown;
	return qAbs(m.pitch - (p.height + p.gap)) <= 1.0 ? Fit::Fits : Fit::Differs;
}

QString ThmJob::parseVersion(const QByteArray &reply)
{
	return QString::fromLatin1(reply).trimmed();
}
