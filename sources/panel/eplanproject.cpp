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
#include "eplanproject.h"

#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QMap>
#include <QPointF>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QtEndian>

#include <cstring>

namespace {

const QByteArray record_start = QByteArray::fromHex("ffff61ea");
const QByteArray function_start = QByteArray::fromHex("ffff61ea1100");
const QByteArray page_tag = QByteArray::fromHex("f2eb0400");

quint32 u32(const QByteArray &b, int at) {return qFromLittleEndian<quint32>(b.constData() + at);}
quint16 u16(const QByteArray &b, int at) {return qFromLittleEndian<quint16>(b.constData() + at);}
double f64(const QByteArray &b, int at)
{
	double v;
	const quint64 raw = qFromLittleEndian<quint64>(b.constData() + at);
	std::memcpy(&v, &raw, sizeof v);
	return v;
}

	/// two doubles after a two byte field tag
bool point(const QByteArray &r, const QByteArray &tag, QPointF *p)
{
	const int at = r.indexOf(tag);
	if (at < 0 || at + 18 > r.size()) return false;
	*p = QPointF(f64(r, at + 2), f64(r, at + 10));
	return true;
}

	/// the property fields after an identity tag: <u16 id> 00 00 <text> 00 ...
QMap<int, QString> fields(const QByteArray &r, int from)
{
	QMap<int, QString> f;
	int q = from;
	while (q + 4 <= r.size()) {
		const quint16 tag = u16(r, q);
		if (tag == 0xffff || r.at(q + 2) != 0 || r.at(q + 3) != 0) break;
		const int end = r.indexOf('\0', q + 4);
		if (end < 0) break;
		f.insert(tag, QString::fromLatin1(r.mid(q + 4, end - q - 4)));
		q = end + 1;
	}
	return f;
}

struct FunctionRecord {
	quint32 page = 0;
	QMap<int, QString> id;
	QPointF ins, lo, hi;
	bool has_ins = false, has_box = false;
};

}

QString EplanProject::unpack(const QString &zw1, const QString &dir, QString *error)
{
	QString seven = QStandardPaths::findExecutable(QStringLiteral("7z"));
	for (const QString &p : {QStringLiteral("C:/Program Files/7-Zip/7z.exe"), QStringLiteral("C:/Program Files (x86)/7-Zip/7z.exe")})
		if (seven.isEmpty() && QFileInfo::exists(p)) seven = p;
	if (seven.isEmpty()) {
		if (error) *error = QCoreApplication::translate("EplanProject", "A project archive (.zw1) is a 7z archive: install 7-Zip or unpack it and choose the project folder.");
		return QString();
	}
	QProcess p;
	p.start(seven, {QStringLiteral("x"), QStringLiteral("-y"), QStringLiteral("-o") + QDir::toNativeSeparators(dir),
					QDir::toNativeSeparators(zw1), QStringLiteral("Function.eod"), QStringLiteral("Instance.eod")});
	if (!p.waitForFinished(300000) || p.exitCode() != 0) {
		if (error) *error = QCoreApplication::translate("EplanProject", "7-Zip could not unpack %1.").arg(QFileInfo(zw1).fileName());
		return QString();
	}
	return dir;
}

EplanProject::Layout EplanProject::readLayout(const QString &path)
{
	Layout out;
	QTemporaryDir tmp;
	QString dir;
	const QFileInfo fi(path);
	if (fi.isFile() && fi.suffix().compare(QLatin1String("zw1"), Qt::CaseInsensitive) == 0)
		dir = unpack(path, tmp.path(), &out.error);
	else if (fi.isFile()) dir = fi.absolutePath();
	else if (fi.isDir()) {
		dir = fi.absoluteFilePath();
		if (!QFileInfo::exists(dir + QStringLiteral("/Function.eod"))) {
			QDirIterator it(dir, {QStringLiteral("Function.eod")}, QDir::Files, QDirIterator::Subdirectories);
			if (it.hasNext()) dir = QFileInfo(it.next()).absolutePath();
		}
	}
	if (dir.isEmpty()) return out;
	QFile ff(dir + QStringLiteral("/Function.eod")), fi2(dir + QStringLiteral("/Instance.eod"));
	if (!ff.open(QIODevice::ReadOnly) || !fi2.open(QIODevice::ReadOnly)) {
		out.error = QCoreApplication::translate("EplanProject", "No EPLAN project tables (Function.eod, Instance.eod) in %1.").arg(path);
		return out;
	}
	const QByteArray fb = ff.readAll(), ib = fi2.readAll();

		// the functions with a place on a page
	QList<FunctionRecord> recs;
	for (int at = fb.indexOf(function_start); at >= 0; ) {
		const int next = fb.indexOf(function_start, at + 6);
		const QByteArray r = fb.mid(at, (next < 0 ? fb.size() : next) - at).left(6000);
		at = next;
		FunctionRecord f;
		const int pg = r.indexOf(page_tag);
		if (pg < 0 || pg + 8 > r.size()) continue;
		f.page = u32(r, pg + 4);
			// the full designation of this record (the last record of the
			// table runs on into other data: the first one after the page)
		const int idm = r.indexOf(QByteArray::fromHex("f6ea"), pg);
		if (idm < 0) continue;
		f.id = fields(r, idm + 2);
		if (!f.id.contains(20013)) continue;
		f.has_ins = point(r, QByteArray::fromHex("5aed"), &f.ins);
		f.has_box = point(r, QByteArray::fromHex("d3f0"), &f.lo) && point(r, QByteArray::fromHex("d4f0"), &f.hi);
		recs << f;
	}
		// the layout page: the one with the mounting plate (identifier MP)
		// and the most outlined devices
	QMap<quint32, int> plates;
	for (const FunctionRecord &f : std::as_const(recs))
		if (f.id.value(20013).startsWith(QLatin1String("MP"))) plates[f.page] += 1;
	if (plates.isEmpty()) {
		out.error = QCoreApplication::translate("EplanProject", "The project has no panel layout page with a mounting plate.");
		return out;
	}
	quint32 page = plates.firstKey();
	int best = -1;
	for (auto it = plates.constBegin(); it != plates.constEnd(); ++it) {
		int n = 0;
		for (const FunctionRecord &f : std::as_const(recs)) if (f.page == it.key() && f.has_box) ++n;
		if (n > best) {best = n; page = it.key();}
	}

		// the zero point: the corner of the mounting plate macro (…MPL) on that page
	QByteArray page_bytes = page_tag;
	const quint32 page_le = qToLittleEndian(page);
	page_bytes.append(reinterpret_cast<const char *>(&page_le), 4);
	QPointF origin;
	bool have_origin = false;
	QList<QPair<QString, QPointF>> page_texts;
	for (int at = ib.indexOf(record_start); at >= 0; ) {
		const int next = ib.indexOf(record_start, at + 4);
		const QByteArray r = ib.mid(at, (next < 0 ? ib.size() : next) - at).left(4000);
		at = next;
		if (!r.left(48).contains(page_bytes)) continue;
		const int t = r.indexOf(QByteArray::fromHex("5fec"));
		if (t < 0) continue;
		const int end = r.indexOf('\0', t + 2);
		if (end < 0) continue;
		const QByteArray text = r.mid(t + 2, end - t - 2);
		QPointF pos;
		if (!point(r, QByteArray::fromHex("55ec"), &pos)) continue;
		if (text.endsWith("MPL;") && !have_origin) {origin = pos; have_origin = true; continue;}
			// "??_??@PCSS-A1;" -> PCSS-A1
		QString plain = QString::fromLatin1(text);
		plain = plain.section(QLatin1Char('@'), -1).section(QLatin1Char(';'), 0, 0).trimmed().toUpper();
		if (!plain.isEmpty()) page_texts << qMakePair(plain, pos);
	}
	if (!have_origin) {
		out.error = QCoreApplication::translate("EplanProject", "The zero point of the mounting plate was not found on the layout page.");
		return out;
	}

	for (const FunctionRecord &f : std::as_const(recs)) {
		if (f.page != page || !f.has_ins) continue;
		const QString kind = f.id.value(20013);
		if (kind.startsWith(QLatin1String("MP"))) continue;
		Placement p;
		p.plant = f.id.value(1100);
		p.location = f.id.value(1200);
		p.bmk = QLatin1Char('-') + kind + f.id.value(20014);
		if (f.id.contains(20015)) p.bmk += QLatin1Char('.') + f.id.value(20015);
		const QPointF at = f.ins - origin;
		if (f.has_box) p.rect_mm = QRectF(at + f.lo, at + f.hi).normalized();
		else {
			p.centre_only = true;
			p.rect_mm = QRectF(at, QSizeF(0, 0));
		}
		out.items << p;
	}
	for (const auto &t : std::as_const(page_texts)) out.texts << qMakePair(t.first, t.second - origin);
	return out;
}
