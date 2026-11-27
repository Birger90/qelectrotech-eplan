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
#include "networkaddresses.h"

#include "../diagram.h"
#include "../diagramcontext.h"
#include "../duplicatelabels.h"
#include "../qetgraphicsitem/element.h"
#include "../qetproject.h"
#include "../undocommand/changeelementinformationcommand.h"

#include <QCoreApplication>
#include <QFile>
#include <QHash>
#include <QRegularExpression>
#include <QSet>
#include <QTextStream>
#include <QUndoCommand>

namespace {

const QString key_ip = QStringLiteral("ip_address");
const QString key_mask = QStringLiteral("subnet_mask");
const QString key_net = QStringLiteral("network_name");
const QString key_pn = QStringLiteral("profinet_name");
const QString key_networks = QStringLiteral("ip_networks");

quint32 maskOf(int prefix)
{
	return prefix <= 0 ? 0u : (prefix >= 32 ? 0xffffffffu : (0xffffffffu << (32 - prefix)));
}

bool isDevice(Element *e)
{
	const DiagramContext info = e->elementInformations();
	if (info.contains(key_ip) || info.contains(key_pn)) return true;
	return e->elementData().m_type == ElementData::Master
			&& e->elementData().m_master_type == ElementData::PLC
			&& !info.value(QStringLiteral("plc_station")).toString().isEmpty();
}

QUndoCommand *infoCommand(const QHash<Element *, DiagramContext> &changes, const QString &text)
{
	if (changes.isEmpty()) return nullptr;
	auto *macro = new QUndoCommand(text);
	for (auto it = changes.constBegin(); it != changes.constEnd(); ++it)
		new ChangeElementInformationCommand(it.key(), it.key()->elementInformations(), it.value(), macro);
	return macro;
}

}

bool NetworkAddresses::ipv4(const QString &text, quint32 *value)
{
	const QStringList parts = text.trimmed().split(QLatin1Char('.'));
	if (parts.size() != 4) return false;
	quint32 v = 0;
	for (const QString &p : parts) {
		bool ok = false;
		const int n = p.toInt(&ok);
		if (!ok || n < 0 || n > 255 || p.isEmpty() || (p.size() > 1 && p.startsWith(QLatin1Char('0'))))
			return false;
		v = (v << 8) | quint32(n);
	}
	if (value) *value = v;
	return true;
}

QString NetworkAddresses::ipText(quint32 v)
{
	return QStringLiteral("%1.%2.%3.%4").arg(v >> 24).arg((v >> 16) & 0xff).arg((v >> 8) & 0xff).arg(v & 0xff);
}

QString NetworkAddresses::profinetNameProblem(const QString &name)
{
	if (name.isEmpty()) return QCoreApplication::translate("NetworkAddresses", "is empty");
	if (name.size() > 240) return QCoreApplication::translate("NetworkAddresses", "is longer than 240 characters");
	if (ipv4(name, nullptr)) return QCoreApplication::translate("NetworkAddresses", "has the form of an IP address");
	static const QRegularExpression port(QStringLiteral("^port-\\d{3}(-\\d{5})?$"));
	for (const QString &label : name.split(QLatin1Char('.'))) {
		if (label.isEmpty()) return QCoreApplication::translate("NetworkAddresses", "has an empty part between dots");
		if (label.size() > 63) return QCoreApplication::translate("NetworkAddresses", "has a part longer than 63 characters");
		for (const QChar ch : label)
			if (!((ch >= QLatin1Char('a') && ch <= QLatin1Char('z')) || (ch >= QLatin1Char('0') && ch <= QLatin1Char('9'))
				  || ch == QLatin1Char('-')))
				return QCoreApplication::translate("NetworkAddresses", "holds \"%1\": only a-z, 0-9, - and . are allowed").arg(ch);
		if (label.startsWith(QLatin1Char('-')) || label.endsWith(QLatin1Char('-')))
			return QCoreApplication::translate("NetworkAddresses", "has a part beginning or ending with -");
	}
	if (port.match(name.section(QLatin1Char('.'), 0, 0)).hasMatch())
		return QCoreApplication::translate("NetworkAddresses", "begins like a port name (port-xyz)");
	return QString();
}

QString NetworkAddresses::profinetNameFor(const QString &label, const QString &location)
{
	const QString raw = (location.trimmed() + QLatin1Char(' ') + label.trimmed()).toLower();
	QString out;
	for (const QChar ch : raw) {
		if ((ch >= QLatin1Char('a') && ch <= QLatin1Char('z')) || (ch >= QLatin1Char('0') && ch <= QLatin1Char('9')))
			out += ch;
		else if (ch == QChar(0xe4)) out += QStringLiteral("ae");
		else if (ch == QChar(0xf6)) out += QStringLiteral("oe");
		else if (ch == QChar(0xfc)) out += QStringLiteral("ue");
		else if (ch == QChar(0xdf)) out += QStringLiteral("ss");
		else if (!out.isEmpty() && !out.endsWith(QLatin1Char('-')))
			out += QLatin1Char('-');
	}
	while (out.endsWith(QLatin1Char('-'))) out.chop(1);
	if (out.isEmpty()) out = QStringLiteral("device");
		//A name beginning with a digit is allowed but reads like an
		//address: the label -2A1 alone gives "pn-2a1"
	if (out.at(0).isDigit()) out.prepend(QStringLiteral("pn-"));
	out = out.left(63);
	while (out.endsWith(QLatin1Char('-'))) out.chop(1);
	return out;
}

QList<NetworkAddresses::Network> NetworkAddresses::networks(QETProject *project)
{
	QList<Network> out;
	if (!project) return out;
	const QString text = project->projectProperties().value(key_networks).toString();
	for (const QString &entry : text.split(QLatin1Char(';'), Qt::SkipEmptyParts)) {
		const QStringList f = entry.split(QLatin1Char('|'));
		Network n;
		n.name = f.value(0).trimmed();
		const QString cidr = f.value(1).trimmed();
		bool ok = false;
		n.prefix = cidr.section(QLatin1Char('/'), 1, 1).toInt(&ok);
		if (!ok || n.prefix < 1 || n.prefix > 30) n.prefix = 24;
		quint32 v = 0;
		if (!ipv4(cidr.section(QLatin1Char('/'), 0, 0), &v)) continue;
		n.address = v & maskOf(n.prefix);
		quint32 g = 0;
		if (ipv4(f.value(2).trimmed(), &g)) n.gateway = g;
		const int first = f.value(3).trimmed().toInt(&ok);
		n.firstHost = ok && first > 0 ? first : 1;
		out << n;
	}
	return out;
}

bool NetworkAddresses::parseNetwork(const QString &text, Network *network)
{
	static const QRegularExpression rx(QStringLiteral(
		"^\\s*(?:([^=|]+?)\\s*=\\s*)?(\\d+\\.\\d+\\.\\d+\\.\\d+)\\s*/\\s*(\\d{1,2})"
		"(?:\\s*,\\s*(?:gw|gateway)?\\s*(\\d+\\.\\d+\\.\\d+\\.\\d+))?(?:\\s*,\\s*(?:ab|from)?\\s*(\\d+))?\\s*$"),
		QRegularExpression::CaseInsensitiveOption);
	const auto m = rx.match(text);
	if (!m.hasMatch()) return false;
	Network n;
	n.name = m.captured(1).trimmed();
	n.prefix = m.captured(3).toInt();
	quint32 v = 0;
	if (n.prefix < 1 || n.prefix > 30 || !ipv4(m.captured(2), &v)) return false;
	n.address = v & maskOf(n.prefix);
	quint32 g = 0;
	if (!m.captured(4).isEmpty() && ipv4(m.captured(4), &g)) n.gateway = g;
	if (!m.captured(5).isEmpty()) n.firstHost = qMax(1, m.captured(5).toInt());
	if (network) *network = n;
	return true;
}

QString NetworkAddresses::networkText(const Network &n)
{
	QString s = QStringLiteral("%1/%2").arg(ipText(n.address)).arg(n.prefix);
	if (!n.name.isEmpty()) s.prepend(n.name + QStringLiteral(" = "));
	if (n.gateway) s += QStringLiteral(", gw ") + ipText(n.gateway);
	s += QStringLiteral(", from %1").arg(n.firstHost);
	return s;
}

void NetworkAddresses::setNetworks(QETProject *project, const QList<Network> &list)
{
	if (!project || project->isReadOnly()) return;
	QStringList entries;
	for (const Network &n : list)
		entries << QStringLiteral("%1|%2/%3|%4|%5").arg(n.name, ipText(n.address)).arg(n.prefix)
				   .arg(n.gateway ? ipText(n.gateway) : QString()).arg(n.firstHost);
	DiagramContext p = project->projectProperties();
	const QString value = entries.join(QLatin1Char(';'));
	if (p.value(key_networks).toString() == value) return;
	p.addValue(key_networks, value);
	project->setUnshownProjectProperties(p);
	project->setModified(true);
}

QList<NetworkAddresses::Row> NetworkAddresses::rows(QETProject *project)
{
	QList<Row> out;
	if (!project) return out;
	project->buildFolios(project->diagrams());
	const QList<Network> nets = networks(project);
	QSet<QString> seen;
	for (Diagram *d : project->diagrams()) {
		for (Element *e : d->elements()) {
			if (!isDevice(e)) continue;
			const DiagramContext info = e->elementInformations();
			Row r;
			r.element = e;
			r.label = DuplicateLabels::displayedLabel(e).trimmed();
			r.location = info.value(QStringLiteral("location")).toString().trimmed();
			if (r.location.isEmpty()) r.location = d->border_and_titleblock.locmach().trimmed();
			r.folio = d->folioIndex();
			r.ip = info.value(key_ip).toString().trimmed();
			r.mask = info.value(key_mask).toString().trimmed();
			r.network = info.value(key_net).toString().trimmed();
			r.profinet = info.value(key_pn).toString().trimmed();
			r.station = info.value(QStringLiteral("plc_station")).toString().trimmed();
				//A PLC station takes both, a symbol the fields it has
			r.ipField = info.contains(key_ip) || !r.station.isEmpty();
			r.profinetField = info.contains(key_pn) || !r.station.isEmpty();
				//A device drawn more than once is one device
			const QString key = r.label + QLatin1Char('|') + r.ip + QLatin1Char('|') + r.profinet;
			if (!r.label.isEmpty() && seen.contains(key)) continue;
			seen << key;
			out << r;
		}
	}
		//The problems of each row: an IP given twice or outside its
		//network, a PROFINET name not valid or given twice
	QHash<QString, int> ips, names;
	for (const Row &r : std::as_const(out)) {
		if (!r.ip.isEmpty()) ++ips[r.ip];
		if (!r.profinet.isEmpty()) ++names[r.profinet.toLower()];
	}
	for (Row &r : out) {
		quint32 v = 0;
		if (!r.ip.isEmpty()) {
			if (!ipv4(r.ip, &v)) {
				r.problems << QCoreApplication::translate("NetworkAddresses", "IP address not valid");
			} else {
				if (ips.value(r.ip) > 1)
					r.problems << QCoreApplication::translate("NetworkAddresses", "IP address used %n times",
															  nullptr, ips.value(r.ip));
				for (const Network &n : nets) {
					if (!r.network.isEmpty() && n.name != r.network) continue;
					if ((v & maskOf(n.prefix)) != n.address)
						r.problems << QCoreApplication::translate("NetworkAddresses", "outside network %1")
									  .arg(n.name.isEmpty() ? networkText(n) : n.name);
					else if (v == n.address || v == (n.address | ~maskOf(n.prefix)))
						r.problems << QCoreApplication::translate("NetworkAddresses", "network or broadcast address");
					break;
				}
			}
		}
		if (!r.profinet.isEmpty()) {
			const QString p = profinetNameProblem(r.profinet);
			if (!p.isEmpty())
				r.problems << QCoreApplication::translate("NetworkAddresses", "PROFINET name %1").arg(p);
			if (names.value(r.profinet.toLower()) > 1)
				r.problems << QCoreApplication::translate("NetworkAddresses", "PROFINET name used %n times", nullptr,
														  names.value(r.profinet.toLower()));
		}
	}
	return out;
}

QUndoCommand *NetworkAddresses::assignIpCommand(QETProject *project, int *count)
{
	if (count) *count = 0;
	const QList<Network> nets = networks(project);
	if (!project || nets.isEmpty()) return nullptr;
	const QList<Row> list = rows(project);
	QSet<quint32> used;
	for (const Row &r : list) {
		quint32 v = 0;
		if (ipv4(r.ip, &v)) used << v;
	}
	for (const Network &n : nets)
		if (n.gateway) used << n.gateway;
	QHash<Element *, DiagramContext> changes;
	for (const Row &r : list) {
		if (!r.ip.isEmpty() || !r.ipField) continue;
		Network net = nets.first();
		for (const Network &n : nets)
			if (!r.network.isEmpty() && n.name == r.network) net = n;
		const quint32 broadcast = net.address | ~maskOf(net.prefix);
		quint32 v = net.address + quint32(qMax(1, net.firstHost));
		while (v < broadcast && used.contains(v)) ++v;
		if (v >= broadcast) continue;
		used << v;
		DiagramContext info = r.element->elementInformations();
		info.addValue(key_ip, ipText(v));
		if (info.value(key_mask).toString().trimmed().isEmpty())
			info.addValue(key_mask, ipText(maskOf(net.prefix)));
		if (info.value(key_net).toString().trimmed().isEmpty() && !net.name.isEmpty())
			info.addValue(key_net, net.name);
		changes.insert(r.element, info);
	}
	if (count) *count = int(changes.size());
	return infoCommand(changes, QCoreApplication::translate("NetworkAddresses", "Assign IP addresses"));
}

QUndoCommand *NetworkAddresses::profinetNamesCommand(QETProject *project, int *count)
{
	if (count) *count = 0;
	if (!project) return nullptr;
	const QList<Row> list = rows(project);
	QSet<QString> used;
	for (const Row &r : list)
		if (!r.profinet.isEmpty()) used << r.profinet.toLower();
	QHash<Element *, DiagramContext> changes;
	for (const Row &r : list) {
		if (!r.profinet.isEmpty() || !r.profinetField) continue;
		const QString base = profinetNameFor(r.label.isEmpty() ? r.element->name() : r.label, r.location);
		QString name = base;
		for (int n = 2; used.contains(name); ++n)
			name = QStringLiteral("%1-%2").arg(base).arg(n);
		used << name;
		DiagramContext info = r.element->elementInformations();
		info.addValue(key_pn, name);
		changes.insert(r.element, info);
	}
	if (count) *count = int(changes.size());
	return infoCommand(changes, QCoreApplication::translate("NetworkAddresses", "PROFINET names from the labels"));
}

QUndoCommand *NetworkAddresses::setValueCommand(Element *element, const QString &key, const QString &value)
{
	if (!element) return nullptr;
	DiagramContext info = element->elementInformations();
	if (info.value(key).toString() == value) return nullptr;
	info.addValue(key, value);
	QHash<Element *, DiagramContext> changes;
	changes.insert(element, info);
	return infoCommand(changes, QCoreApplication::translate("NetworkAddresses", "Edit network address"));
}

bool NetworkAddresses::exportCsv(QETProject *project, const QString &path, QString *error)
{
	QFile file(path);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		if (error) *error = file.errorString();
		return false;
	}
	QTextStream out(&file);
	out.setEncoding(QStringConverter::Utf8);
	out << QChar(0xfeff);
	const auto cell = [](QString s) {
		s.replace(QLatin1Char('"'), QStringLiteral("\"\""));
		return QStringLiteral("\"%1\"").arg(s);
	};
	out << QStringLiteral("Label;Location;Folio;IP address;Subnet mask;Network;PROFINET name;PLC station;Problems\r\n");
	for (const Row &r : rows(project)) {
		out << cell(r.label) << ';' << cell(r.location) << ';' << (r.folio + 1) << ';' << cell(r.ip) << ';'
			<< cell(r.mask) << ';' << cell(r.network) << ';' << cell(r.profinet) << ';' << cell(r.station) << ';'
			<< cell(r.problems.join(QStringLiteral(", "))) << "\r\n";
	}
	return true;
}
