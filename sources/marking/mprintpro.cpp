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
#include "mprintpro.h"

#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QProcess>
#include <QProcessEnvironment>
#include <QSaveFile>
#include <QSettings>

namespace {

const char *settings_group = "mprintpro";

	/// The markers a material of the printer takes, the default first.
	/// The PrintJet CONNECT prints MultiCards (name "MC"), the THM
	/// MultiMark roll material ("MM") and labels: Weidmüller brochures
	/// "MultiCard & PrintJet" (3194230000/01/2026) and "MultiMark Marking
	/// System" (2026-02-25), pages in docs/beschriftung-export.md. Names as
	/// M-Print PRO's product reference writes them where it lists them.
const QList<MPrintPro::Material> &table(MPrintPro::Printer printer, MPrintPro::Slot slot)
{
	using M = MPrintPro::Material;
	static const QList<M> none;
		//PrintJet CONNECT: MultiCards
	static const QList<M> pj_terminal {
		{"1609801044", "DEK 5/5 MC-10 NEUTRAL WS"},   //5 mm pitch
		{"1609860000", "WS 12/5 MC NEUTRAL"}};        //Klippon Connect
	static const QList<M> pj_strip {
		{"1876770000", "CC 15/27 MC NEUTRAL WS"},     //ClipCard, holder CC-H 15/27
		{"1609940000", "ESG 9/20 MC NEUTRAL WS"}};
	static const QList<M> pj_device {
		{"1609940000", "ESG 9/20 MC NEUTRAL WS"},
		{"1876770000", "CC 15/27 MC NEUTRAL WS"}};
	static const QList<M> pj_wire {
		{"1918690000", "SF 2/21 NEUTRAL WS V2"},      //0.75-1.5 mm²
		{"1918910000", "SF 3/21 NEUTRAL WS V2"},      //1.5-2.5 mm²
		{"1918630000", "SF 1/21 NEUTRAL WS V2"}};     //0.5-0.8 mm²
	static const QList<M> pj_cable {
		{"1852490000", "SFX 9/24 NEUTRAL WS V2"},     //cable tie, from 7 mm
		{"1860120000", "SFX 11/60 MC NEUTRAL WS"}};
		//THM MultiMark: roll material and labels
	static const QList<M> thm_terminal {
		{"2007110000", "DEK 5/5 MM WS"},
		{"2007190000", "WS 12/5 MM WS"}};
	static const QList<M> thm_strip {
		{"1753231044", "THM MT 30X 15/6 WS/M"}};
	static const QList<M> thm_device {
		{"1753231044", "THM MT 30X 15/6 WS/M"},       //"standard and most used" label
		{"2005190000", "ESG 9/17 MM WS"}};
	static const QList<M> thm_wire {
		{"2719090000", "SFM 2/12 1.8-2.5 MM WS"},
		{"2437870000", "HS-HF 1.6-3.2/25 MM W"}};    //heat shrink sleeve
	static const QList<M> thm_cable {
		{"2005320000", "SFX 9/24 MM WS"},
		{"1774350000", "THM WRITEON 25/76 WS"}};
	const bool pj = printer == MPrintPro::PrintJetConnect;
	switch (slot) {
		case MPrintPro::Wire: return pj ? pj_wire : thm_wire;
		case MPrintPro::Terminal: return pj ? pj_terminal : thm_terminal;
		case MPrintPro::Strip: return pj ? pj_strip : thm_strip;
		case MPrintPro::Device: return pj ? pj_device : thm_device;
		case MPrintPro::Cable: return pj ? pj_cable : thm_cable;
		default: return none;
	}
}

	/// Every field in quotes, quotes doubled, ';' between, Windows text:
	/// what the shipped csvfilter.mis reads (quote '"', delimiter ';', os 0)
QByteArray line(const QString &project, const QString &marker)
{
	auto quote = [](QString v) {
		v.replace(QLatin1Char('"'), QStringLiteral("\"\""));
		v.replace(QLatin1Char('\r'), QLatin1Char(' '));
		v.replace(QLatin1Char('\n'), QLatin1Char(' '));
		return QLatin1Char('"') + v + QLatin1Char('"');
	};
	return (quote(project) + QLatin1Char(';') + quote(marker)).toLatin1() + "\r\n";
}

	/// The project text a marker is filed under: a terminal's strip, else
	/// the plant and location it lies in
QString projectText(const MarkingExport::Item &item)
{
	if (item.kind == MarkingExport::Terminal) {
		return item.group;
	}
	QString place;
	if (!item.plant.isEmpty()) {
		place += (item.plant.startsWith(QLatin1Char('=')) ? QString() : QStringLiteral("=")) + item.plant;
	}
	if (!item.location.isEmpty()) {
		place += (item.location.startsWith(QLatin1Char('+')) ? QString() : QStringLiteral("+")) + item.location;
	}
	return place;
}

QString safeName(QString s)
{
	static const QString bad = QStringLiteral("\\/:*?\"<>| ");
	for (QChar &c : s) {
		if (bad.contains(c)) c = QLatin1Char('_');
	}
	return s;
}

} // namespace

QString MPrintPro::printerKey(Printer printer)
{
	return printer == ThmMultiMark ? QStringLiteral("thm") : QStringLiteral("printjet");
}

bool MPrintPro::printerFromKey(const QString &key, Printer *printer)
{
	const QString k = key.trimmed().toLower();
	Printer p;
	if (k.startsWith(QLatin1String("printjet")) || k == QLatin1String("pjc")) p = PrintJetConnect;
	else if (k.startsWith(QLatin1String("thm")) || k.startsWith(QLatin1String("multimark"))) p = ThmMultiMark;
	else return false;
	if (printer) *printer = p;
	return true;
}

QString MPrintPro::printerTitle(Printer printer)
{
	return printer == ThmMultiMark
		? QCoreApplication::translate("MPrintPro", "THM MultiMark (labels, continuous material)")
		: QCoreApplication::translate("MPrintPro", "PrintJet CONNECT (MultiCard)");
}

QString MPrintPro::slotKey(Slot slot)
{
	switch (slot) {
		case Wire: return QStringLiteral("Draht");
		case Terminal: return QStringLiteral("Klemme");
		case Strip: return QStringLiteral("Klemmleiste");
		case Cable: return QStringLiteral("Kabel");
		default: return QStringLiteral("Geraet");
	}
}

QString MPrintPro::slotTitle(Slot slot)
{
	switch (slot) {
		case Wire: return QCoreApplication::translate("MPrintPro", "Wires:");
		case Terminal: return QCoreApplication::translate("MPrintPro", "Terminals:");
		case Strip: return QCoreApplication::translate("MPrintPro", "Terminal strips:");
		case Cable: return QCoreApplication::translate("MPrintPro", "Cables:");
		default: return QCoreApplication::translate("MPrintPro", "Devices:");
	}
}

MPrintPro::Slot MPrintPro::slotOf(const MarkingExport::Item &item)
{
	switch (item.kind) {
		case MarkingExport::Wire: return Wire;
		case MarkingExport::Terminal: return item.strip_marker ? Strip : Terminal;
		case MarkingExport::Cable: return Cable;
		default: return Device;
	}
}

QList<MPrintPro::Material> MPrintPro::catalog(Printer printer, Slot slot)
{
	return table(printer, slot);
}

QString MPrintPro::materialName(const QString &number)
{
	for (auto printer : {PrintJetConnect, ThmMultiMark}) {
		for (int s = 0; s < SlotCount; ++s) {
			for (const Material &m : table(printer, Slot(s))) {
				if (m.number == number) return m.name;
			}
		}
	}
	return QString();
}

QString MPrintPro::material(const Settings &settings, Slot slot)
{
	const QString own = settings.materials[slot].trimmed();
	if (!own.isEmpty()) return own;
	const QList<Material> &list = table(settings.printer, slot);
	return list.isEmpty() ? QString() : list.first().number;
}

QHash<QString, QString> MPrintPro::materialsByKey(const Settings &settings)
{
	QHash<QString, QString> map;
	for (int s = 0; s < SlotCount; ++s) {
		map.insert(slotKey(Slot(s)), material(settings, Slot(s)));
	}
	return map;
}

MPrintPro::Settings MPrintPro::load()
{
	Settings s;
	QSettings settings;
	settings.beginGroup(settings_group);
	printerFromKey(settings.value("printer", "printjet").toString(), &s.printer);
	for (int i = 0; i < SlotCount; ++i) {
		s.materials[i] = settings.value(QStringLiteral("material_%1_%2")
										.arg(printerKey(s.printer), slotKey(Slot(i)))).toString();
	}
	s.exe = settings.value("exe").toString();
	s.print = settings.value("print", false).toBool();
	return s;
}

void MPrintPro::save(const Settings &s)
{
	QSettings settings;
	settings.beginGroup(settings_group);
	settings.setValue("printer", printerKey(s.printer));
	for (int i = 0; i < SlotCount; ++i) {
		settings.setValue(QStringLiteral("material_%1_%2").arg(printerKey(s.printer), slotKey(Slot(i))),
						  s.materials[i]);
	}
	settings.setValue("exe", s.exe);
	settings.setValue("print", s.print);
}

bool MPrintPro::parseMaterials(const QString &text, Settings *settings, QString *error)
{
	for (const QString &pair : text.split(QLatin1Char(','), Qt::SkipEmptyParts)) {
		const QString key = pair.section(QLatin1Char('='), 0, 0).trimmed().toLower();
		const QString value = pair.section(QLatin1Char('='), 1).trimmed();
		int slot = -1;
		if (key.startsWith(QLatin1String("draht")) || key.startsWith(QLatin1String("wire"))) slot = Wire;
		else if (key.startsWith(QLatin1String("klemmleiste")) || key.startsWith(QLatin1String("strip"))) slot = Strip;
		else if (key.startsWith(QLatin1String("klemme")) || key.startsWith(QLatin1String("terminal"))) slot = Terminal;
		else if (key.startsWith(QLatin1String("ger")) || key.startsWith(QLatin1String("device"))) slot = Device;
		else if (key.startsWith(QLatin1String("kabel")) || key.startsWith(QLatin1String("cable"))) slot = Cable;
		bool number = !value.isEmpty();
		for (QChar c : value) number = number && c.isDigit();
		if (slot < 0 || !number) {
			if (error) {
				*error = QCoreApplication::translate("MPrintPro",
					"Bad material assignment \"%1\" (expected e.g. Klemme=1609801044)").arg(pair);
			}
			return false;
		}
		settings->materials[slot] = value;
	}
	return true;
}

QString MPrintPro::findExe(const Settings &settings)
{
	if (!settings.exe.isEmpty() && QFileInfo(settings.exe).isFile()) {
		return settings.exe;
	}
		//The setup puts MPrintPRO.exe under <install>\ProgramFiles\bin
		//(mprintpro.xml of the installer); the install folder is the
		//user's choice, so the Weidmüller folders of both program
		//directories are searched
	const QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
	QStringList roots;
	for (const char *var : {"ProgramFiles(x86)", "ProgramFiles", "ProgramW6432"}) {
		const QString dir = env.value(QString::fromLatin1(var));
		if (!dir.isEmpty() && !roots.contains(dir)) roots << dir;
	}
	for (const QString &root : roots) {
		const QStringList makers = QDir(root).entryList({QStringLiteral("Weidm*")}, QDir::Dirs | QDir::NoDotAndDotDot);
		for (const QString &maker : makers) {
			QDirIterator it(QDir(root).filePath(maker), {QStringLiteral("MPrintPRO.exe")},
							QDir::Files, QDirIterator::Subdirectories);
			if (it.hasNext()) return QDir::toNativeSeparators(it.next());
		}
	}
	return QString();
}

QByteArray MPrintPro::jobFile(const QList<MarkingExport::Item> &items)
{
	QByteArray out;
	for (const MarkingExport::Item &item : items) {
		for (int n = 0; n < item.count; ++n) {
			out += line(projectText(item), item.text);
		}
	}
	return out;
}

QList<MPrintPro::Job> MPrintPro::writeJobs(const QList<MarkingExport::Item> &items,
										   const Settings &settings, const QString &dir,
										   const QString &base, QString *error)
{
		//Grouped by material, in the order the materials first appear
	QStringList order;
	QHash<QString, QList<MarkingExport::Item>> by_material;
	for (const MarkingExport::Item &item : items) {
		const QString m = material(settings, slotOf(item));
		if (m.isEmpty()) continue;
		if (!by_material.contains(m)) order << m;
		by_material[m] << item;
	}
	QList<Job> jobs;
	if (!QDir().mkpath(dir)) {
		if (error) *error = QCoreApplication::translate("MPrintPro", "Cannot create the folder %1").arg(dir);
		return jobs;
	}
	for (const QString &m : std::as_const(order)) {
		Job job;
		job.material = m;
		job.material_name = materialName(m);
		const QList<MarkingExport::Item> &list = by_material[m];
		for (const auto &i : list) job.markers += i.count;
		QString kinds;
		for (int s = 0; s < SlotCount; ++s) {
			if (material(settings, Slot(s)) == m) {
				kinds += (kinds.isEmpty() ? QString() : QStringLiteral("+")) + slotKey(Slot(s));
			}
		}
		job.file = QDir::toNativeSeparators(QDir(dir).filePath(
			safeName(QStringLiteral("%1_%2_%3.csv").arg(base, kinds, m))));
		const QByteArray bytes = jobFile(list);
		QSaveFile file(job.file);
		if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit()) {
			if (error) *error = file.errorString();
			return {};
		}
		jobs << job;
	}
	return jobs;
}

QStringList MPrintPro::arguments(const Job &job, bool print)
{
	QStringList args {job.file};
	if (print) args << QStringLiteral("-p");
	args << QStringLiteral("-ImportFilter:CSV")
		 << QStringLiteral("-MatNo:") + job.material;
	return args;
}

bool MPrintPro::open(const QList<Job> &jobs, const Settings &settings, QString *error)
{
	const QString exe = findExe(settings);
	if (exe.isEmpty()) {
		if (error) {
			*error = QCoreApplication::translate("MPrintPro",
				"M-Print PRO was not found. Choose MPrintPRO.exe in the export dialog.");
		}
		return false;
	}
	for (const Job &job : jobs) {
		if (!QProcess::startDetached(exe, arguments(job, settings.print))) {
			if (error) {
				*error = QCoreApplication::translate("MPrintPro",
					"M-Print PRO could not be started: %1").arg(exe);
			}
			return false;
		}
	}
	return true;
}
