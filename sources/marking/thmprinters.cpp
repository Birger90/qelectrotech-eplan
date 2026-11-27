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
#include "thmprinters.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QSet>
#include <QSettings>

namespace {
const char *settings_group = "thm_print";
}

QList<ThmPrinter> ThmPrinters::load(QSettings &s)
{
	QList<ThmPrinter> printers;
	s.beginGroup(settings_group);
	const int n = s.beginReadArray(QStringLiteral("printers"));
	for (int i = 0; i < n; ++i) {
		s.setArrayIndex(i);
		ThmPrinter p;
		p.id = s.value("id").toString();
		p.name = s.value("name").toString();
		p.model = s.value("model").toString();
		p.host = s.value("host").toString();
		p.port = quint16(s.value("port", 9100).toUInt());
			//Same keys as ThmClient::transportKey(); spelled out here so the
			//printer list needs no network code
		p.transport = s.value("transport", "rawip").toString().compare(QLatin1String("lpd"), Qt::CaseInsensitive) == 0
				? ThmClient::Transport::Lpd : ThmClient::Transport::RawIp;
		p.lpd_queue = s.value("lpdQueue", "lp").toString();
		p.material_no = s.value("materialNo").toString();
		p.material_name = s.value("materialName").toString();
		p.loaded_at = QDateTime::fromString(s.value("loadedAt").toString(), Qt::ISODate);
			//Printers set up before the usual article existed: what they
			//carry is their usual article
		p.usual_material_no = s.value("usualMaterialNo", p.material_no).toString();
		p.usual_material_name = s.value("usualMaterialName", p.material_name).toString();
		p.roller = s.value("roller").toString();
		ThmJob::Profile &f = p.profile;
		f.width = s.value("width", 0.0).toDouble();
		f.height = s.value("height", 0.0).toDouble();
		f.gap = s.value("gap", 0.0).toDouble();
		ThmJob::sensorFromKey(s.value("sensor", "l1").toString(), &f.sensor);
		f.columns = s.value("columns", 1).toInt();
		f.column_pitch = s.value("columnPitch", 0.0).toDouble();
		f.margin_left = s.value("marginLeft", 0.5).toDouble();
		f.margin_right = s.value("marginRight", 0.5).toDouble();
		f.offset_x = s.value("offsetX", 0.0).toDouble();
		f.offset_y = s.value("offsetY", 0.0).toDouble();
		f.font = s.value("font", 3).toInt();
		f.text_size = s.value("textSize", 3.0).toDouble();
		f.baseline = s.value("baseline", 0.0).toDouble();
		f.set_heat = s.value("setHeat", false).toBool();
		f.speed = s.value("speed", 0).toInt();
		f.heat = s.value("heat", 0).toInt();
		f.transfer = s.value("transfer", true).toBool();
		f.rotate = s.value("rotate", false).toBool();
		if (p.id.isEmpty()) p.id = newId(printers);
		printers << p;
	}
	s.endArray();
	s.endGroup();
	return printers;
}

void ThmPrinters::save(QSettings &s, const QList<ThmPrinter> &printers)
{
	s.beginGroup(settings_group);
	s.remove(QStringLiteral("printers"));
	s.beginWriteArray(QStringLiteral("printers"), printers.size());
	for (int i = 0; i < printers.size(); ++i) {
		s.setArrayIndex(i);
		const ThmPrinter &p = printers.at(i);
		const ThmJob::Profile &f = p.profile;
		s.setValue("id", p.id);
		s.setValue("name", p.name);
		s.setValue("model", p.model);
		s.setValue("host", p.host);
		s.setValue("port", p.port);
		s.setValue("transport", p.transport == ThmClient::Transport::Lpd ? QStringLiteral("lpd") : QStringLiteral("rawip"));
		s.setValue("lpdQueue", p.lpd_queue);
		s.setValue("materialNo", p.material_no);
		s.setValue("materialName", p.material_name);
		s.setValue("loadedAt", p.loaded_at.isValid() ? p.loaded_at.toString(Qt::ISODate) : QString());
		s.setValue("usualMaterialNo", p.usual_material_no);
		s.setValue("usualMaterialName", p.usual_material_name);
		s.setValue("roller", p.roller);
		s.setValue("width", f.width);
		s.setValue("height", f.height);
		s.setValue("gap", f.gap);
		s.setValue("sensor", ThmJob::sensorKey(f.sensor));
		s.setValue("columns", f.columns);
		s.setValue("columnPitch", f.column_pitch);
		s.setValue("marginLeft", f.margin_left);
		s.setValue("marginRight", f.margin_right);
		s.setValue("offsetX", f.offset_x);
		s.setValue("offsetY", f.offset_y);
		s.setValue("font", f.font);
		s.setValue("textSize", f.text_size);
		s.setValue("baseline", f.baseline);
		s.setValue("setHeat", f.set_heat);
		s.setValue("speed", f.speed);
		s.setValue("heat", f.heat);
		s.setValue("transfer", f.transfer);
		s.setValue("rotate", f.rotate);
	}
	s.endArray();
	s.endGroup();
}

QList<ThmPrinter> ThmPrinters::load()
{
	QSettings settings;
	return load(settings);
}

void ThmPrinters::save(const QList<ThmPrinter> &printers)
{
	QSettings settings;
	save(settings, printers);
}

namespace {
QStringList data_dirs;

QString firstExisting(const QStringList &files)
{
	for (const QString &f : files)
		if (!f.isEmpty() && QFileInfo::exists(f)) return QDir::fromNativeSeparators(f);
	return QString();
}
}

void ThmPrinters::setDataDirs(const QStringList &dirs)
{
	data_dirs = dirs;
}

QStringList ThmPrinters::dataDirs()
{
		//First the folder installed with the program (the deployment puts
		//the Weidmüller files there), then those the application set
	QStringList dirs;
	if (QCoreApplication::instance())
		dirs << QCoreApplication::applicationDirPath() + QStringLiteral("/weidmueller");
	return dirs + data_dirs;
}

QString ThmPrinters::catalogFile(QSettings &s)
{
	const QString chosen = s.value(QStringLiteral("%1/catalogFile").arg(QLatin1String(settings_group))).toString();
	if (!chosen.isEmpty() && QFileInfo::exists(chosen)) return chosen;
		//Where M-Print PRO keeps it, then the data folders
	QStringList files;
	const QString program_data = qEnvironmentVariable("ProgramData");
	if (!program_data.isEmpty()) {
		const QString products = program_data + QStringLiteral("/weidmueller/M-Print PRO/products/");
		files << products + QStringLiteral("my_cat_DE.mca") << products + QStringLiteral("my_cat.mca");
	}
	for (const QString &d : dataDirs())
		files << d + QStringLiteral("/my_cat_DE.mca") << d + QStringLiteral("/my_cat.mca");
	const QString found = firstExisting(files);
	return found.isEmpty() ? chosen : found;
}

void ThmPrinters::setCatalogFile(QSettings &s, const QString &path)
{
	s.setValue(QStringLiteral("%1/catalogFile").arg(QLatin1String(settings_group)), path);
}

QString ThmPrinters::layoutsFile(QSettings &s)
{
	const QString chosen = s.value(QStringLiteral("%1/layoutsFile").arg(QLatin1String(settings_group))).toString();
	if (!chosen.isEmpty() && QFileInfo::exists(chosen)) return chosen;
	QStringList files {qEnvironmentVariable("QET_THM_LAYOUTS")};
	for (const QString &d : dataDirs()) files << d + QStringLiteral("/mprint-multimark-layouts.json");
	const QString found = firstExisting(files);
	return found.isEmpty() ? chosen : found;
}

void ThmPrinters::setLayoutsFile(QSettings &s, const QString &path)
{
	s.setValue(QStringLiteral("%1/layoutsFile").arg(QLatin1String(settings_group)), path);
}

void ThmPrinters::confirmLoading(ThmPrinter *p, const QString &material_no,
								 const QString &material_name, const QDateTime &when, bool usual)
{
	if (!p) return;
	p->material_no = material_no.trimmed();
	p->material_name = material_name.trimmed();
	p->loaded_at = when;
	if (usual) {
		p->usual_material_no = p->material_no;
		p->usual_material_name = p->material_name;
	}
}

bool ThmPrinters::changedFromUsual(const ThmPrinter &p)
{
	return !normalizedMaterial(p.usual_material_no).isEmpty()
		   && normalizedMaterial(p.usual_material_no) != normalizedMaterial(p.material_no);
}

QString ThmPrinters::newId(const QList<ThmPrinter> &existing)
{
	QSet<QString> used;
	for (const ThmPrinter &p : existing) used.insert(p.id);
	for (int n = 1;; ++n) {
		const QString id = QStringLiteral("thm%1").arg(n);
		if (!used.contains(id)) return id;
	}
}

bool ThmPrinters::validate(const ThmPrinter &p, QString *error, bool check_profile)
{
	QString e;
	if (p.id.trimmed().isEmpty()) e = QCoreApplication::translate("ThmPrinters", "The printer has no ID.");
	else if (p.name.trimmed().isEmpty()) e = QCoreApplication::translate("ThmPrinters", "The printer has no name.");
	else if (p.host.trimmed().isEmpty()) e = QCoreApplication::translate("ThmPrinters", "The printer \"%1\" has no address.").arg(p.name);
	else if (p.port == 0) e = QCoreApplication::translate("ThmPrinters", "The printer \"%1\" has no port.").arg(p.name);
	else if (normalizedMaterial(p.material_no).isEmpty())
		e = QCoreApplication::translate("ThmPrinters", "No article variant is set for the printer \"%1\".").arg(p.name);
	else if (check_profile) {
		QString profile_error;
		if (!ThmJob::validate(p.profile, &profile_error))
			e = QCoreApplication::translate("ThmPrinters", "Print profile of \"%1\": %2").arg(p.name, profile_error);
	}
	if (error) *error = e;
	return e.isEmpty();
}

QString ThmPrinters::normalizedMaterial(const QString &material_no)
{
	QString m = material_no;
	m.remove(QLatin1Char(' '));
	return m.trimmed().toUpper();
}

int ThmPrinters::indexOf(const QList<ThmPrinter> &printers, const QString &id)
{
	for (int i = 0; i < printers.size(); ++i) {
		if (printers.at(i).id.compare(id.trimmed(), Qt::CaseInsensitive) == 0) return i;
	}
	return -1;
}

ThmPrinters::Choice ThmPrinters::choose(const QList<ThmPrinter> &printers,
										 const QString &material_no)
{
	Choice c;
	const QString want = normalizedMaterial(material_no);
	if (printers.isEmpty()) {
		c.outcome = Choice::NoPrinters;
		c.message = QCoreApplication::translate("ThmPrinters", "No THM printer is set up. Add one under \"Printers...\".");
		return c;
	}
	if (want.isEmpty()) {
		c.outcome = Choice::NoMaterial;
		c.message = QCoreApplication::translate("ThmPrinters", "Choose the article variant to print on.");
		return c;
	}
	for (int i = 0; i < printers.size(); ++i) {
		if (normalizedMaterial(printers.at(i).material_no) == want) c.matches << i;
	}
	if (c.matches.isEmpty()) {
		c.outcome = Choice::NoMatch;
		c.message = QCoreApplication::translate("ThmPrinters", "No printer is loaded with article %1. Load it on a printer "
					   "and confirm it with \"Change material...\".").arg(material_no.trimmed());
	} else if (c.matches.size() == 1) {
		c.outcome = Choice::Unique;
		c.message = QCoreApplication::translate("ThmPrinters", "Printer \"%1\" is loaded with article %2.")
				.arg(printers.at(c.matches.first()).name, material_no.trimmed());
	} else {
		c.outcome = Choice::Several;
		c.message = QCoreApplication::translate("ThmPrinters", "%1 printers are loaded with article %2: choose one.")
				.arg(c.matches.size()).arg(material_no.trimmed());
	}
	return c;
}
