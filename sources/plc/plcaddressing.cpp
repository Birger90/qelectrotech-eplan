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
// SPDX-License-Identifier: GPL-2.0-or-later
#include "plcaddressing.h"

#include "../diagram.h"
#include "../diagramcontext.h"
#include "../qetgraphicsitem/element.h"
#include "../qetproject.h"
#include "../undocommand/changeelementdatacommand.h"

#include <QCoreApplication>
#include <QRegularExpression>
#include <QSet>
#include <QUndoCommand>
#include <algorithm>

namespace {

const QString station_info = QStringLiteral("plc_station");

	/// The project property of @p what of @p station: a DiagramContext key
	/// takes a-z, 0-9, - and _ only, so the station name is written so
QString propertyKey(const QString &what, const QString &station)
{
	QString name;
	for (const QChar ch : station.toLower())
		name += (ch.isLetterOrNumber() && ch.unicode() < 128) || ch == QLatin1Char('-') ? ch : QLatin1Char('_');
	return QStringLiteral("plc_station_%1__%2").arg(what, name);
}

bool isPlcModule(const Element *e)
{
	return e && e->elementData().m_type == ElementData::Master
			&& e->elementData().m_master_type == ElementData::PLC;
}

bool isInput(ElementData::PlcIOType t)
{
	return t == ElementData::EntreeDigitale || t == ElementData::EntreeAnalogique
			|| t == ElementData::EntreeUniverselle;
}

bool isAnalog(ElementData::PlcIOType t)
{
	return t == ElementData::EntreeAnalogique || t == ElementData::SortieAnalogique;
}

	/// "IOL:2/2", "IOL 4" -- or an IO-Link range written before,
	/// "IB4…IB5 / QB2…QB3": the process data bytes in and out
bool ioLinkBytes(const QString &raw, int *in, int *out)
{
	const QString s = raw.trimmed();
	static const QRegularExpression spec(QStringLiteral("^IOL\\s*:?\\s*(\\d+)(?:\\s*/\\s*(\\d+))?$"),
										 QRegularExpression::CaseInsensitiveOption);
	const auto m = spec.match(s);
	if (m.hasMatch()) {
		*in = m.captured(1).toInt();
		*out = m.capturedLength(2) ? m.captured(2).toInt() : 0;
		return true;
	}
	static const QRegularExpression range(
			QStringLiteral("%?([IQEA])B(\\d+)(?:\\s*(?:…|\\.\\.)\\s*%?[IQEA]B(\\d+))?"),
			QRegularExpression::CaseInsensitiveOption);
	*in = *out = 0;
		//TwinCAT: "%I* [2 B] / %Q* [2 B]"
	static const QRegularExpression linked(QStringLiteral("%([IQ])\\*\\s*\\[(\\d+)\\s*B\\]"));
	auto lt = linked.globalMatch(s);
	bool linked_any = false;
	while (lt.hasNext()) {
		const auto r = lt.next();
		(r.captured(1) == QLatin1String("I") ? *in : *out) = r.captured(2).toInt();
		linked_any = true;
	}
	if (linked_any) return true;
		//Omron: "IN 0…1 [4 B] / OUT 100 [2 B]", channels with the byte count
	static const QRegularExpression omron(
			QStringLiteral("\\b(IN|OUT)\\s+\\d+(?:\\s*(?:…|\\.\\.)\\s*\\d+)?\\s*\\[(\\d+)\\s*B\\]"),
			QRegularExpression::CaseInsensitiveOption);
	auto ot = omron.globalMatch(s);
	bool omron_any = false;
	while (ot.hasNext()) {
		const auto r = ot.next();
		(r.captured(1).compare(QLatin1String("IN"), Qt::CaseInsensitive) == 0 ? *in : *out)
				= r.captured(2).toInt();
		omron_any = true;
	}
	if (omron_any) return true;
	if (!s.contains(QLatin1Char('B'), Qt::CaseInsensitive)) return false;
	bool any = false;
	auto it = range.globalMatch(s);
	while (it.hasNext()) {
		const auto r = it.next();
		const QChar a = r.captured(1).toUpper().at(0);
		const int first = r.captured(2).toInt();
		const int last = r.capturedLength(3) ? r.captured(3).toInt() : first;
		const int n = last - first + 1;
		if (n <= 0) continue;
		if (a == QLatin1Char('I') || a == QLatin1Char('E')) *in = n; else *out = n;
		any = true;
	}
	return any;
}

bool isIoLinkSpec(const QString &raw)
{
	return raw.trimmed().startsWith(QLatin1String("IOL"), Qt::CaseInsensitive);
}

	/// whether @p raw is an absolute address of @p format already
bool isAbsolute(const QString &raw, PlcAddressing::Format format)
{
	const QString s = raw.trimmed();
	if (s.isEmpty()) return false;
	switch (format) {
		case PlcAddressing::Format::TwinCat:
			return s == QLatin1String("%I*") || s == QLatin1String("%Q*");
		case PlcAddressing::Format::Omron: {
			static const QRegularExpression rx(QStringLiteral("^\\d{1,4}(?:\\.(?:0\\d|1[0-5]))?$"));
			return rx.match(s).hasMatch();
		}
		default: {
			static const QRegularExpression rx(
				QStringLiteral("^%?[IQEA][XBWD]?\\d{1,5}(?:\\.[0-7])?$"),
				QRegularExpression::CaseInsensitiveOption);
			if (!rx.match(s).hasMatch()) return false;
				//"E0" or "A3" alone names a channel, not a byte
			static const QRegularExpression channel(QStringLiteral("^%?[IQEA]\\d{1,3}$"),
													QRegularExpression::CaseInsensitiveOption);
			return !channel.match(s).hasMatch();
		}
	}
}

QString bitAddress(PlcAddressing::Format f, bool input, int byte, int bit)
{
	switch (f) {
		case PlcAddressing::Format::SiemensGerman:
			return QStringLiteral("%1%2.%3").arg(input ? QLatin1Char('E') : QLatin1Char('A')).arg(byte).arg(bit);
		case PlcAddressing::Format::Iec:
			return QStringLiteral("%%1X%2.%3").arg(input ? QLatin1Char('I') : QLatin1Char('Q')).arg(byte).arg(bit);
		case PlcAddressing::Format::TwinCat:
			return input ? QStringLiteral("%I*") : QStringLiteral("%Q*");
		default:
			return QStringLiteral("%1%2.%3").arg(input ? QLatin1Char('I') : QLatin1Char('Q')).arg(byte).arg(bit);
	}
}

QString wordAddress(PlcAddressing::Format f, bool input, int byte)
{
	switch (f) {
		case PlcAddressing::Format::SiemensGerman:
			return QStringLiteral("%1W%2").arg(input ? QLatin1Char('E') : QLatin1Char('A')).arg(byte);
		case PlcAddressing::Format::Iec:
			return QStringLiteral("%%1W%2").arg(input ? QLatin1Char('I') : QLatin1Char('Q')).arg(byte);
		case PlcAddressing::Format::TwinCat:
			return input ? QStringLiteral("%I*") : QStringLiteral("%Q*");
		default:
			return QStringLiteral("%1W%2").arg(input ? QLatin1Char('I') : QLatin1Char('Q')).arg(byte);
	}
}

QString byteRange(PlcAddressing::Format f, bool input, int first, int count)
{
	if (count <= 0) return QString();
	QString area;
	switch (f) {
		case PlcAddressing::Format::SiemensGerman: area = input ? QStringLiteral("EB") : QStringLiteral("AB"); break;
		case PlcAddressing::Format::Iec: area = input ? QStringLiteral("%IB") : QStringLiteral("%QB"); break;
		default: area = input ? QStringLiteral("IB") : QStringLiteral("QB"); break;
	}
	if (count == 1) return area + QString::number(first);
	return QStringLiteral("%1%2…%1%3").arg(area).arg(first).arg(first + count - 1);
}

bool hasAnalog(const Element *e)
{
	for (const ElementData::PlcIO &io : e->elementData().plcMasterData().ios)
		if (isAnalog(io.type)) return true;
	return false;
}

}

QString PlcAddressing::formatKey(Format f)
{
	switch (f) {
		case Format::SiemensGerman: return QStringLiteral("siemens_de");
		case Format::Iec: return QStringLiteral("iec");
		case Format::Omron: return QStringLiteral("omron");
		case Format::TwinCat: return QStringLiteral("twincat");
		default: return QStringLiteral("siemens");
	}
}

PlcAddressing::Format PlcAddressing::formatFromKey(const QString &key, Format fallback)
{
	const QString k = key.trimmed().toLower();
	if (k == QLatin1String("siemens") || k == QLatin1String("tia")) return Format::Siemens;
	if (k == QLatin1String("siemens_de") || k == QLatin1String("siemens-de") || k == QLatin1String("deutsch"))
		return Format::SiemensGerman;
	if (k == QLatin1String("iec") || k == QLatin1String("codesys")) return Format::Iec;
	if (k == QLatin1String("omron")) return Format::Omron;
	if (k == QLatin1String("twincat") || k == QLatin1String("beckhoff")) return Format::TwinCat;
	return fallback;
}

QString PlcAddressing::formatTitle(Format f)
{
	switch (f) {
		case Format::SiemensGerman:
			return QCoreApplication::translate("PlcAddressing", "Siemens, German mnemonics (E0.0, A0.0, EW, AW)");
		case Format::Iec:
			return QCoreApplication::translate("PlcAddressing", "IEC 61131-3 / CODESYS (%IX0.0, %QX0.0, %IW, %QW)");
		case Format::Omron:
			return QCoreApplication::translate("PlcAddressing", "Omron CJ/CP (0.00, 100.00, 2000, 2100)");
		case Format::TwinCat:
			return QCoreApplication::translate("PlcAddressing", "Beckhoff TwinCAT (%I*, %Q*: linked in TwinCAT)");
		default:
			return QCoreApplication::translate("PlcAddressing", "Siemens TIA (I0.0, Q0.0, IW, QW)");
	}
}

QString PlcAddressing::stationOf(const Element *module)
{
	return module ? module->elementInformations().value(station_info).toString().trimmed() : QString();
}

QStringList PlcAddressing::stations(QETProject *project)
{
	QStringList out;
	if (!project) return out;
	project->buildFolios(project->diagrams());
	for (Diagram *d : project->diagrams())
		for (Element *e : d->elements())
			if (isPlcModule(e) && !out.contains(stationOf(e)))
				out << stationOf(e);
	return out;
}

PlcAddressing::Station PlcAddressing::station(QETProject *project, const QString &name)
{
	Station s;
	s.name = name;
	if (!project) return s;
	const DiagramContext p = project->projectProperties();
	s.format = formatFromKey(p.value(propertyKey(QStringLiteral("format"), name)).toString(),
							 formatFromKey(p.value(QStringLiteral("plc_address_format")).toString()));
	s.order = p.value(propertyKey(QStringLiteral("order"), name)).toString() == QLatin1String("analog_first")
			? Order::AnalogFirst : Order::Cards;
	const auto read = [&p, &name](const char *what, int &value) {
		bool ok = false;
		const int v = p.value(propertyKey(QString::fromLatin1(what), name)).toString().toInt(&ok);
		if (ok) value = v;
	};
	read("input_start", s.inputStart);
	read("output_start", s.outputStart);
	read("analog_input_start", s.analogInputStart);
	read("analog_output_start", s.analogOutputStart);
	return s;
}

void PlcAddressing::setStation(QETProject *project, const Station &s)
{
	if (!project || project->isReadOnly()) return;
	DiagramContext p = project->projectProperties();
	const DiagramContext old = p;
	p.addValue(propertyKey(QStringLiteral("format"), s.name), formatKey(s.format));
	p.addValue(propertyKey(QStringLiteral("order"), s.name),
			   s.order == Order::AnalogFirst ? QStringLiteral("analog_first") : QStringLiteral("cards"));
	const auto put = [&p, &s](const char *what, int value) {
		const QString key = propertyKey(QString::fromLatin1(what), s.name);
		if (value < 0) p.remove(key); else p.addValue(key, QString::number(value));
	};
	put("input_start", s.inputStart);
	put("output_start", s.outputStart);
	put("analog_input_start", s.analogInputStart);
	put("analog_output_start", s.analogOutputStart);
	if (p == old) return;
	project->setUnshownProjectProperties(p);
	project->setModified(true);
}

QVariantMap PlcAddressing::toMap(const Station &s)
{
	QVariantMap m;
	m.insert(QStringLiteral("name"), s.name);
	m.insert(QStringLiteral("format"), formatKey(s.format));
	m.insert(QStringLiteral("order"), s.order == Order::AnalogFirst ? QStringLiteral("analog_first")
																	 : QStringLiteral("cards"));
	m.insert(QStringLiteral("inputStart"), s.inputStart);
	m.insert(QStringLiteral("outputStart"), s.outputStart);
	m.insert(QStringLiteral("analogInputStart"), s.analogInputStart);
	m.insert(QStringLiteral("analogOutputStart"), s.analogOutputStart);
	return m;
}

PlcAddressing::Station PlcAddressing::fromMap(const QVariantMap &m, const Station &base)
{
	Station s = base;
	if (m.contains(QStringLiteral("format")))
		s.format = formatFromKey(m.value(QStringLiteral("format")).toString(), s.format);
	if (m.contains(QStringLiteral("order")))
		s.order = m.value(QStringLiteral("order")).toString() == QLatin1String("analog_first")
				? Order::AnalogFirst : Order::Cards;
	const auto take = [&m](const char *key, int &value) {
		if (m.contains(QString::fromLatin1(key))) value = m.value(QString::fromLatin1(key)).toInt();
	};
	take("inputStart", s.inputStart);
	take("outputStart", s.outputStart);
	take("analogInputStart", s.analogInputStart);
	take("analogOutputStart", s.analogOutputStart);
	return s;
}

QList<PlcAddressing::Change> PlcAddressing::plan(QETProject *project, bool all, const QString &only)
{
	QList<Change> changes;
	if (!project) return changes;
	const QList<Diagram *> folios = project->diagrams();
	project->buildFolios(folios);

		//The modules of each station in folio order, then left to right
	QHash<QString, QList<Element *>> by_station;
	QStringList order;
	for (Diagram *d : folios) {
		QList<Element *> modules;
		for (Element *e : d->elements())
			if (isPlcModule(e) && !e->elementData().plcMasterData().ios.isEmpty()) modules << e;
		std::sort(modules.begin(), modules.end(), [](Element *a, Element *b) {
			if (a->pos().x() != b->pos().x()) return a->pos().x() < b->pos().x();
			return a->pos().y() < b->pos().y();
		});
		for (Element *e : std::as_const(modules)) {
			const QString name = stationOf(e);
			if (!by_station.contains(name)) order << name;
			by_station[name] << e;
		}
	}

	for (const QString &name : std::as_const(order)) {
		if (!only.isNull() && name != only) continue;
		const Station s = station(project, name);
		QList<Element *> modules = by_station.value(name);
			//WAGO and other CODESYS couplers put the analog (word) modules
			//into the process image first, then the digital ones
		if (s.order == Order::AnalogFirst)
			std::stable_sort(modules.begin(), modules.end(), [](Element *a, Element *b) {
				return hasAnalog(a) && !hasAnalog(b);
			});

		const bool omron = s.format == Format::Omron;
		const int unit = omron ? 1 : 1;          //one word (Omron) or one byte per step
		const int bits = omron ? 16 : 8;
		int in = s.inputStart >= 0 ? s.inputStart : 0;
		int out = s.outputStart >= 0 ? s.outputStart : (omron ? 100 : 0);
		int ain = s.analogInputStart >= 0 ? s.analogInputStart : (omron ? 2000 : -1);
		int aout = s.analogOutputStart >= 0 ? s.analogOutputStart : (omron ? 2100 : -1);

			//Stable: a module whose every row has an address of this
			//format keeps them, and the counters go past them
		QList<Element *> to_number;
		for (Element *module : std::as_const(modules)) {
			bool complete = !all;
			for (const ElementData::PlcIO &io : module->elementData().plcMasterData().ios) {
				int a = 0, b = 0;
				const bool io_link = ioLinkBytes(io.address, &a, &b);
				if ((!io_link && !isAbsolute(io.address, s.format)) || isIoLinkSpec(io.address))
					complete = false;
			}
			if (!complete) {
				to_number << module;
				continue;
			}
			if (s.format == Format::TwinCat) continue;
			if (omron) {
					//channels "0.05", "2000", "IN 3…4 [4 B]": the counters go past them
				for (const ElementData::PlcIO &io : module->elementData().plcMasterData().ios) {
					static const QRegularExpression area(
							QStringLiteral("\\b(IN|OUT)\\s+\\d+(?:\\s*(?:…|\\.\\.)\\s*(\\d+))?"),
							QRegularExpression::CaseInsensitiveOption);
					static const QRegularExpression first(QStringLiteral("\\b(IN|OUT)\\s+(\\d+)"),
														  QRegularExpression::CaseInsensitiveOption);
					auto it = first.globalMatch(io.address);
					if (it.hasNext()) {
						auto rt = area.globalMatch(io.address);
						while (it.hasNext() && rt.hasNext()) {
							const auto m = it.next(), r = rt.next();
							const int last = r.capturedLength(2) ? r.captured(2).toInt() : m.captured(2).toInt();
							if (m.captured(1).compare(QLatin1String("IN"), Qt::CaseInsensitive) == 0)
								in = qMax(in, last + 1);
							else
								out = qMax(out, last + 1);
						}
						continue;
					}
					const int ch = io.address.trimmed().section(QLatin1Char('.'), 0, 0).toInt();
					const bool input = isInput(io.type);
					if (isAnalog(io.type)) {
						int &a = input ? ain : aout;
						a = qMax(a, ch + 1);
					} else {
						int &d = input ? in : out;
						d = qMax(d, ch + 1);
					}
				}
				continue;
			}
			for (const ElementData::PlcIO &io : module->elementData().plcMasterData().ios) {
				static const QRegularExpression rx(QStringLiteral("%?([IQEA])([XBWD]?)(\\d+)"),
												   QRegularExpression::CaseInsensitiveOption);
				auto it = rx.globalMatch(io.address);
				while (it.hasNext()) {
					const auto m = it.next();
					const QChar area = m.captured(1).toUpper().at(0);
					const int next = m.captured(3).toInt()
							+ (m.captured(2).compare(QLatin1String("W"), Qt::CaseInsensitive) == 0 ? 2 : 1);
					if (area == QLatin1Char('I') || area == QLatin1Char('E')) in = qMax(in, next);
					else out = qMax(out, next);
				}
			}
		}

			//pass 0: every channel of each module in turn; with "analog
			//first" pass 1 gives the word channels (analog, IO-Link) of all
			//modules, then pass 2 the digital ones
		const auto number = [&](Element *module, int pass) {
			const QVector<ElementData::PlcIO> ios = module->elementData().plcMasterData().ios;
			int di = 0, dq = 0;
			for (const ElementData::PlcIO &io : ios) {
				int a = 0, b = 0;
				if (ioLinkBytes(io.address, &a, &b) || isAnalog(io.type)) continue;
				if (isInput(io.type)) ++di; else ++dq;
			}
			const int in_base = in, out_base = out;
			if (pass != 1) {
				in += (di + bits - 1) / bits * unit;
				out += (dq + bits - 1) / bits * unit;
			}
			int di_n = 0, dq_n = 0;
			for (int row = 0; row < ios.size(); ++row) {
				const ElementData::PlcIO &io = ios.at(row);
				QString address;
				int ib = 0, ob = 0;
				const bool word = ioLinkBytes(io.address, &ib, &ob) || isAnalog(io.type);
				if ((pass == 1 && !word) || (pass == 2 && word)) continue;
				if (ioLinkBytes(io.address, &ib, &ob)) {
						//IO-Link: the process data bytes of the port, in and out
					QStringList parts;
					if (s.format == Format::TwinCat) {
							//TwinCAT links them; the byte counts stay written
						if (ib) parts << QStringLiteral("%I* [%1 B]").arg(ib);
						if (ob) parts << QStringLiteral("%Q* [%1 B]").arg(ob);
					} else if (omron) {
							//Omron counts channels of 16 bits: the bytes fill
							//whole words; the byte count stays written
						const auto words = [](const char *area, int bytes, int first) {
							const int n = (bytes + 1) / 2;
							const QString ch = n == 1 ? QString::number(first)
									: QStringLiteral("%1…%2").arg(first).arg(first + n - 1);
							return QStringLiteral("%1 %2 [%3 B]").arg(QLatin1String(area), ch).arg(bytes);
						};
						if (ib) { parts << words("IN", ib, in); in += (ib + 1) / 2; }
						if (ob) { parts << words("OUT", ob, out); out += (ob + 1) / 2; }
					} else {
						if (ib) { parts << byteRange(s.format, true, in, ib); in += ib; }
						if (ob) { parts << byteRange(s.format, false, out, ob); out += ob; }
					}
					address = parts.join(QStringLiteral(" / "));
				} else if (isAnalog(io.type)) {
					const bool input = isInput(io.type);
					if (omron) {
						int &w = input ? ain : aout;
						address = QString::number(w++);
					} else {
						int &separate = input ? ain : aout;
						int &shared = input ? in : out;
						int &counter = separate >= 0 ? separate : shared;
						address = wordAddress(s.format, input, counter);
						counter += 2;
					}
				} else {
					const bool input = isInput(io.type);
					int &n = input ? di_n : dq_n;
					const int base = input ? in_base : out_base;
					if (omron)
						address = QStringLiteral("%1.%2").arg(base + n / 16).arg(n % 16, 2, 10, QLatin1Char('0'));
					else
						address = bitAddress(s.format, input, base + n / 8, n % 8);
					++n;
				}
				if (address != io.address.trimmed())
					changes << Change{module, row, io.address, address};
			}
		};
		if (s.order == Order::AnalogFirst) {
			for (Element *module : std::as_const(to_number)) number(module, 1);
			for (Element *module : std::as_const(to_number)) number(module, 2);
		} else {
			for (Element *module : std::as_const(to_number)) number(module, 0);
		}
	}
	return changes;
}

QUndoCommand *PlcAddressing::command(const QList<Change> &changes, QUndoCommand *parent)
{
	if (changes.isEmpty()) return nullptr;
	auto *macro = new QUndoCommand(QCoreApplication::translate("PlcAddressing", "Assign PLC addresses"),
								   parent);
	QHash<Element *, ElementData> data;
	QList<Element *> modules;
	for (const Change &c : changes) {
		if (!data.contains(c.module)) {
			data.insert(c.module, c.module->elementData());
			modules << c.module;
		}
		ElementData &ed = data[c.module];
		ElementData::PlcMasterData plc = ed.plcMasterData();
		if (c.row < plc.ios.size()) plc.ios[c.row].address = c.newAddress;
		ed.setPlcMasterData(plc);
	}
	for (Element *module : std::as_const(modules))
		new ChangeElementDataCommand(module, data.value(module), macro);
	return macro;
}
