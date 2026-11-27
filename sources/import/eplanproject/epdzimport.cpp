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
#include "epdzimport.h"

#include "../../diagram.h"
#include "../../qetgraphicsitem/diagramimageitem.h"
#include "../../qetproject.h"
#include "../../qetresult.h"
#include "../../titleblockproperties.h"
#include "../../undocommand/addgraphicsobjectcommand.h"
#include "../edz/edzsevenzip.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QImage>
#include <QPainter>
#include <QSaveFile>
#include <QSet>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSvgRenderer>
#include <QTemporaryDir>
#include <QUuid>

namespace {

	/// a CSV cell: quoted when it holds the separator, a quote or a break
QString cell(QString text)
{
	text.replace(QLatin1Char('\n'), QLatin1Char(' ')).replace(QLatin1Char('\r'), QLatin1Char(' '));
	if (text.contains(QLatin1Char(';')) || text.contains(QLatin1Char('"'))) {
		text.replace(QStringLiteral("\""), QStringLiteral("\"\""));
		return QLatin1Char('"') + text + QLatin1Char('"');
	}
	return text;
}

bool writeText(const QString &path, const QString &text)
{
	QSaveFile f(path);
	if (!f.open(QIODevice::WriteOnly)) return false;
	f.write("\xEF\xBB\xBF");   // UTF-8 with BOM: Excel reads umlauts
	f.write(text.toUtf8());
	return f.commit();
}

	/// the value without the prefix EPLAN writes in front ("==EES" -> "EES")
QString bare(QString value)
{
	value = value.trimmed();
	while (!value.isEmpty() && (value.at(0) == QLatin1Char('=') || value.at(0) == QLatin1Char('+')
								|| value.at(0) == QLatin1Char('#') || value.at(0) == QLatin1Char('&')))
		value.remove(0, 1);
	return value.trimmed();
}

} // namespace

EpdzImport::Result EpdzImport::run(const QString &epdz, const QString &target_qet)
{
	Result r;
	QTemporaryDir dir;
	if (!dir.isValid()) {r.error = QObject::tr("No temporary folder"); return r;}
	QString error;
	if (!sevenZipExtract(epdz, dir.path(), error)) {
		r.error = QObject::tr("%1 cannot be unpacked: %2").arg(QFileInfo(epdz).fileName(), error);
		return r;
	}
		// every file of the archive by its name (the manifest gives names only)
	QHash<QString, QString> files;
	for (QDirIterator it(dir.path(), QDir::Files, QDirIterator::Subdirectories); it.hasNext();) {
		const QString f = it.next();
		files.insert(QFileInfo(f).fileName().toLower(), f);
	}
	const QString manifest = files.value(QStringLiteral("manifest.db"));
	if (manifest.isEmpty()) {
		r.error = QObject::tr("%1 holds no manifest.db: no EPLAN export archive").arg(QFileInfo(epdz).fileName());
		return r;
	}

	struct Page {qint64 id = 0; QString name, title, folio, plant, location, svg;};
	QList<Page> pages;
	QStringList device_rows, connection_rows;
	QList<QPair<QString, QString>> spaces;   // name, .e3d file
	QString project_name;
	const QString connection = QStringLiteral("epdz_") + QUuid::createUuid().toString(QUuid::WithoutBraces);
	{
		QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connection);
		db.setDatabaseName(manifest);
		db.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
		if (!db.open()) {
			r.error = QObject::tr("manifest.db cannot be read");
		} else {
			auto query = [&](const QString &sql) {QSqlQuery q(db); q.exec(sql); return q;};
			QSqlQuery q = query(QStringLiteral("SELECT name FROM package WHERE type='project' LIMIT 1"));
			if (q.next()) project_name = q.value(0).toString();

			QHash<qint64, QStringList> locators;
			q = query(QStringLiteral("SELECT packageid, locator FROM item"));
			while (q.next()) locators[q.value(0).toLongLong()] << q.value(1).toString();

			QHash<qint64, QString> counter, description;
			q = query(QStringLiteral("SELECT packageid, propid, value FROM property WHERE propid IN (11000, 11011) "
									 "AND value IS NOT NULL AND value != ''"));
			while (q.next()) {
				if (q.value(1).toInt() == 11000) counter.insert(q.value(0).toLongLong(), q.value(2).toString());
				else description.insert(q.value(0).toLongLong(), q.value(2).toString());
			}

				// the columns this manifest has (older ones lack some)
			QSet<QString> columns;
			q = query(QStringLiteral("PRAGMA table_info(page_package)"));
			while (q.next()) columns << q.value(1).toString().toLower();
			auto col = [&](const char *name) {
				return columns.contains(QString::fromLatin1(name))
					? QStringLiteral("COALESCE(%1,'')").arg(QString::fromLatin1(name)) : QStringLiteral("''");
			};
			q = query(QStringLiteral("SELECT packageid, name, %1, %2, %3, %4, %5 FROM page_package")
						  .arg(col("ep1340"), col("ep1440"), col("ep1140"), col("ep1240"), col("ep1640")));
			while (q.next()) {
				Page p;
				p.id = q.value(0).toLongLong();
				p.name = q.value(1).toString();
				const QString higher = bare(q.value(2).toString()), plant = bare(q.value(3).toString());
				const QString site = bare(q.value(4).toString()), location = bare(q.value(5).toString());
				p.plant = plant.isEmpty() ? higher : plant;
				p.location = location.isEmpty() ? site : location;
				p.folio = bare(q.value(6).toString());
				if (p.folio.isEmpty()) p.folio = counter.value(p.id);
				p.title = description.value(p.id);
				if (p.title.isEmpty()) p.title = p.name;
				for (const QString &l : locators.value(p.id))
					if (l.endsWith(QLatin1String(".svg"), Qt::CaseInsensitive)) {
						p.svg = files.value(QFileInfo(l).fileName().toLower());
						if (QFileInfo(l).completeBaseName().compare(p.name, Qt::CaseInsensitive) == 0) break;
					}
				pages << p;
			}

				// the functions with their designation and two properties
			q = query(QStringLiteral(
				"SELECT k.name, "
				"(SELECT p.value FROM property p WHERE p.packageid = k.id AND p.propid = 20001 AND p.value != '' ORDER BY p.propindex LIMIT 1), "
				"(SELECT p.value FROM property p WHERE p.packageid = k.id AND p.propid = 20100 AND p.value != '' ORDER BY p.propindex LIMIT 1), "
				"(SELECT p.value FROM property p WHERE p.packageid = k.id AND p.propid = 20193 AND p.value != '' ORDER BY p.propindex LIMIT 1) "
				"FROM package k WHERE k.type = 'function'"));
			while (q.next())
				device_rows << QStringList{cell(q.value(0).toString()), cell(q.value(1).toString()),
										   cell(q.value(2).toString()), cell(q.value(3).toString())}.join(QLatin1Char(';'));

				// the connections: source, target, potential, colour, section, length
			auto first = [](int id) {
				return QStringLiteral("(SELECT p.value FROM property p WHERE p.packageid = k.id AND p.propid = %1 "
									  "AND p.value != '' ORDER BY p.propindex LIMIT 1)").arg(id);
			};
			q = query(QStringLiteral("SELECT k.name, %1, %2, %3, %4, %5, %6, %7 FROM package k WHERE k.type = 'mergedconnection'")
						  .arg(first(31019), first(31020), first(33003), first(20045), first(31004), first(31007), first(31003)));
			while (q.next()) {
				const QString potential = q.value(3).toString().isEmpty() ? q.value(4).toString() : q.value(3).toString();
				connection_rows << QStringList{cell(q.value(1).toString()), cell(q.value(2).toString()), cell(potential),
											   cell(q.value(5).toString()), cell(q.value(6).toString()),
											   cell(q.value(7).toString())}.join(QLatin1Char(';'));
			}

			q = query(QStringLiteral("SELECT packageid, name FROM installationspace_package"));
			while (q.next())
				for (const QString &l : locators.value(q.value(0).toLongLong()))
					if (l.endsWith(QLatin1String(".e3d"), Qt::CaseInsensitive))
						spaces << qMakePair(q.value(1).toString(), files.value(QFileInfo(l).fileName().toLower()));
			db.close();
		}
	}
	QSqlDatabase::removeDatabase(connection);
	if (!r.error.isEmpty()) return r;
	if (pages.isEmpty()) {r.error = QObject::tr("The manifest lists no page"); return r;}

		// the project: one folio per page, the page as a picture
	QETProject project;
	project.setTitle(project_name.isEmpty() ? QFileInfo(epdz).completeBaseName() : project_name);
	for (const Page &p : std::as_const(pages)) {
		Diagram *d = project.addNewDiagram();
		if (!d) continue;
		TitleBlockProperties tb = d->border_and_titleblock.exportTitleBlock();
		tb.title = p.title;
		tb.folio = p.folio;
		tb.plant = p.plant;
		tb.locmach = p.location;
		d->border_and_titleblock.importTitleBlock(tb);
		++r.pages;
		if (p.svg.isEmpty()) {r.notes << QObject::tr("Page %1: no drawing in the archive").arg(p.name); continue;}
		QSvgRenderer svg(p.svg);
		if (!svg.isValid()) {r.notes << QObject::tr("Page %1: the drawing cannot be read").arg(p.name); continue;}
		const QRectF area = d->border_and_titleblock.insideBorderRect();
		QSizeF size = svg.viewBoxF().size();
		if (size.isEmpty()) size = svg.defaultSize();
		if (size.isEmpty()) continue;
			// drawn at three times the folio's resolution, so it stays sharp when zoomed
		const double fit = qMin(area.width() / size.width(), area.height() / size.height());
		const double k = 3.0;
		QImage image((size * fit * k).toSize(), QImage::Format_ARGB32_Premultiplied);
		image.fill(Qt::white);
		{
			QPainter painter(&image);
			painter.setRenderHint(QPainter::Antialiasing);
			svg.render(&painter, QRectF(QPointF(0, 0), size * fit * k));
		}
		auto *item = new DiagramImageItem(QPixmap::fromImage(image));
		d->undoStack().push(new AddGraphicsObjectCommand(item, d, area.topLeft()));
		item->setScaleFactorX(1.0 / k);
		item->setScaleFactorY(1.0 / k);
			// the picture scales about its centre: put its corner back on the folio's
		item->setPos(item->pos() + (area.topLeft() - item->sceneBoundingRect().topLeft()));
	}

		// the lists and the 3D beside the project
	const QFileInfo target(target_qet);
	const QString base = target.absoluteDir().filePath(target.completeBaseName());
	if (!device_rows.isEmpty()) {
		writeText(base + QStringLiteral("_Geraete.csv"),
				  QStringLiteral("Funktion;BMK (20001);Eigenschaft 20100;Eigenschaft 20193\n")
					  + device_rows.join(QLatin1Char('\n')) + QLatin1Char('\n'));
		r.functions = int(device_rows.size());
	}
	if (!connection_rows.isEmpty()) {
		writeText(base + QStringLiteral("_Verbindungen.csv"),
				  QStringLiteral("Quelle;Ziel;Potential;Farbe;Querschnitt;Laenge\n")
					  + connection_rows.join(QLatin1Char('\n')) + QLatin1Char('\n'));
		r.connections = int(connection_rows.size());
	}
	if (!spaces.isEmpty()) {
		const QString dir3d = base + QStringLiteral("_3D");
		QDir().mkpath(dir3d);
		for (const auto &s : std::as_const(spaces)) {
			if (s.second.isEmpty()) continue;
			const QString to = QDir(dir3d).filePath(QFileInfo(s.second).fileName());
			QFile::remove(to);
			if (QFile::copy(s.second, to)) ++r.spaces;
		}
	}

	project.setFilePath(target_qet);
	const QETResult written = project.write();
	if (!written.isOk()) {r.error = written.errorMessage(); return r;}
	r.notes.prepend(QObject::tr("%1 page(s), %2 function(s), %3 connection(s), %4 3D installation space(s)")
						.arg(r.pages).arg(r.functions).arg(r.connections).arg(r.spaces));
	r.ok = true;
	return r;
}
