/*
	Copyright 2006-2025 The QElectroTech Team
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
#include "cli_export.h"
#include "revision/projectrevisions.h"
#include "qetresult.h"
#include "projectlock.h"
#include "bomexport.h"
#include "marking/markingexport.h"
#include "marking/thmclient.h"
#include "marking/thmjob.h"
#include "marking/thmlayouts.h"
#include "marking/thmprinters.h"
#include "purchase/purchasefetch.h"
#include "purchase/purchaselist.h"
#include "qetapp.h"
#include <QTranslator>
#include "marking/mprintpro.h"

#include "bordertitleblock.h"
#include "conductornumexport.h"
#include "conductorproperties.h"
#include "dataBase/projectdatabase.h"
#include "dataBase/qetdbfile.h"
#include "dataBase/sqlreadonly.h"
#include "diagram.h"
#include "diagramcontext.h"
#include "devicetype/devicetypecheck.h"
#include "projectcheck.h"
#include "devicetype/devicetypelist.h"
#include "editor/symbolgridcheck.h"
#include "editor/terminalnamecheck.h"
#include "fluid/fluiddata.h"
#include "simulation/simulationmodel.h"
#include "simulation/simulationcatalog.h"
#include "qetversion.h"
#include <QCryptographicHash>
#include <QElapsedTimer>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "simulation/simulationoverlay.h"
#include "simulation/simulationreference.h"
#include "import/eplandxf/eplanpartimporter.h"
#include "import/portals/manufacturerportals.h"
#include "import/portals/partfetcher.h"
#include "import/portals/portalcredentials.h"
#include "dxfexport.h"
#include "exportproperties.h"
#include "pdf_links.h"
#include "container/qetcontainer.h"
#include "qetgraphicsitem/conductor.h"
#include "qetgraphicsitem/diagramimageitem.h"
#include "qetgraphicsitem/element.h"
#include "qetgraphicsitem/terminal.h"
#include "qetproject.h"
#include "titleblockproperties.h"
#include "wiringlistdb.h"
#include "wiringlistexport.h"
#include "plc/tiaplctags.h"

// Private Qt PDF engine for drawHyperlink() — see pdf_links / projectprintwindow.
#include <private/qpdf_p.h>

#include <QDir>
#include <QDirIterator>
#include <QDomDocument>
#include <QDate>
#include <QDateTime>
#include <QFile>
#include <QSaveFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QPageLayout>
#include <QPair>
#include <QPainter>
#include <QPdfWriter>
#include <QSet>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSvgGenerator>
#include <QTemporaryDir>
#include <QTextStream>
#include <QTimeZone>
#include <QTransform>
#include <QUuid>

namespace {

QTextStream out(stdout);
QTextStream err(stderr);

/// All CLI option flags, mapped to a short format name.
const QHash<QString, QString> &exportFlags()
{
	static const QHash<QString, QString> flags {
		{"--export-pdf", "pdf"},
		{"--export-png", "png"},
		{"--export-svg", "svg"},
		{"--export-dxf", "dxf"},
		{"--export-cables", "cables"},
		{"--export-wires", "wires"},
		{"--export-bom", "bom"},
		{"--export-marking", "marking"},
		{"--thm-prn", "thmprn"},
		{"--thm-status", "thmstatus"},
		{"--thm-send", "thmsend"},
		{"--thm-printers", "thmprinters"},
		{"--thm-load", "thmload"},
		{"--thm-transport", "thmtransport"},
		{"--export-purchase", "purchase"},
		{"--fetch-commercial", "fetchcommercial"},
		{"--export-wiring", "wiring"},
		{"--export-nets", "nets"},
		{"--export-links", "links"},
		{"--export-tia-tags", "tiaexport"},
		{"--compare-tia-tags", "tiacompare"},
		{"--check-device-types", "devicetypes"},
		{"--check-project", "checkproject"},
		{"--export-device-types", "devicetypelist"},
		{"--export-device-types-brief", "devicetypebrief"},
		{"--info", "info"},
		{"--check-elements", "check"},
		{"--import-eplan", "importeplan"},
		{"--fetch-parts", "fetchparts"},
		{"--portal-status", "portalstatus"},
		{"--portal-login", "portallogin"},
		{"--portal-logout", "portallogout"},
		{"--portal-request", "portalrequest"},
		{"--resave", "resave"},
		{"--lock-status", "lockstatus"},
		{"--recover", "recover"},
		{"--history", "history"},
		{"--history-on", "historyon"},
		{"--history-off", "historyoff"},
		{"--history-diff", "historydiff"},
		{"--last-change", "lastchange"},
		{"--history-keep", "historykeep"},
		{"--qetdb-repair", "qetdbrepair"},
		{"--revision", "revision"},
		{"--export-delivery", "delivery"},
		{"--from-database", "fromdb"},
		{"--set-titleblock", "settb"},
		{"--query", "query"},
		{"--simulate", "simulate"},
		{"--simulate-reference", "simreference"},
		{"--simulate-catalog", "simcatalog"},
		{"--close-revision", "closerevision"},
		{"--revisions", "revisions"},
		{"--revision-marks", "revisionmarks"},
		{"--compare-revisions", "comparerevisions"},
	};
	return flags;
}

/// Device tag of an element ("K1", "Q55"), falling back to its name.
QString elementLabel(Element *element)
{
	const QString label = element->elementInformations()["label"].toString();
	return label.isEmpty() ? element->name() : label;
}

/// Pixel rect of a diagram's border + title block (the printable page area).
QRect diagramRect(Diagram *diagram)
{
	QRectF r = diagram->border_and_titleblock.borderAndTitleBlockRect();
	r.adjust(0, 0, 1, 1); // include the 1px border line
	return r.toAlignedRect();
}

/// A filesystem-safe per-diagram file stem: "01_Title".
QString diagramStem(Diagram *diagram, int index)
{
	QString title = diagram->title();
	title.replace(QRegularExpression("[^\\w \\-]"), "_");
	title = title.simplified();
	if (title.isEmpty())
		title = "diagram";
	return QStringLiteral("%1_%2")
		.arg(index, 2, 10, QChar('0'))
		.arg(title);
}

/// Render @p diagram into @p painter, fitting @p target to the page rect.
/// @p showTerminals: paint terminal markers (red stroke + blue docking
/// dot) and terminal names, as the interactive editor does. Off by
/// default — Terminal::paint() draws them whenever the diagram's
/// drawTerminals()/drawTerminalNames() flags are set (default true, so
/// headless export used to ship them as editor UI); the GUI export
/// dialog already defaults to clearing them via
/// Diagram::applyProperties(). Pass true (--show-terminals) to keep
/// them, e.g. to visually debug an unconnected pin.
void renderDiagram(Diagram *diagram, QPainter &painter, const QRectF &target,
					bool showTerminals = false)
{
	const QRect source = diagramRect(diagram);
	// Export without the editor grid: drawBackground() only paints it when
	// draw_grid_ is set (default true), so toggle it off around the render
	// and restore it afterwards.
	const bool was_drawing_grid = diagram->displayGrid();
	const bool was_drawing_guides = diagram->displayGuides();
	const bool was_drawing_terminals = diagram->drawTerminals();
	const bool was_drawing_terminal_names = diagram->drawTerminalNames();
	diagram->setDisplayGrid(false);
	diagram->setDisplayGuides(false);
	diagram->setDrawTerminals(showTerminals);
	diagram->setDrawTerminalNames(showTerminals);
	diagram->render(&painter, target, source, Qt::KeepAspectRatio);
	diagram->setDisplayGrid(was_drawing_grid);
	diagram->setDisplayGuides(was_drawing_guides);
	diagram->setDrawTerminals(was_drawing_terminals);
	diagram->setDrawTerminalNames(was_drawing_terminal_names);
}

/// The time SOURCE_DATE_EPOCH names, in seconds since 1970 UTC, or an
/// invalid QDateTime when it is unset. A value that is not a whole number of
/// seconds is reported and ignored.
QDateTime sourceDateEpoch()
{
	const QByteArray value = qgetenv("SOURCE_DATE_EPOCH");
	if (value.isEmpty())
		return {};
	bool ok = false;
	const qlonglong seconds = value.toLongLong(&ok);
	if (!ok || seconds < 0) {
		err << "SOURCE_DATE_EPOCH '" << value << "' is not a number of seconds; ignored.\n";
		return {};
	}
	return QDateTime::fromSecsSinceEpoch(seconds, QTimeZone::utc());
}

int exportPdf(QETProject &project, const QString &output,
			 bool showTerminals = false)
{
	const QList<Diagram *> diagrams = project.diagrams();
	if (diagrams.isEmpty()) {
		err << "No diagrams to export.\n";
		return 1;
	}

	// Page numbers (1-based) for cross-reference hyperlink targets: each
	// diagram is exactly one page in the CLI export (no tiling).
	QMap<Diagram *, int> pageMap;
	for (int i = 0; i < diagrams.size(); ++i)
		pageMap.insert(diagrams.at(i), i + 1);

	QPdfWriter writer(output);
	writer.setCreator("QElectroTech");
	writer.setResolution(96);

	// SOURCE_DATE_EPOCH (reproducible-builds.org) asks for the same file
	// from the same input: the time it names instead of now, and a document
	// id from what the PDF shows instead of a random one. Qt has no setter
	// for the dates, and the content is not known yet, so both are
	// rewritten once the file is written; Qt writes a fixed id until then.
	// The id does not come from the project file, whose uuids are new each
	// time a project is generated again from the same data. Before Qt 6.8
	// there is no document id to set: it is only written for PDF/A.
	const QDateTime sourceDate = sourceDateEpoch();
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
	if (sourceDate.isValid())
		writer.setDocumentId(PdfLinks::placeholderDocumentId());
#endif

	QPainter painter;
	bool first = true;
	for (Diagram *diagram : diagrams) {
		const QRect r = diagramRect(diagram);
		// Match the page to the diagram (in points: 1px @ 96dpi = 0.75pt).
		// QPageSize rounds a size within 3 pt of a standard sheet to the
		// sheet, but knows the sheets upright only (bar Ledger), so a wide
		// folio is matched upright and turned: otherwise an A3 landscape
		// folio became a 1190 x 841 pt page while an A3 portrait one was
		// 842 x 1191, the sheet.
		QSizeF points(r.width() * 72.0 / 96.0, r.height() * 72.0 / 96.0);
		const bool wide = points.width() > points.height();
		if (wide) points.transpose();
		writer.setPageLayout(QPageLayout(QPageSize(points, QPageSize::Point),
										 wide ? QPageLayout::Landscape
											  : QPageLayout::Portrait,
										 QMarginsF(0, 0, 0, 0)));

		if (first) {
			if (!painter.begin(&writer)) {
				err << "Cannot open '" << output << "' for writing.\n";
				return 1;
			}
			first = false;
		} else {
			writer.newPage();
		}
		const QRectF target(0, 0,
							writer.width(), writer.height());
		renderDiagram(diagram, painter, target, showTerminals);

		// Inject clickable cross-reference / folio-report hyperlinks for this
		// page.  The geometry is rebuilt from the QPdfWriter (not a QPrinter):
		// render() anchors the diagram top-left with KeepAspectRatio, and the
		// page is sized to the diagram so the scale is ~1.
		if (auto *engine = dynamic_cast<QPdfEngine *>(painter.paintEngine())) {
			const QRectF source(r);
			const qreal s = qMin(target.width()  / source.width(),
								 target.height() / source.height());
			QTransform fit;
			fit.translate(target.x(), target.y());
			fit.scale(s, s);
			fit.translate(-source.x(), -source.y());

			// Device pixels -> PDF points, replicating the engine's page matrix
			// (72/resolution scale + Y flip; zero margins -> no paint offset).
			const qreal pt_scale = 72.0 / writer.resolution();
			const qreal fullH_pt = writer.pageLayout().fullRectPoints().height();
			const bool  fullPageMode =
				(writer.pageLayout().mode() == QPageLayout::FullPageMode);
			const QRect paintPx =
				writer.pageLayout().paintRectPixels(writer.resolution());

			PdfLinks::PageGeometry geom;
			geom.sceneToDevice = fit;
			geom.target        = target;
			geom.pageBounds    = QRectF(0, 0, target.width(), target.height());
			geom.devToPdf = [=](const QPointF &d) -> QPointF {
				qreal dx = d.x(), dy = d.y();
				if (!fullPageMode) { dx += paintPx.left(); dy += paintPx.top(); }
				return QPointF(pt_scale * dx, fullH_pt - pt_scale * dy);
			};
			geom.sourceRectOf = [](Diagram *dg) {
				return QRectF(diagramRect(dg));
			};
			PdfLinks::injectCrossRefLinks(engine, diagram, geom, pageMap, output);
		}
	}
	painter.end();

	// Rewrite the URI link annotations into native internal GoTo actions, so
	// the cross-references jump inside the document in any PDF viewer.
	PdfLinks::convertUriToGoTo(output);
	PdfLinks::removeUnusedPdfxNamespace(output);
	if (sourceDate.isValid()) {
		PdfLinks::setDocumentDate(output, sourceDate);
		PdfLinks::setDocumentIdFromContent(output);
	}

	out << "Exported " << diagrams.size() << " page(s) -> " << output << "\n";
	return 0;
}

int exportImages(QETProject &project, const QString &format,
				 const QString &out_dir, bool showTerminals = false,
				 bool svgSmallPictures = false)
{
	const QList<Diagram *> diagrams = project.diagrams();
	if (diagrams.isEmpty()) {
		err << "No diagrams to export.\n";
		return 1;
	}
	QDir().mkpath(out_dir);

	int index = 0;
	for (Diagram *diagram : diagrams) {
		++index;
		const QRect r = diagramRect(diagram);
		const QString path = QDir(out_dir).filePath(
			diagramStem(diagram, index) + "." + format);

		if (format == "svg") {
			QSvgGenerator gen;
			gen.setFileName(path);
			gen.setSize(r.size());
			gen.setViewBox(QRect(0, 0, r.width(), r.height()));
			gen.setTitle(diagram->title());
			QPainter painter(&gen);
			diagram->setSmallPictures(svgSmallPictures);
			renderDiagram(diagram, painter, QRectF(QPointF(0, 0), r.size()),
						 showTerminals);
			diagram->setSmallPictures(false);
			painter.end();
		} else { // png
			QImage image(r.size(), QImage::Format_ARGB32);
			image.fill(Qt::white);
			QPainter painter(&image);
			painter.setRenderHint(QPainter::Antialiasing, true);
			renderDiagram(diagram, painter, QRectF(QPointF(0, 0), r.size()),
						 showTerminals);
			painter.end();
			if (!image.save(path)) {
				err << "Failed to write '" << path << "'.\n";
				return 1;
			}
		}
		out << "  " << path << "\n";
	}
	out << "Exported " << diagrams.size() << " diagram(s) -> " << out_dir << "\n";
	return 0;
}

/// One DXF file per diagram, written by the same code as the export dialog
/// (DxfExport) with the dialog's default options -- the export settings of
/// the preferences -- so both give the same file.
int exportDxf(QETProject &project, const QString &out_dir, bool showTerminals,
			  bool dxfBlocks, bool dxfAttributes)
{
	const QList<Diagram *> diagrams = project.diagrams();
	if (diagrams.isEmpty()) {
		err << "No diagrams to export.\n";
		return 1;
	}
	QDir().mkpath(out_dir);

	ExportProperties properties = ExportProperties::defaultExportProperties();
	properties.format = QStringLiteral("DXF");
	if (showTerminals)
		properties.draw_terminals = true;
	if (dxfBlocks || dxfAttributes)
		properties.dxf_blocks = true;
	if (dxfAttributes)
		properties.dxf_attributes = true;

	int index = 0;
	bool has_images = false;
	for (Diagram *diagram : diagrams) {
		++index;
		const QString path = QDir(out_dir).filePath(
			diagramStem(diagram, index) + ".dxf");

		// Createdxf answers a file it cannot open with a message box and
		// exit(0), which headless means no message and a success code.
		QFile probe(path);
		if (!probe.open(QIODevice::WriteOnly)) {
			err << "Cannot open '" << path << "' for writing.\n";
			return 1;
		}
		probe.close();

		const QSize size = DxfExport::folioSize(diagram, properties);
		DxfExport::write(diagram, size.width(), size.height(), path, properties);

		for (QGraphicsItem *item : diagram->items()) {
			if (qgraphicsitem_cast<DiagramImageItem *>(item)) {
				has_images = true;
				break;
			}
		}
		out << "  " << path << "\n";
	}
	if (has_images)
		err << "Note: DXF has no pictures in this format; "
			   "each picture is exported as an outline box.\n";
	out << "Exported " << diagrams.size() << " diagram(s) -> " << out_dir << "\n";
	return 0;
}

int exportCsv(QETProject &project, const QString &format, const QString &output)
{
	QString csv;
	if (format == "cables") {
		WiringListExport wle(&project, nullptr);
		csv = wle.toCsvString();
	} else { // wires
		ConductorNumExport cne(&project, nullptr);
		csv = cne.wiresNum();
	}
	if (csv.isEmpty()) {
		err << "Nothing to export (empty list).\n";
		return 1;
	}

	QFile file(output);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		err << "Cannot open '" << output << "' for writing.\n";
		return 1;
	}
	QTextStream fout(&file);
	fout << csv;
	file.close();
	out << "Exported " << format << " list -> " << output << "\n";
	return 0;
}

/// Bill of materials from the same project database and default query as the
/// GUI nomenclature export.
int exportBom(QETProject &project, const QString &output,
			  bool includeSlaves, bool includeJunctions)
{
	project.dataBase()->updateDB();
	QSqlQuery query = project.dataBase()->newQuery(
			BomExport::defaultQuery(includeSlaves, includeJunctions));
	if (!query.exec()) {
		err << "BOM query failed: " << query.lastError().text() << "\n";
		return 1;
	}
	int rows = 0;
	const auto csv = BomExport::toCsv(
			query, BomExport::defaultColumns(), true, &rows);
	QString error;
	if (!BomExport::writeCsv(output, csv, &error)) {
		err << "Cannot write '" << output << "': " << error << "\n";
		return 1;
	}
	out << "Exported " << rows << " component(s) -> " << output << "\n";
	return 0;
}

/// Markers for the marking software of the terminal makers, the same file
/// Project > Export markers... writes. @p rest: the project, the output and
/// the options, in any order after the two files.
int exportMarking(const QStringList &rest)
{
	QStringList positional;
	MarkingExport::Options o;
	const QString usage = QStringLiteral(
		"Usage: qelectrotech --export-marking <project.qet> <output.csv> "
		"[--format phoenix|weidmueller|wago|neutral] "
		"[--kinds wires,terminals,devices,cables] [--folios N[-M]] "
		"[--plant P] [--location L] [--short] [--one-end] "
		"[--printer printjet|thm] [--material Klemme=N,Draht=N,...] "
		"[--mprint [--open] [--print] [--mprint-exe PATH]]\n");
		//Weidmüller: the printer's materials, and with --mprint one file
		//per material for M-Print PRO's command line instead of one file
	MPrintPro::Settings mprint = MPrintPro::load();
	bool mprint_jobs = false, mprint_open = false;
	QString material_text;  //read after --printer, whatever the order
	for (int i = 0; i < rest.size(); ++i) {
		const QString &a = rest.at(i);
		const auto value = [&]() { return i + 1 < rest.size() ? rest.at(++i) : QString(); };
		if (a == QLatin1String("--printer")) {
			if (!MPrintPro::printerFromKey(value(), &mprint.printer)) {
				err << "Unknown --printer; expected printjet or thm.\n";
				return 2;
			}
			for (QString &m : mprint.materials) m.clear();
			o.format = MarkingExport::Weidmueller;
		} else if (a == QLatin1String("--material")) {
			material_text += value() + QLatin1Char(',');
			o.format = MarkingExport::Weidmueller;
		} else if (a == QLatin1String("--mprint")) {
			mprint_jobs = true;
		} else if (a == QLatin1String("--open")) {
			mprint_open = true;
		} else if (a == QLatin1String("--print")) {
			mprint.print = true;
		} else if (a == QLatin1String("--mprint-exe")) {
			mprint.exe = value();
		} else if (a == QLatin1String("--format")) {
			if (!MarkingExport::formatFromKey(value(), &o.format)) {
				err << "Unknown --format; expected phoenix, weidmueller, wago or neutral.\n";
				return 2;
			}
		} else if (a == QLatin1String("--kinds")) {
			if (!MarkingExport::kindsFromKeys(value(), &o.kinds)) {
				err << "Unknown --kinds; expected wires, terminals, devices, cables or all.\n";
				return 2;
			}
		} else if (a == QLatin1String("--folios")) {
			const QStringList range = value().split(QLatin1Char('-'));
			bool ok_from = false, ok_to = true;
			o.folio_from = range.value(0).toInt(&ok_from);
			o.folio_to = range.size() > 1 ? range.value(1).toInt(&ok_to) : o.folio_from;
			if (!ok_from || !ok_to || o.folio_from < 1 || o.folio_to < o.folio_from) {
				err << "--folios takes a folio number or a range such as 2-5.\n";
				return 2;
			}
		} else if (a == QLatin1String("--plant")) {
			o.plant = value();
		} else if (a == QLatin1String("--location")) {
			o.location = value();
		} else if (a == QLatin1String("--short")) {
			o.full_designation = false;
		} else if (a == QLatin1String("--one-end")) {
			o.both_ends = false;
		} else if (a.startsWith(QLatin1String("--"))) {
			err << "Unknown option " << a << "\n" << usage;
			return 2;
		} else {
			positional << a;
		}
	}
	if (positional.size() < 2) {
		err << usage;
		return 2;
	}
	{
		QString error;
		if (!MPrintPro::parseMaterials(material_text, &mprint, &error)) {
			err << error << "\n";
			return 2;
		}
	}
	if (!QFileInfo::exists(positional.at(0))) {
		err << "Project not found: " << positional.at(0) << "\n";
		return 2;
	}
	QETProject project(positional.at(0));
	if (project.state() != QETProject::Ok) {
		err << "Failed to open project: " << positional.at(0) << "\n";
		return 1;
	}
	QString error;
	if (mprint_jobs || mprint_open) {
			//<output> is a folder: a file per material, and the command
			//that hands each to M-Print PRO
		const QList<MarkingExport::Item> items = MarkingExport::collect(&project, o, &error);
		if (!error.isEmpty()) {
			err << "Marking export failed: " << error << "\n";
			return 1;
		}
		const QString base = QFileInfo(positional.at(0)).completeBaseName();
		const QList<MPrintPro::Job> jobs = MPrintPro::writeJobs(items, mprint, positional.at(1), base, &error);
		if (jobs.isEmpty()) {
			err << (error.isEmpty() ? QStringLiteral("No markers to export.") : error) << "\n";
			return 1;
		}
		const QString exe = MPrintPro::findExe(mprint);
			//The command as it would be typed: arguments with spaces quoted
		const auto quoted = [](const QString &arg) {
			return arg.contains(QLatin1Char(' ')) ? QLatin1Char('"') + arg + QLatin1Char('"') : arg;
		};
		for (const MPrintPro::Job &job : jobs) {
			QStringList command {quoted(exe.isEmpty() ? QStringLiteral("MPrintPRO.exe") : exe)};
			for (const QString &arg : MPrintPro::arguments(job, mprint.print)) command << quoted(arg);
			out << job.markers << " marker(s) on " << job.material << " "
				<< (job.material_name.isEmpty() ? QStringLiteral("(own material)") : job.material_name)
				<< " -> " << job.file << "\n"
				<< "  " << command.join(QLatin1Char(' ')) << "\n";
		}
		if (mprint_open && !MPrintPro::open(jobs, mprint, &error)) {
			err << error << "\n";
			return 1;
		}
		return 0;
	}
	if (o.format == MarkingExport::Weidmueller) {
		o.materials = MPrintPro::materialsByKey(mprint);
	}
	int markers = 0;
	if (!MarkingExport::exportFile(&project, positional.at(1), o, &error, &markers)) {
		err << "Marking export failed: " << error << "\n";
		return 1;
	}
	out << "Exported " << markers << " marker(s) ("
		<< MarkingExport::formatKey(o.format) << ") -> " << positional.at(1) << "\n";
	return 0;
}

/// The texts of the THM commands in the language of the settings, as
/// the purchase list writes its headings; silent when none is found
void installThmTranslator()
{
	static QTranslator translator;
	static bool done = false;
	if (done) return;
	done = true;
	const QString language = QETApp::langFromSetting();
	for (const QString &dir : {QETApp::languagesPath(),
							   QCoreApplication::applicationDirPath() + QStringLiteral("/lang")}) {
		for (const QString &name : {language, language.section(QLatin1Char('_'), 0, 0)}) {
			if (translator.load(QStringLiteral("qet_") + name, dir) && !translator.isEmpty()) {
				QCoreApplication::installTranslator(&translator);
				return;
			}
		}
	}
}

/// What --thm-prn and --thm-send read: the markers chosen as for
/// --export-marking, the printer, and profile values that replace the
/// printer's own
struct ThmCliOptions
{
	QStringList positional;
	MarkingExport::Options marking;
	int copies = 1;
	bool merge = true;
	bool yes = false;
	QString printer_id;
	QString material;
	QString layouts;                   ///< --layouts: the layout file, else the settings'
	QString family;                    ///< --family: mmp or multimark
	bool usual = false;                ///< --usual (--thm-load)
	QString roller;                    ///< --roller (--thm-load)
	QHash<QString, QString> profile;   ///< option -> value
};

/// @return 0, or the exit code of a bad option (the message written)
int parseThmOptions(const QStringList &rest, ThmCliOptions *o)
{
	o->marking.kinds = MarkingExport::Wire;
	static const QStringList profile_options {
		QStringLiteral("--width"), QStringLiteral("--height"), QStringLiteral("--gap"),
		QStringLiteral("--sensor"), QStringLiteral("--columns"), QStringLiteral("--pitch"),
		QStringLiteral("--margin"), QStringLiteral("--text-size"), QStringLiteral("--font"),
		QStringLiteral("--baseline"), QStringLiteral("--speed"), QStringLiteral("--heat")};
	for (int i = 0; i < rest.size(); ++i) {
		const QString &a = rest.at(i);
		const auto value = [&]() { return i + 1 < rest.size() ? rest.at(++i) : QString(); };
		if (a == QLatin1String("--kinds")) {
			if (!MarkingExport::kindsFromKeys(value(), &o->marking.kinds)) {
				err << QCoreApplication::translate("ThmCli", "Unknown --kinds; expected wires, terminals, devices, cables or all.") << "\n";
				return 2;
			}
		} else if (a == QLatin1String("--folios")) {
			const QStringList range = value().split(QLatin1Char('-'));
			bool ok_from = false, ok_to = true;
			o->marking.folio_from = range.value(0).toInt(&ok_from);
			o->marking.folio_to = range.size() > 1 ? range.value(1).toInt(&ok_to) : o->marking.folio_from;
			if (!ok_from || !ok_to || o->marking.folio_from < 1 || o->marking.folio_to < o->marking.folio_from) {
				err << QCoreApplication::translate("ThmCli", "--folios takes a folio number or a range such as 2-5.") << "\n";
				return 2;
			}
		} else if (a == QLatin1String("--plant")) {
			o->marking.plant = value();
		} else if (a == QLatin1String("--location")) {
			o->marking.location = value();
		} else if (a == QLatin1String("--full")) {
			o->marking.full_designation = true;
		} else if (a == QLatin1String("--short")) {
			o->marking.full_designation = false;
		} else if (a == QLatin1String("--one-end")) {
			o->marking.both_ends = false;
		} else if (a == QLatin1String("--copies")) {
			bool ok = false;
			o->copies = value().toInt(&ok);
			if (!ok || o->copies < 1) {
				err << QCoreApplication::translate("ThmCli", "--copies takes a number of 1 or more.") << "\n";
				return 2;
			}
		} else if (a == QLatin1String("--no-merge")) {
			o->merge = false;
		} else if (a == QLatin1String("--yes")) {
			o->yes = true;
		} else if (a == QLatin1String("--printer")) {
			o->printer_id = value();
		} else if (a == QLatin1String("--material")) {
			o->material = value();
		} else if (a == QLatin1String("--layouts")) {
			o->layouts = value();
		} else if (a == QLatin1String("--family")) {
			o->family = value();
			if (!ThmJob::familyFromKey(o->family, nullptr)) {
				err << QCoreApplication::translate("ThmCli", "Unknown --family; expected mmp or multimark.") << "\n";
				return 2;
			}
		} else if (a == QLatin1String("--usual")) {
			o->usual = true;
		} else if (a == QLatin1String("--roller")) {
			o->roller = value();
		} else if (a == QLatin1String("--rotate")) {
			o->profile.insert(a, QStringLiteral("1"));
		} else if (profile_options.contains(a)) {
			o->profile.insert(a, value());
		} else if (a.startsWith(QLatin1String("--"))) {
			err << QCoreApplication::translate("ThmCli", "Unknown option %1").arg(a) << "\n";
			return 2;
		} else {
			o->positional << a;
		}
	}
	return 0;
}

/// The printer named by --printer, or the one loaded with --material.
/// @return its index, -1 when none (the message written)
int thmPrinterFor(const QList<ThmPrinter> &printers, const ThmCliOptions &o, bool needed)
{
	if (!o.printer_id.isEmpty()) {
		const int i = ThmPrinters::indexOf(printers, o.printer_id);
		if (i < 0) err << QCoreApplication::translate("ThmCli", "No THM printer with the ID \"%1\" is set up (see --thm-printers).").arg(o.printer_id) << "\n";
		return i;
	}
	if (o.material.isEmpty()) {
		if (needed) err << QCoreApplication::translate("ThmCli", "Name the printer with --printer <ID> or the article loaded with --material <number>.") << "\n";
		return -1;
	}
	const ThmPrinters::Choice c = ThmPrinters::choose(printers, o.material);
	if (c.outcome == ThmPrinters::Choice::Unique) return c.matches.first();
	err << c.message << "\n";
	if (c.outcome == ThmPrinters::Choice::Several) {
		for (int i : c.matches)
			err << "  --printer " << printers.at(i).id << "  (" << printers.at(i).name << ")\n";
	}
	return -1;
}

/// The profile options over @p profile. @return false on a bad value
bool applyThmProfile(const QHash<QString, QString> &values, ThmJob::Profile *p)
{
	for (auto it = values.constBegin(); it != values.constEnd(); ++it) {
		const QString &k = it.key();
		bool ok = true;
		if (k == QLatin1String("--sensor")) ok = ThmJob::sensorFromKey(it.value(), &p->sensor);
		else if (k == QLatin1String("--rotate")) p->rotate = true;
		else if (k == QLatin1String("--columns")) p->columns = it.value().toInt(&ok);
		else if (k == QLatin1String("--font")) p->font = it.value().toInt(&ok);
		else if (k == QLatin1String("--speed")) { p->speed = it.value().toInt(&ok); p->set_heat = true; }
		else if (k == QLatin1String("--heat")) { p->heat = it.value().toInt(&ok); p->set_heat = true; }
		else {
			const double v = it.value().toDouble(&ok);
			if (k == QLatin1String("--width")) p->width = v;
			else if (k == QLatin1String("--height")) p->height = v;
			else if (k == QLatin1String("--gap")) p->gap = v;
			else if (k == QLatin1String("--pitch")) p->column_pitch = v;
			else if (k == QLatin1String("--margin")) p->margin_left = p->margin_right = v;
			else if (k == QLatin1String("--text-size")) p->text_size = v;
			else if (k == QLatin1String("--baseline")) p->baseline = v;
		}
		if (!ok) {
			err << QCoreApplication::translate("ThmCli", "Bad value for %1: %2").arg(k, it.value()) << "\n";
			return false;
		}
	}
	return true;
}

/// The layouts of --layouts, else of the settings (the message written
/// when the file cannot be read)
QList<ThmLayouts::Variant> thmLayouts(const ThmCliOptions &o)
{
	QString error;
	if (!o.layouts.isEmpty()) {
		const QList<ThmLayouts::Variant> l = ThmLayouts::read(o.layouts, &error);
		if (!error.isEmpty()) err << error << "\n";
		return l;
	}
	const QList<ThmLayouts::Variant> &l = ThmLayouts::settingsLayouts(nullptr, &error);
	if (!error.isEmpty()) err << error << "\n";
	return l;
}

/// The printer as the job sees it: --family turns its model into one of
/// that family (a printer of nobody's: only the family)
ThmPrinter thmFamilyPrinter(const ThmPrinter *printer, const ThmCliOptions &o)
{
	ThmPrinter p = printer ? *printer : ThmPrinter();
	ThmJob::Family family = p.family();
	if (ThmJob::familyFromKey(o.family, &family))
		p.model = family == ThmJob::Family::MultiMark ? QStringLiteral("THM MultiMark") : QStringLiteral("THM MMP");
	return p;
}

/// The labels of a project as the dialog collects them. @return false
/// when the project cannot be read (the message written)
bool thmLabels(const QString &path, const ThmCliOptions &o, QList<ThmJob::Label> *labels)
{
	if (!QFileInfo::exists(path)) {
		err << QCoreApplication::translate("ThmCli", "Project not found: %1").arg(path) << "\n";
		return false;
	}
	QETProject project(path);
	if (project.state() != QETProject::Ok) {
		err << QCoreApplication::translate("ThmCli", "Failed to open project: %1").arg(path) << "\n";
		return false;
	}
	QString error;
	const QList<MarkingExport::Item> items = MarkingExport::collect(&project, o.marking, &error);
	if (!error.isEmpty()) {
		err << error << "\n";
		return false;
	}
	*labels = ThmJob::toLabels(items, o.copies, o.merge);
	return true;
}

/// --thm-prn: the job for a THM printer as a file, nothing sent
int exportThmJob(const QStringList &rest)
{
	installThmTranslator();
	ThmCliOptions o;
	if (const int code = parseThmOptions(rest, &o)) return code;
	if (o.positional.size() < 2) {
		err << "Usage: qelectrotech --thm-prn <project.qet> <output.prn> "
			   "[--printer ID | --material NO] [--kinds wires,cables,terminals,devices] "
			   "[--folios N[-M]] [--plant P] [--location L] [--full|--short] [--one-end] "
			   "[--copies N] [--no-merge] [--layouts FILE] [--family mmp|multimark] "
			   "[--width MM --height MM --gap MM --sensor l1|e|l0|l2 "
			   "--columns N --pitch MM --margin MM --text-size MM --font N --baseline MM "
			   "--speed MM_S --heat N --rotate]\n";
		return 2;
	}
	const QList<ThmPrinter> printers = ThmPrinters::load();
	const QList<ThmLayouts::Variant> layouts = thmLayouts(o);
	ThmJob::Profile profile;
		//An article the layout file knows: its layout, for the printer
		//named (its family) or for --family; no printer needs to carry it
	const ThmPrinter *named = nullptr;
	if (!o.printer_id.isEmpty()) {
		const int i = thmPrinterFor(printers, o, true);
		if (i < 0) return 2;
		named = &printers.at(i);
	}
	const QString article = !o.material.isEmpty() ? o.material : (named ? named->material_no : QString());
	if (!article.isEmpty() && ThmLayouts::best(layouts, article) >= 0) {
		const ThmPrinter family_printer = thmFamilyPrinter(named, o);
		QString source, error;
		if (!ThmLayouts::jobProfile(layouts, article, &family_printer, &profile, &source, &error)) {
			err << error << "\n";
			return 1;
		}
		out << QCoreApplication::translate("ThmCli", "Profile: %1").arg(source) << "\n";
	} else if (named) {
		profile = named->profile;
	} else if (!o.material.isEmpty()) {
		const int i = thmPrinterFor(printers, o, true);
		if (i < 0) return 2;
		profile = printers.at(i).profile;
	}
	if (!applyThmProfile(o.profile, &profile)) return 2;
	QList<ThmJob::Label> labels;
	if (!thmLabels(o.positional.at(0), o, &labels)) return 1;
	QString error;
	const QByteArray job = ThmJob::build(labels, profile, &error);
	if (job.isEmpty()) {
		err << error << "\n";
		return 1;
	}
	QSaveFile file(o.positional.at(1));
	if (!file.open(QIODevice::WriteOnly) || file.write(job) != job.size() || !file.commit()) {
		err << QCoreApplication::translate("ThmCli", "Cannot write %1: %2").arg(o.positional.at(1), file.errorString()) << "\n";
		return 1;
	}
	out << QCoreApplication::translate("ThmCli", "%1 label(s), %2 text(s) -> %3")
		   .arg(ThmJob::labelCount(labels)).arg(labels.size()).arg(o.positional.at(1)) << "\n";
	return 0;
}

/// --thm-status: firmware, status and material of a printer. Exit code
/// 0 online without error, 3 offline or an error reported, 1 no answer
int thmStatus(const QStringList &rest, bool json)
{
	installThmTranslator();
	if (rest.isEmpty()) {
		err << "Usage: qelectrotech --thm-status <printer ID | host[:port]> [--json]\n";
		return 2;
	}
	const QList<ThmPrinter> printers = ThmPrinters::load();
	const int index = ThmPrinters::indexOf(printers, rest.at(0));
	QString host = rest.at(0);
	quint16 port = 9100;
	if (index >= 0) {
		host = printers.at(index).host;
		port = printers.at(index).port;
	} else {
		const int colon = host.lastIndexOf(QLatin1Char(':'));
		if (colon > 0 && host.count(QLatin1Char(':')) == 1) {
			bool ok = false;
			port = quint16(host.mid(colon + 1).toUInt(&ok));
			if (!ok || port == 0) {
				err << QCoreApplication::translate("ThmCli", "Bad port in %1").arg(rest.at(0)) << "\n";
				return 2;
			}
			host = host.left(colon);
		}
	}
	ThmClient client;
	client.setTarget(host, port);
	const ThmClient::Reply version = client.run(ThmClient::Request::Version);
	if (!version.ok) {
		err << version.error << "\n";
		return 1;
	}
	const ThmClient::Reply status_reply = client.run(ThmClient::Request::Status);
	const ThmClient::Reply material_reply = client.run(ThmClient::Request::Material);
	const ThmJob::Status status = ThmJob::parseStatus(status_reply.data);
	const ThmJob::Material material = ThmJob::parseMaterial(material_reply.data);
	const ThmJob::Fit fit = index >= 0 ? ThmJob::materialFit(material, printers.at(index).profile)
									   : ThmJob::Fit::Unknown;
	if (json) {
		QJsonObject o;
		o.insert("host", host);
		o.insert("port", int(port));
		if (index >= 0) o.insert("printer", printers.at(index).id);
		o.insert("version", ThmJob::parseVersion(version.data));
		o.insert("status_raw", QString::fromLatin1(status.raw));
		o.insert("status_valid", status.valid);
		o.insert("online", status.online);
		o.insert("error", QString(QLatin1Char(status.error)));
		o.insert("error_text", ThmJob::errorText(status.error));
		o.insert("pending", status.pending);
		o.insert("interpreter_active", status.interpreter_active);
		o.insert("material_raw", QString::fromLatin1(material.raw));
		o.insert("synchronized", material.synchronized);
		o.insert("label_distance_mm", material.pitch);
		o.insert("profile_fits", fit == ThmJob::Fit::Unknown ? QJsonValue()
								 : QJsonValue(fit == ThmJob::Fit::Fits));
		if (!status_reply.ok) o.insert("status_failure", status_reply.error);
		if (!material_reply.ok) o.insert("material_failure", material_reply.error);
		out << QJsonDocument(o).toJson(QJsonDocument::Indented);
	} else {
		out << QCoreApplication::translate("ThmCli", "Printer: %1").arg(ThmJob::parseVersion(version.data)) << "\n";
		out << QCoreApplication::translate("ThmCli", "Status: %1").arg(status_reply.ok ? ThmJob::statusText(status) : status_reply.error) << "\n";
		out << QCoreApplication::translate("ThmCli", "Material: %1").arg(material_reply.ok ? ThmJob::materialText(material) : material_reply.error) << "\n";
		if (fit == ThmJob::Fit::Differs)
			out << QCoreApplication::translate("ThmCli", "The label distance does not fit the profile of %1.").arg(printers.at(index).name) << "\n";
	}
	out.flush();
	if (!status_reply.ok) return 1;
	return status.valid && status.online && !status.hasError() ? 0 : 3;
}

/// --thm-send: a project's labels, or a .prn file, to a printer; only
/// with --yes, after the printer has reported itself online without
/// error. Exit code 4: not sent because --yes was not given
int thmSend(const QStringList &rest)
{
	installThmTranslator();
	ThmCliOptions o;
	if (const int code = parseThmOptions(rest, &o)) return code;
	if (o.positional.isEmpty()) {
		err << "Usage: qelectrotech --thm-send <project.qet | job.prn> "
			   "(--printer ID | --material NO) [the options of --thm-prn] [--yes]\n";
		return 2;
	}
	const QList<ThmPrinter> printers = ThmPrinters::load();
	const int index = thmPrinterFor(printers, o, true);
	if (index < 0) return 2;
	ThmPrinter printer = printers.at(index);
	QString error;
		//Only what the printer is loaded with goes to it
	if (!o.material.isEmpty() && ThmPrinters::normalizedMaterial(o.material)
			!= ThmPrinters::normalizedMaterial(printer.material_no)) {
		err << QCoreApplication::translate("ThmCli", "\"%1\" is loaded with %2, not %3. Confirm the change first: "
					   "--thm-load %4 %3").arg(printer.name, printer.material_no, o.material, printer.id) << "\n";
		return 2;
	}
	if (!ThmPrinters::validate(printer, &error, false)) {
		err << error << "\n";
		return 2;
	}
	if (!o.positional.at(0).endsWith(QLatin1String(".prn"), Qt::CaseInsensitive)) {
		const ThmPrinter family_printer = thmFamilyPrinter(&printer, o);
		QString source;
		if (!ThmLayouts::jobProfile(thmLayouts(o), printer.material_no, &family_printer,
									&printer.profile, &source, &error)) {
			err << error << "\n";
			return 2;
		}
		out << QCoreApplication::translate("ThmCli", "Profile: %1").arg(source) << "\n";
	}
	if (!applyThmProfile(o.profile, &printer.profile)) return 2;
	const QString input = o.positional.at(0);
	QByteArray job;
	QString quantity;
	if (input.endsWith(QLatin1String(".prn"), Qt::CaseInsensitive)) {
		QFile file(input);
		if (!file.open(QIODevice::ReadOnly)) {
			err << QCoreApplication::translate("ThmCli", "Cannot open %1: %2").arg(input, file.errorString()) << "\n";
			return 1;
		}
		job = file.readAll();
		quantity = QCoreApplication::translate("ThmCli", "file %1, %2 bytes").arg(QFileInfo(input).fileName()).arg(job.size());
	} else {
		QList<ThmJob::Label> labels;
		if (!thmLabels(input, o, &labels)) return 1;
		job = ThmJob::build(labels, printer.profile, &error);
		if (job.isEmpty()) {
			err << error << "\n";
			return 1;
		}
		quantity = QCoreApplication::translate("ThmCli", "%1 label(s), %2 text(s)").arg(ThmJob::labelCount(labels)).arg(labels.size());
	}
	out << QCoreApplication::translate("ThmCli", "Target: %1 (%2:%3)").arg(printer.name, printer.host).arg(printer.port) << "\n";
	out << QCoreApplication::translate("ThmCli", "Article: %1").arg(printer.material_name.isEmpty()
									 ? printer.material_no
									 : printer.material_no + QStringLiteral(" – ") + printer.material_name) << "\n";
	out << QCoreApplication::translate("ThmCli", "Quantity: %1").arg(quantity) << "\n";
	if (!o.yes) {
		out << QCoreApplication::translate("ThmCli", "Nothing sent. Add --yes to print.") << "\n";
		out.flush();
		return 4;
	}
	ThmClient client;
	client.setTarget(printer.host, printer.port);
	client.setJobTransport(printer.transport, printer.lpd_queue);
	const bool lpd = printer.transport == ThmClient::Transport::Lpd;
	const ThmClient::Reply status_reply = client.run(ThmClient::Request::Status);
	if (!status_reply.ok) {
			//A printer sent to by LPD often keeps RawIP silent: then
			//LPD's own confirmation is all there is
		if (!lpd) {
			err << status_reply.error << "\n";
			return 1;
		}
		out << QCoreApplication::translate("ThmCli", "No status by RawIP (%1); sending by LPD all the same.").arg(status_reply.error) << "\n";
	} else {
		const ThmJob::Status status = ThmJob::parseStatus(status_reply.data);
		if (!status.valid || !status.online || status.hasError()) {
			err << QCoreApplication::translate("ThmCli", "Not sent: %1").arg(ThmJob::statusText(status)) << "\n";
			return 3;
		}
	}
	const ThmClient::Reply sent = client.run(ThmClient::Request::Job, job);
	if (!sent.ok) {
		err << sent.error << "\n";
		return 1;
	}
	out << (lpd ? QCoreApplication::translate("ThmCli", "Sent to %1 by LPD; the printer confirmed the job.").arg(printer.name)
				: QCoreApplication::translate("ThmCli", "Sent to %1.").arg(printer.name)) << "\n";
	return 0;
}

/// --thm-printers: the printers set up, one per line
int thmPrinters(bool json)
{
	installThmTranslator();
	const QList<ThmPrinter> printers = ThmPrinters::load();
	if (json) {
		QJsonArray rows;
		for (const ThmPrinter &p : printers) {
			QJsonObject o;
			o.insert("id", p.id);
			o.insert("name", p.name);
			o.insert("model", p.model);
			o.insert("host", p.host);
			o.insert("port", int(p.port));
			o.insert("transport", ThmClient::transportKey(p.transport));
			o.insert("lpd_queue", p.lpd_queue);
			o.insert("material_no", p.material_no);
			o.insert("material_name", p.material_name);
			o.insert("loaded_at", p.loaded_at.isValid() ? p.loaded_at.toString(Qt::ISODate) : QString());
			o.insert("usual_material_no", p.usual_material_no);
			o.insert("usual_material_name", p.usual_material_name);
			o.insert("roller", p.roller);
			o.insert("family", ThmJob::familyKey(p.family()));
			o.insert("width", p.profile.width);
			o.insert("height", p.profile.height);
			o.insert("gap", p.profile.gap);
			o.insert("sensor", ThmJob::sensorKey(p.profile.sensor));
			o.insert("columns", p.profile.columns);
			rows.append(o);
		}
		out << QJsonDocument(rows).toJson(QJsonDocument::Indented);
		return 0;
	}
	if (printers.isEmpty()) out << QCoreApplication::translate("ThmCli", "No THM printer is set up.") << "\n";
	for (const ThmPrinter &p : printers) {
		out << p.id << "\t" << p.name << "\t" << p.host << ":"
			<< (p.transport == ThmClient::Transport::Lpd ? QStringLiteral("515 (LPD)") : QString::number(p.port)) << "\t"
			<< p.material_no << "\t" << ThmJob::number(p.profile.width) << " x "
			<< ThmJob::number(p.profile.height) << " mm";
		if (ThmPrinters::changedFromUsual(p))
			out << "\t" << QCoreApplication::translate("ThmCli", "usually %1").arg(p.usual_material_no);
		out << "\n";
	}
	return 0;
}

/// --thm-load: the user has loaded another article (and roller) on a
/// printer and confirms it; with --usual it becomes the usual one too
int thmLoad(const QStringList &rest)
{
	installThmTranslator();
	ThmCliOptions o;
	if (const int code = parseThmOptions(rest, &o)) return code;
	if (o.positional.size() < 2) {
		err << "Usage: qelectrotech --thm-load <printer ID> <article> [--usual] [--roller TEXT] [--layouts FILE]\n";
		return 2;
	}
	QList<ThmPrinter> printers = ThmPrinters::load();
	const int index = ThmPrinters::indexOf(printers, o.positional.at(0));
	if (index < 0) {
		err << QCoreApplication::translate("ThmCli", "No THM printer with the ID \"%1\" is set up (see --thm-printers).").arg(o.positional.at(0)) << "\n";
		return 2;
	}
	const QString article = o.positional.at(1).trimmed();
	const QList<ThmLayouts::Variant> layouts = thmLayouts(o);
	const int v = ThmLayouts::best(layouts, article);
	const QString name = v >= 0 ? layouts.at(v).name : QString();
	ThmPrinter &p = printers[index];
	ThmPrinters::confirmLoading(&p, article, name, QDateTime::currentDateTime(), o.usual);
	if (!o.roller.isEmpty()) p.roller = o.roller;
	ThmPrinters::save(printers);
	out << QCoreApplication::translate("ThmCli", "%1 is now loaded with %2.").arg(p.name, name.isEmpty() ? article : article + QStringLiteral(" – ") + name) << "\n";
	if (v >= 0) {
		const ThmLayouts::Variant &var = layouts.at(v);
		if (!var.ribbon.isEmpty()) out << QCoreApplication::translate("ThmCli", "Ribbon: %1").arg(var.ribbon) << "\n";
		if (!var.refusal.isEmpty()) out << QCoreApplication::translate("ThmCli", "Cannot be printed yet: %1").arg(var.refusal) << "\n";
	}
	if (!p.roller.isEmpty()) out << QCoreApplication::translate("ThmCli", "Roller: %1").arg(p.roller) << "\n";
	if (ThmPrinters::changedFromUsual(p))
		out << QCoreApplication::translate("ThmCli", "Usual article: %1").arg(p.usual_material_no) << "\n";
	return 0;
}

/// --thm-transport: how jobs go to a printer, RawIP or LPD
int thmTransport(const QStringList &rest)
{
	installThmTranslator();
	QStringList positional;
	QString queue;
	for (int i = 0; i < rest.size(); ++i) {
		if (rest.at(i) == QLatin1String("--queue") && i + 1 < rest.size()) queue = rest.at(++i);
		else positional << rest.at(i);
	}
	const QString key = positional.value(1).trimmed().toLower();
	if (positional.size() < 2 || (key != QLatin1String("rawip") && key != QLatin1String("lpd"))) {
		err << "Usage: qelectrotech --thm-transport <printer ID> rawip|lpd [--queue NAME]\n";
		return 2;
	}
	QList<ThmPrinter> printers = ThmPrinters::load();
	const int index = ThmPrinters::indexOf(printers, positional.at(0));
	if (index < 0) {
		err << QCoreApplication::translate("ThmCli", "No THM printer with the ID \"%1\" is set up (see --thm-printers).").arg(positional.at(0)) << "\n";
		return 2;
	}
	ThmPrinter &p = printers[index];
	p.transport = ThmClient::transportFromKey(key);
	if (!queue.trimmed().isEmpty()) p.lpd_queue = queue.trimmed();
	ThmPrinters::save(printers);
	out << (p.transport == ThmClient::Transport::Lpd
			? QCoreApplication::translate("ThmCli", "%1 now gets its jobs by LPD (port 515, queue %2).").arg(p.name, p.lpd_queue)
			: QCoreApplication::translate("ThmCli", "%1 now gets its jobs by RawIP on port %2.").arg(p.name).arg(p.port)) << "\n";
	return 0;
}

/// The purchase list (PurchaseList): articles grouped by supplier, sums,
/// the devices without an article; .xlsx or CSV, with --erp also the
/// neutral ERP CSV beside it. --online first asks the Data Portal for
/// the articles the article data cache does not know.
int exportPurchase(QETProject &project, const QString &output, bool online, bool erp)
{
		//The list is read by people in purchasing: its headings in the
		//language of the settings, as the menu writes it (the other CLI
		//outputs stay as they are)
	static QTranslator translator;
	const QString language = QETApp::langFromSetting();
	bool loaded = false;
	for (const QString &dir : {QETApp::languagesPath(),
							   QCoreApplication::applicationDirPath() + QStringLiteral("/lang")}) {
		for (const QString &name : {language, language.section(QLatin1Char('_'), 0, 0)}) {
			if (!loaded && translator.load(QStringLiteral("qet_") + name, dir) && !translator.isEmpty())
				loaded = true;
		}
	}
	if (loaded) QCoreApplication::installTranslator(&translator);
	else err << "No translation qet_" << language << " in " << QETApp::languagesPath() << "\n";
	QString error;
	QList<PurchaseList::Item> items = PurchaseList::collect(&project, &error);
	if (!error.isEmpty()) {
		err << "Purchase list failed: " << error << "\n";
		return 1;
	}
	const PurchaseFetch::Result fetched = PurchaseFetch::fetch(items, online);
	if (!fetched.error.isEmpty())
		err << "Data Portal: " << fetched.error << "\n";
	if (fetched.from_portal + fetched.from_edz + fetched.not_found > 0)
		items = PurchaseList::collect(&project, &error);
	QStringList written;
	if (!PurchaseList::write(items, output, erp, &written, &error)) {
		err << "Cannot write '" << output << "': " << error << "\n";
		return 1;
	}
	QJsonObject summary = PurchaseList::summarize(items).toJson();
	summary.insert(QStringLiteral("lookup"), fetched.toJson());
	summary.insert(QStringLiteral("files"), QJsonArray::fromStringList(written));
	summary.insert(QStringLiteral("price_list"), QDir::toNativeSeparators(PurchaseList::priceListPath()));
	out << QJsonDocument(summary).toJson(QJsonDocument::Compact) << "\n";
	return 0;
}

/// Fills the article data cache for the articles of a project (EDZ
/// packages, and the Data Portal unless --offline) and prints what came
/// of it as JSON.
int fetchCommercial(QETProject &project, bool online)
{
	QString error;
	const QList<PurchaseList::Item> items = PurchaseList::collect(&project, &error);
	if (!error.isEmpty()) {
		err << "Purchase list failed: " << error << "\n";
		return 1;
	}
	const PurchaseFetch::Result result = PurchaseFetch::fetch(items, online,
		[](int done, int total, const QString &part) {
			if (!part.isEmpty()) err << "[" << (done + 1) << "/" << total << "] " << part << "\n";
			err.flush();
			return true;
		});
	QJsonObject o = result.toJson();
	o.insert(QStringLiteral("cache"), QDir::toNativeSeparators(PurchaseList::cachePath()));
	out << QJsonDocument(o).toJson(QJsonDocument::Indented);
	return result.error.isEmpty() ? 0 : 1;
}

QString csvField(const QString &value)
{
	if (value.contains(QLatin1Char(';')) || value.contains(QLatin1Char('"'))
			|| value.contains(QLatin1Char('\n')) || value.contains(QLatin1Char('\r')))
	{
		QString escaped = value;
		escaped.replace(QLatin1Char('"'), QStringLiteral("\"\""));
		return QLatin1Char('"') % escaped % QLatin1Char('"');
	}
	return value;
}

/// Count terminals on @p element that no conductor connects to.
int freeTerminals(Element *element)
{
	int free = 0;
	const QList<Terminal *> terminals = element->terminals();
	for (Terminal *t : terminals)
		if (t->conductorsCount() == 0)
			++free;
	return free;
}

/// Structural ground-truth dump of a project, as JSON, to stdout (or a file).
/// Uses QET's own loaded model, so it reports what the editor actually sees:
/// per-page element / conductor counts and unconnected terminals.
int exportInfo(QETProject &project, const QString &output)
{
	const QList<Diagram *> diagrams = project.diagrams();

	int total_elements = 0, total_conductors = 0, total_free = 0;
	QJsonArray pages;
	int index = 0;
	for (Diagram *diagram : diagrams) {
		++index;
		const QList<Element *> elements = diagram->elements();
		const int conductors = diagram->conductors().size();
		int page_free = 0;
		for (Element *e : elements)
			page_free += freeTerminals(e);

		const QRect r = diagramRect(diagram);
		QJsonObject page;
		page["index"]             = index;
		page["title"]             = diagram->title();
		page["folio"]             = QStringLiteral("%1 of %2")
									 .arg(index).arg(diagrams.size());
		page["width_px"]          = r.width();
		page["height_px"]         = r.height();
		page["elements"]          = elements.size();
		page["conductors"]        = conductors;
		page["free_terminals"]    = page_free;
		pages.append(page);

		total_elements   += elements.size();
		total_conductors += conductors;
		total_free       += page_free;
	}

	QJsonObject root;
	root["project"]    = project.title();
	root["diagrams"]   = diagrams.size();
	root["elements"]   = total_elements;
	root["conductors"] = total_conductors;
	root["free_terminals"] = total_free;
	root["pages"]      = pages;

	const QByteArray json =
		QJsonDocument(root).toJson(QJsonDocument::Indented);

	if (output.isEmpty()) {
		out << QString::fromUtf8(json);
	} else {
		QFile file(output);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
			err << "Cannot open '" << output << "' for writing.\n";
			return 1;
		}
		file.write(json);
		file.close();
		out << "Wrote project info -> " << output << "\n";
	}
	return 0;
}

/// Validate one .elmt file against QET's element schema.
/// @return 0 = OK, 1 = warning (loads but suspicious), 2 = failure.
int checkOneElement(const QString &path, bool grid)
{
	QFile file(path);
	if (!file.open(QIODevice::ReadOnly)) {
		out << "FAIL  " << path << "  (cannot open)\n";
		return 2;
	}
	QDomDocument doc;
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
	if (const auto result = doc.setContent(&file); !result) {
		file.close();
		out << "FAIL  " << path << "  (XML error line "
			<< result.errorLine << ": " << result.errorMessage << ")\n";
		return 2;
	}
#else
	QString error;
	int line = 0;
	if (!doc.setContent(&file, &error, &line)) {
		file.close();
		out << "FAIL  " << path << "  (XML error line "
			<< line << ": " << error << ")\n";
		return 2;
	}
#endif
	file.close();

	const QDomElement root = doc.documentElement();
	if (root.tagName() != "definition" || root.attribute("type") != "element") {
		out << "FAIL  " << path << "  (root is not <definition type=\"element\">)\n";
		return 2;
	}

	bool w_ok = false, h_ok = false;
	const double w = root.attribute("width").toDouble(&w_ok);
	const double h = root.attribute("height").toDouble(&h_ok);
	if (!w_ok || !h_ok || w == 0 || h == 0) {
		out << "FAIL  " << path << "  (missing/zero bounding box "
			<< root.attribute("width") << "x"
			<< root.attribute("height") << ")\n";
		return 2;
	}

	const QDomNodeList terminal_nodes = root.elementsByTagName("terminal");
	const int terminals = terminal_nodes.count();
	QStringList terminal_names;
	for (int i = 0; i < terminals; ++i)
		terminal_names << terminal_nodes.at(i).toElement().attribute("name");

	// Two terminals with one name cannot be told apart in a wiring list
	// (IEC 61666), the same rule the element editor applies on save.
	const auto repeated = TerminalNameCheck::repeatedNames(terminal_names);
	if (!repeated.isEmpty()) {
		out << "FAIL  " << path << "  (repeated terminal names: "
			<< TerminalNameCheck::describe(repeated) << ")\n";
		return 2;
	}

	// Fluid data (simulation): must load in the model, and every path of a
	// valve position must name terminals the element has.
	const QDomElement fluid = root.firstChildElement(FluidData::xmlTagName());
	if (!fluid.isNull()) {
		FluidData data;
		if (!data.fromXml(fluid)) {
			out << "FAIL  " << path << "  (<fluid> does not load: kind \""
				<< fluid.attribute("kind") << "\")\n";
			return 2;
		}
		QStringList unknown;
		for (const FluidData::Position &position : data.m_positions)
			for (const FluidData::Path &p : position.paths)
				for (const QString &port : {p.from, p.to})
					if (!port.isEmpty() && !terminal_names.contains(port)
						&& !unknown.contains(port))
						unknown << port;
		if (!unknown.isEmpty()) {
			out << "FAIL  " << path << "  (<fluid> path names unknown terminals: "
				<< unknown.join(", ") << ")\n";
			return 2;
		}
	}

	// QET loads the element but leaves out a shape with a "nan" or "inf"
	// coordinate, on the folio and in the element editor.
	const QDomNodeList description = root.elementsByTagName("description");
	for (QDomNode n = description.isEmpty() ? QDomNode()
					  : description.at(0).firstChild() ;
		 !n.isNull() ; n = n.nextSibling()) {
		const QDomElement shape = n.toElement();
		if (!shape.isNull() && QET::hasNonFiniteGeometry(shape)) {
			out << "WARN  " << path << "  (<" << shape.tagName()
				<< "> with a non-finite coordinate is not drawn)\n";
			return 1;
		}
	}
	// Negative dimensions are malformed but QET still loads them; surface as a
	// warning rather than a failure so this agrees with QET's own loader.
	if (w < 0 || h < 0) {
		out << "WARN  " << path << "  (negative bounding box "
			<< w << "x" << h << ", " << terminals << " terminals)\n";
		return 1;
	}

	if (terminals == 0) {
		out << "WARN  " << path << "  (loads, but 0 terminals)\n";
		return 1;
	}

	const QString type = root.attribute("link_type");
	const int unnamed = TerminalNameCheck::unnamedCount(terminal_names);
	if (unnamed && !type.endsWith("_report")
		&& type != "conductor_definition" && type != "thumbnail") {
		out << "WARN  " << path << "  (" << unnamed << " of " << terminals
			<< " terminals have no name)\n";
		return 1;
	}

	// --grid: the insertion point on a connection point, the frame and the
	// connection points on the grid (SymbolGridCheck)
	if (grid && type != "thumbnail") {
		QList<QPointF> points;
		for (int i = 0; i < terminals; ++i) {
			const QDomElement t = terminal_nodes.at(i).toElement();
			points << QPointF(t.attribute("x").toDouble(), t.attribute("y").toDouble());
		}
		const QStringList problems = SymbolGridCheck::problems(
				root.attribute("hotspot_x").toInt(), root.attribute("hotspot_y").toInt(),
				int(w), int(h), points);
		if (!problems.isEmpty()) {
			out << "WARN  " << path << "  (grid: " << problems.join("; ") << ")\n";
			return 1;
		}
	}

	out << "OK    " << path << "  (" << terminals << " terminals)\n";
	return 0;
}

/// --simulate <project> [--event "0.2 -SF1 an"]... [--duration s] [--pressure bar] [--csv file]
///            [--measure all|id,id…] [--raster s] [--meta file.json] [--signals]
/// --signals lists the measurable quantities (id, unit, reference) as JSON and ends; --measure adds them
/// to the CSV (header "id [unit]"); --raster is the output step (the integration step stays 10 µs);
/// --meta writes the run's identity and status (program, catalog, parameters, end time, error).
/// Events press/release a push button ("0.2 -SF1 an") or actuate a valve by hand ("0.2 -QM1 links an").
int simulateProject(QETProject &project, const QStringList &args)
{
	double duration = 2.0, pressure = 6.0, png_at = -1.0, raster = 0.005;
	QString csv_path, png_dir, measure, meta_path;
	bool list_signals = false;
	bool until_change = false;   // R16
	QString folios;               // R16: 1,3 or 2-4
	struct Event { double t; QString label; int side; bool on; double value = 0.0; };   // side 2: setpoint (R07)
	QList<Event> events;
	static const QRegularExpression ev_re(QStringLiteral(
		"^\\s*([\\d.]+)\\s+(.+?)\\s+(?:(links|rechts|left|right)\\s+)?(an|aus|on|off)\\s*$"));
	for (int i = 0; i < args.size(); ++i) {
		const QString a = args.at(i);
		const QString v = args.value(i + 1);
		if (a == QLatin1String("--duration")) { duration = v.toDouble(); ++i; }
		else if (a == QLatin1String("--pressure")) { pressure = v.toDouble(); ++i; }
		else if (a == QLatin1String("--csv")) { csv_path = v; ++i; }
		else if (a == QLatin1String("--png")) { png_dir = v; ++i; }
		else if (a == QLatin1String("--at")) { png_at = v.toDouble(); ++i; }
		else if (a == QLatin1String("--measure")) { measure = v; ++i; }
		else if (a == QLatin1String("--raster")) { raster = v.toDouble(); ++i; }
		else if (a == QLatin1String("--meta")) { meta_path = v; ++i; }
		else if (a == QLatin1String("--signals")) list_signals = true;
		else if (a == QLatin1String("--until-change")) until_change = true;
		else if (a == QLatin1String("--folios")) { folios = v; ++i; }
		else if (a == QLatin1String("--event")) {
			// R07: "0.5 -QM1 sollwert 5" sets the setpoint of a proportional valve (volts)
			static const QRegularExpression sp_re(QStringLiteral("^\\s*([\\d.]+)\\s+(.+?)\\s+(?:sollwert|setpoint)\\s+(-?[\\d.]+)\\s*$"));
			// R15: "1.2 -QM1 reparieren" repairs the faults of a part
			static const QRegularExpression rep_re(QStringLiteral("^\\s*([\\d.]+)\\s+(.+?)\\s+(?:reparieren|repair)\\s*$"));
			const auto rep = rep_re.match(v);
			if (rep.hasMatch()) {
				events << Event{rep.captured(1).toDouble(), rep.captured(2), 3, true};
				++i;
				continue;
			}
			const auto sp = sp_re.match(v);
			if (sp.hasMatch()) {
				events << Event{sp.captured(1).toDouble(), sp.captured(2), 2, true, sp.captured(3).toDouble()};
				++i;
				continue;
			}
			const auto m = ev_re.match(v);
			if (!m.hasMatch()) { err << "event not understood: " << v << "\n"; return 2; }
			const QString s = m.captured(3);
			events << Event{m.captured(1).toDouble(), m.captured(2),
							s.isEmpty() ? -1 : (s == QLatin1String("links") || s == QLatin1String("left") ? 0 : 1),
							m.captured(4) == QLatin1String("an") || m.captured(4) == QLatin1String("on")};
			++i;
		}
	}
	// R03: events at the same time keep the order of the command line (stable), so a run is reproducible
	std::stable_sort(events.begin(), events.end(), [](const Event &x, const Event &y) { return x.t < y.t; });
	// R16: only some folios of the project (1-based, "1,3" or "2-4")
	QList<Diagram *> chosen;
	if (folios.isEmpty()) chosen = project.diagrams();
	else {
		const QList<Diagram *> all = project.diagrams();
		for (const QString &part : folios.split(QLatin1Char(','), Qt::SkipEmptyParts)) {
			const QStringList r = part.trimmed().split(QLatin1Char('-'));
			bool ok1 = false, ok2 = true;
			const int lo = r.value(0).toInt(&ok1), hi = r.size() > 1 ? r.value(1).toInt(&ok2) : lo;
			if (!ok1 || !ok2 || lo < 1 || hi > all.size() || hi < lo) { err << "folios not understood: " << folios << "\n"; return 2; }
			for (int k = lo; k <= hi; ++k)
				if (!chosen.contains(all.at(k - 1))) chosen << all.at(k - 1);
		}
	}
	QElapsedTimer wall;
	wall.start();
	SimulationModel sim(chosen, pressure);
	if (!list_signals) out << sim.summary() << "\n";
	for (const QString &e : sim.modelErrors()) err << "Model error: " << e << "\n";
	if (sim.failed()) {
		err << "Model error: the simulation does not run: " << sim.failure() << "\n";
		err.flush();
		return 3;
	}
	for (const QString &e : sim.programErrors()) err << "PLC program: " << e << "\n";
	if (list_signals) {
		QJsonArray list;
		for (const auto &s : sim.signalList()) {
			QJsonObject o;
			o.insert(QStringLiteral("id"), s.id);
			o.insert(QStringLiteral("quantity"), s.quantity);
			o.insert(QStringLiteral("unit"), s.unit);
			o.insert(QStringLiteral("reference"), s.reference);
			o.insert(QStringLiteral("part"), s.part);
			o.insert(QStringLiteral("port"), s.port);
			list.append(o);
		}
		out << QString::fromUtf8(QJsonDocument(list).toJson(QJsonDocument::Indented));
		out.flush();
		return 0;
	}
	if (!(raster > 0) || !std::isfinite(raster)) { err << "raster must be a positive number of seconds\n"; return 2; }
	// the measured quantities: all, or the ids asked for; an unknown id is an error, not a column of zeros
	QList<SimulationModel::Signal> measured;
	if (!measure.isEmpty()) {
		const QList<SimulationModel::Signal> all = sim.signalList();
		if (measure == QLatin1String("all")) measured = all;
		else
			for (const QString &id : measure.split(QLatin1Char(','), Qt::SkipEmptyParts)) {
				auto it = std::find_if(all.begin(), all.end(), [&](const SimulationModel::Signal &s) { return s.id == id.trimmed(); });
				if (it == all.end()) { err << "unknown or unsupported signal: " << id.trimmed() << " (see --signals)\n"; return 2; }
				measured << *it;
			}
		bool flows = false;
		for (const auto &s : measured) flows = flows || s.id.startsWith(QLatin1String("q:")) || s.id.startsWith(QLatin1String("F:"));
		sim.setRecordFlows(flows);
	}
	bool png_done = png_dir.isEmpty();
	auto write_png = [&]() {
		// the folios with the simulation display (pressure, live wires, states) at this moment
		SimulationOverlay overlay;
		overlay.update(sim, project.diagrams());
		QDir().mkpath(png_dir);
		int index = 0;
		for (Diagram *diagram : project.diagrams()) {
			++index;
			const QRect r = diagramRect(diagram);
			QImage image(r.size(), QImage::Format_ARGB32);
			image.fill(Qt::white);
			QPainter painter(&image);
			painter.setRenderHint(QPainter::Antialiasing, true);
			renderDiagram(diagram, painter, QRectF(QPointF(0, 0), r.size()), false);
			painter.end();
			const QString path = QDir(png_dir).filePath(diagramStem(diagram, index) + ".png");
			image.save(path);
			out << "  " << path << "\n";
		}
		overlay.clear(true);
	};
	QFile csv(csv_path);
	QTextStream csv_out(&csv);
	if (!csv_path.isEmpty() && csv.open(QIODevice::WriteOnly | QIODevice::Text)) {
		csv_out << "t";
		for (const auto &c : sim.cylinders()) csv_out << ";" << c.label << " x [mm]";
		for (const auto &v : sim.valveStates()) csv_out << ";" << v.label << " Stellung";
		for (const auto &s : measured) csv_out << ";" << s.id << " [" << s.unit << "]";
		csv_out << "\n";
	}
	const double dt = 1e-5;
	double next_row = 0.0;
	int ei = 0;
	while (sim.time() <= duration + 1e-12) {
		while (ei < events.size() && events.at(ei).t <= sim.time()) {
			const Event &e = events.at(ei++);
			if (e.side == 2) {
				if (!sim.setSetpoint(e.label, e.value)) { err << "no proportional valve labelled " << e.label << "\n"; return 2; }
			} else if (e.side == 3) {
				if (!sim.repair(e.label)) { err << "no fault at " << e.label << "\n"; return 2; }
			} else if (e.side < 0) sim.pressButton(e.label, e.on);
			else if (!sim.actuateValve(e.label, e.side == 0, e.on)) {
				err << "no valve labelled " << e.label << "\n";
				return 2;
			}
		}
		if (csv.isOpen() && sim.time() >= next_row - 1e-12) {
			csv_out << QString::number(sim.time(), 'f', 4);
			for (const auto &c : sim.cylinders()) csv_out << ";" << QString::number(c.x_mm, 'f', 2);
			for (const auto &v : sim.valveStates()) csv_out << ";" << v.position;
			for (const auto &s : measured) {
				bool ok = false;
				const double val = sim.signalValue(s.id, &ok);
				csv_out << ";" << (ok && std::isfinite(val) ? QString::number(val, 'g', 9) : QStringLiteral("NaN"));
			}
			csv_out << "\n";
			next_row += raster;
		}
		if (!png_done && sim.time() >= (png_at < 0 ? duration : png_at) - 1e-9) {
			write_png();
			png_done = true;
		}
		sim.step(dt);
		bool changed = false;
		for (const auto &m : sim.takeMessages()) {
			out << QString::number(m.time, 'f', 3).rightJustified(8) << " s  " << m.text << "\n";
			changed = changed || m.time > 1e-9;
		}
		if (sim.failed()) break;
		// R16: --until-change ends the run at the first state change after the start
		if (until_change && changed) {
			out << "Halt bei der ersten Zustandsänderung\n";
			break;
		}
	}
	if (!png_done) write_png();
	out << "Status: " << (sim.failed() ? "abgebrochen" : "ok") << ", Endzeit " << QString::number(sim.time(), 'f', 4) << " s\n";
	if (!meta_path.isEmpty()) {
		// identity of the run (K15): program, catalog, project, inputs, solver settings, status
		QJsonObject meta;
		meta.insert(QStringLiteral("program"), QStringLiteral("QElectroTech ") + QetVersion::displayedVersionWithFork());
		meta.insert(QStringLiteral("catalogVersion"), SimulationCatalog::version());
		meta.insert(QStringLiteral("project"), QFileInfo(project.filePath()).absoluteFilePath());
		QFile pf(project.filePath());
		if (pf.open(QIODevice::ReadOnly))
			meta.insert(QStringLiteral("projectSha256"), QString::fromLatin1(QCryptographicHash::hash(pf.readAll(), QCryptographicHash::Sha256).toHex()));
		meta.insert(QStringLiteral("supplyPressureBar"), pressure);
		meta.insert(QStringLiteral("durationS"), duration);
		meta.insert(QStringLiteral("integrationStepS"), dt);
		// R16 performance figures: wall time of build and run, steps, problem size
		const double seconds = wall.nsecsElapsed() / 1e9;
		meta.insert(QStringLiteral("wallTimeS"), seconds);
		meta.insert(QStringLiteral("steps"), static_cast<double>(sim.stepCount()));
		meta.insert(QStringLiteral("stepsPerSecond"), seconds > 0 ? sim.stepCount() / seconds : 0.0);
		meta.insert(QStringLiteral("fluidNets"), sim.netCount());
		meta.insert(QStringLiteral("folios"), folios.isEmpty() ? QStringLiteral("alle") : folios);
		meta.insert(QStringLiteral("outputRasterS"), raster);
		meta.insert(QStringLiteral("integrator"), QStringLiteral("explicit Euler; oil nets with accumulator implicit (bisection)"));
		QJsonArray evs;
		for (const auto &e : events)
			evs.append(e.side == 3 ? QStringLiteral("%1 %2 reparieren").arg(e.t).arg(e.label) : QStringLiteral("%1 %2 %3%4").arg(e.t).arg(e.label)
					   .arg(e.side < 0 ? QString() : (e.side == 0 ? QStringLiteral("links ") : QStringLiteral("rechts ")))
					   .arg(e.on ? QStringLiteral("an") : QStringLiteral("aus")));
		meta.insert(QStringLiteral("events"), evs);
		QJsonArray sig;
		for (const auto &s : measured) {
			QJsonObject o;
			o.insert(QStringLiteral("id"), s.id);
			o.insert(QStringLiteral("unit"), s.unit);
			o.insert(QStringLiteral("reference"), s.reference);
			sig.append(o);
		}
		meta.insert(QStringLiteral("signals"), sig);
		meta.insert(QStringLiteral("status"), sim.failed() ? QStringLiteral("failed") : QStringLiteral("ok"));
		meta.insert(QStringLiteral("endTimeS"), sim.time());
		if (sim.failed()) meta.insert(QStringLiteral("error"), sim.failure());
		QFile mf(meta_path);
		if (mf.open(QIODevice::WriteOnly)) mf.write(QJsonDocument(meta).toJson(QJsonDocument::Indented));
	}
	const QHash<QString, double> vars = sim.plcVariables();
	if (!vars.isEmpty()) {
		QStringList names = vars.keys();
		names.sort();
		QStringList parts;
		for (const QString &n : names) parts << n + QLatin1Char('=') + QString::number(vars.value(n), 'g', 6);
		out << "PLC variables at the end: " << parts.join(QStringLiteral(", ")) << "\n";
	}
	out.flush();
	if (sim.failed()) {
		err << "Model error: " << sim.failure() << "\n";
		err.flush();
		return 3;
	}
	return 0;
}

/// Validate a single .elmt file or every .elmt under a directory; with
/// @p grid, also check that each symbol lines up with the folio grid.
int checkElements(const QString &path, bool grid)
{
	QStringList files;
	const QFileInfo info(path);
	if (info.isDir()) {
		QDirIterator it(path, {"*.elmt"}, QDir::Files,
						QDirIterator::Subdirectories);
		while (it.hasNext())
			files << it.next();
		files.sort();
	} else if (info.isFile()) {
		files << path;
	} else {
		err << "Not found: " << path << "\n";
		return 2;
	}

	if (files.isEmpty()) {
		err << "No .elmt files found under: " << path << "\n";
		return 2;
	}

	int warnings = 0, failures = 0;
	for (const QString &f : files) {
		const int r = checkOneElement(f, grid);
		if (r == 1) ++warnings;
		else if (r == 2) ++failures;
	}
	out << files.size() << " file(s), " << warnings
		<< " warning(s), " << failures << " failure(s)\n";
	return failures > 0 ? 1 : 0;
}

/// Map every element in the project to its 1-based folio (page) position.
QHash<Element *, int> folioIndex(QETProject &project)
{
	QHash<Element *, int> folio;
	int index = 0;
	const QList<Diagram *> diagrams = project.diagrams();
	for (Diagram *diagram : diagrams) {
		++index;
		const QList<Element *> elements = diagram->elements();
		for (Element *e : elements)
			folio.insert(e, index);
	}
	return folio;
}

/// From-to wiring list: one row per conductor, each endpoint resolved to its
/// element label and terminal name.
///
/// Most symbols leave their terminals unnamed, so each end also carries the
/// terminal's index -- the one the scripting API's addConductor() takes,
/// empty for two terminals at one point -- and its uuid, the one the project
/// file names the terminal by. These come last, after the columns the list
/// has always had, so a reader of those is not disturbed.
///
/// Reads wiring_list_view out of the project database. --export-cables produces
/// the same logical list from the document XML instead, and the two are meant
/// to agree: running both and diffing them is a direct check that the database
/// still describes the project, which is otherwise only observable through the
/// GUI.
int exportWiring(QETProject &project, const QString &output)
{
	// Read from the project database, with every end which is a junction
	// already resolved to where the junction's chain leads: the one rule
	// this export and the CSV one share, so the two lists agree (#1500).
	int unassigned_junctions = 0;
	QString error;
	const QVector<WiringListRow> rows =
		wiringListRows(&project, &unassigned_junctions, &error);
	if (!error.isEmpty()) {
		err << "Wiring list query failed: " << error << "\n";
		return 1;
	}

	QString csv = WiringListRow::columns().join(";") % "\n";
	for (const WiringListRow &row : rows) {
		QStringList values;
		for (const QString &value : row.values())
			values << csvField(value);
		csv += values.join(";") % "\n";
	}

		//Written through QSaveFile so a failure part-way leaves the previous
		//file intact rather than a truncated one, and with a UTF-8 byte order
		//mark: without it Excel opens a .csv as the local 8-bit codepage and
		//mangles any accented element label. Qt writes UTF-8 by default, so
		//the bytes were already right -- the mark is what tells Excel so.
	QSaveFile file(output);
	if (!file.open(QIODevice::WriteOnly)) {
		err << "Cannot open '" << output << "' for writing.\n";
		return 1;
	}
	static const char utf8_bom[] = "\xEF\xBB\xBF";
	file.write(utf8_bom, 3);
	file.write(csv.toUtf8());
	if (!file.commit()) {
		err << "Cannot write '" << output << "': " << file.errorString() << "\n";
		return 1;
	}
	out << "Exported " << rows.size() << " conductor(s) -> " << output << "\n";
		//Said rather than left out: those rows end at the junction, which
		//is what the export would otherwise not tell anyone about
	if (unassigned_junctions > 0)
		out << unassigned_junctions << " junction(s) have rows that still "
			   "end at a junction: no source connection set, or no way on "
			   "to follow.\n";
	return 0;
}

/// Electrical nets: groups of terminals joined into one potential.
/// Walks QET's own potential graph, so each net is a connected component
/// of terminals across all folios. The ground truth for connectivity.
int exportNets(QETProject &project, const QString &output)
{
	const QHash<Element *, int> folio = folioIndex(project);

	QList<Conductor *> all_conductors;
	const QList<Diagram *> diagrams = project.diagrams();
	for (Diagram *diagram : diagrams)
		all_conductors << diagram->conductors();

	QSet<Conductor *> visited;
	QJsonArray nets;
	int net_no = 0;
	for (Conductor *c : all_conductors) {
		if (visited.contains(c))
			continue;

		// The whole potential this conductor belongs to. relatedPotential-
		// Conductors() also fills t_list with every terminal in the net
		// (following folio reports and terminal blocks too).
		QList<Terminal *> t_list;
		QSet<Conductor *> group = c->relatedPotentialConductors(true, &t_list);
		group.insert(c);
		for (Conductor *g : group)
			visited.insert(g);
		if (c->terminal1) t_list << c->terminal1;
		if (c->terminal2) t_list << c->terminal2;

		// Wire number: smallest non-empty conductor text (deterministic).
		QStringList wire_nos;
		for (Conductor *g : group)
			if (!g->properties().text.isEmpty())
				wire_nos << g->properties().text;
		wire_nos.sort();

		++net_no;
		QJsonArray terminals;
		QSet<Terminal *> seen;
		for (Terminal *t : t_list) {
			if (!t || seen.contains(t))
				continue;
			seen.insert(t);
			Element *pe = t->parentElement();
			QJsonObject to;
			to["element"]  = pe ? elementLabel(pe) : QString();
			to["terminal"] = t->name();
			to["folio"]    = pe ? folio.value(pe, 0) : 0;
			terminals.append(to);
		}
		QJsonObject net;
		net["net"]       = net_no;
		net["wire_no"]   = wire_nos.value(0);
		net["terminals"] = terminals;
		nets.append(net);
	}

	QJsonObject root;
	root["project"] = project.title();
	root["nets"]    = nets.size();
	root["list"]    = nets;

	QFile file(output);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		err << "Cannot open '" << output << "' for writing.\n";
		return 1;
	}
	file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
	file.close();
	out << "Exported " << nets.size() << " net(s) -> " << output << "\n";
	return 0;
}

/// Cross-references: each linkable element (coil / contact / report) and the
/// elements it links to, flagging masters/slaves with no link as unresolved.
int exportLinks(QETProject &project, const QString &output)
{
	const QHash<Element *, int> folio = folioIndex(project);

	QString csv("element;link_type;linked_to;folio;status\n");
	int linkable = 0, unresolved = 0;

	const QList<Diagram *> diagrams = project.diagrams();
	for (Diagram *diagram : diagrams) {
		const QList<Element *> elements = diagram->elements();
		for (Element *e : elements) {
			if (e->linkType() == Element::Simple)
				continue;
			++linkable;

			const QList<Element *> linked = e->linkedElements();
			QStringList names;
			for (Element *le : linked)
				names << elementLabel(le) % "(f"
					   % QString::number(folio.value(le, 0)) % ")";

			QString status = "linked";
			if ((e->linkType() == Element::Master
				 || e->linkType() == Element::Slave)
				&& linked.isEmpty()) {
				status = "UNRESOLVED";
				++unresolved;
			}

			csv += csvField(elementLabel(e)) % ";"
				 % e->linkTypeToString() % ";"
				 % csvField(names.join(", ")) % ";"
				 % QString::number(folio.value(e, 0)) % ";"
				 % status % "\n";
		}
	}

	QFile file(output);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		err << "Cannot open '" << output << "' for writing.\n";
		return 1;
	}
	QTextStream fout(&file);
	fout << csv;
	file.close();
	out << "Exported " << linkable << " linkable element(s), "
		<< unresolved << " unresolved -> " << output << "\n";
	return 0;
}

/// "Check project" headless: every check of the Check panel
/// (ProjectCheck::runAll()) as JSON, to stdout or to @p output. Exit code 0
/// when no finding is an error, 3 when at least one is (so a script can
/// stop on it), 1 when the file cannot be written.
int checkProject(QETProject &project, const QString &output)
{
	const QJsonObject report = ProjectCheck::toJson(ProjectCheck::runAll(&project), &project);
	const QByteArray json = QJsonDocument(report).toJson(QJsonDocument::Indented);
	const QJsonObject summary = report.value(QStringLiteral("summary")).toObject();
	const int errors = summary.value(QStringLiteral("errors")).toInt();
	const int warnings = summary.value(QStringLiteral("warnings")).toInt();
	const int infos = summary.value(QStringLiteral("infos")).toInt();
	if (output.isEmpty() || output == QLatin1String("-")) {
		out << QString::fromUtf8(json);
	} else {
		QFile file(output);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
			err << "Cannot open '" << output << "' for writing.\n";
			return 1;
		}
		file.write(json);
		file.close();
		out << "Checked plan: " << errors << " error(s), " << warnings
			<< " warning(s), " << infos << " note(s) -> " << output << "\n";
	}
	return errors ? 3 : 0;
}

/// Each device with a device type: is it drawn the way its type says?
/// One row per device (DeviceTypeCheck::check()): its type, how many parts
/// the type has, how many symbols it is drawn with, the parts not drawn
/// (listed) and the symbols beyond the type. status: complete, incomplete,
/// or unknown when the project has no type of that name.
int exportDeviceTypeCheck(QETProject &project, const QString &output)
{
	const QHash<Element *, int> folio = folioIndex(project);
	QList<Element *> unlinked;
	const auto devices = DeviceTypeCheck::check(&project, &unlinked);

	QString csv("device;device_type;status;parts;symbols;not_drawn;beyond;missing;folio;extra\n");
	int complete = 0, incomplete = 0, unknown = 0;
	for (const auto &device : devices) {
		QString status;
		QStringList missing;
		if (!device.known()) {
			status = "unknown";
			++unknown;
		} else {
			status = device.complete() ? "complete" : "incomplete";
			++(device.complete() ? complete : incomplete);
			for (int part : device.missing)
				missing << DeviceTypeCheck::partText(device.type.parts.at(part));
		}
		Element *head = device.symbols.isEmpty() ? nullptr : device.symbols.first().data();
		csv += csvField(device.designation) % ";"
			 % csvField(device.typeName) % ";"
			 % status % ";"
			 % (device.known() ? QString::number(device.type.parts.size()) : QString()) % ";"
			 % QString::number(device.symbols.size()) % ";"
			 % (device.known() ? QString::number(device.missing.size()) : QString()) % ";"
			 % (device.known() ? QString::number(device.beyond()) : QString()) % ";"
			 % csvField(missing.join(", ")) % ";"
			 % QString::number(folio.value(head, 0)) % ";"
			 % QString::number(DeviceTypeCheck::extraContacts(device)) % "\n";
	}
		//A contact linked to no master: a row of its own, status unlinked
	for (Element *contact : unlinked) {
		csv += csvField(elementLabel(contact)) % ";"
			 % csvField(DeviceTypeCheck::typeNameOf(contact->elementInformations()))
			 % ";unlinked;;1;;;;"
			 % QString::number(folio.value(contact, 0)) % ";\n";
	}

	QFile file(output);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		err << "Cannot open '" << output << "' for writing.\n";
		return 1;
	}
	QTextStream fout(&file);
	fout << csv;
	file.close();
	out << "Checked " << devices.size() << " device(s) with a device type: "
		<< complete << " complete, " << incomplete << " incomplete, "
		<< unknown << " of an unknown type, " << unlinked.size()
		<< " contact(s) linked to no master -> " << output << "\n";
	return 0;
}

/// The project's device types as a list (DeviceTypeList): --export-device-types
/// gives every part of each type and its symbols' terminals,
/// --export-device-types-brief one row per type with the symbols using it.
int exportDeviceTypeList(QETProject &project, const QString &output, bool brief)
{
	const QHash<QString, int> used = DeviceTypeList::usedBy(&project);
	const QList<DeviceType> types = project.deviceTypes();
	const QString csv = DeviceTypeList::csv(types, brief ? DeviceTypeList::Kind::Brief
														 : DeviceTypeList::Kind::Full,
											brief ? &used : nullptr);
	QString error;
	if (!DeviceTypeList::write(output, csv, &error)) {
		err << "Cannot write '" << output << "': " << error << "\n";
		return 1;
	}
	out << "Listed " << types.size() << " device type(s) -> " << output << "\n";
	return 0;
}

/// Round-trip: load the project and write its XML back out, so an external
/// diff can reveal markup QET silently normalises (tolerated-but-invalid XML).
/// What can be read of the damaged .qetz @p input, written to the .qet
/// @p output, and what was left out (QetContainer::recover()).
int recoverProject(const QString &input, const QString &output)
{
	QFile file(input);
	if (!file.open(QIODevice::ReadOnly)) {
		err << "Cannot read '" << input << "': " << file.errorString() << "\n";
		return 2;
	}
	const QByteArray content = file.readAll();
	if (!QetContainer::isZip(content)) {
		err << "'" << input << "' is not a .qetz project; a .qet opens as it is, or can be fixed in a text editor\n";
		return 2;
	}
	QDomDocument document;
	QStringList lost;
	QString error;
	if (!QetContainer::recover(content, &document, &lost, &error)) {
		err << "Nothing can be recovered from '" << input << "': " << error << "\n";
		return 1;
	}
	if (!QET::writeXmlFile(document, output, &error)) {
		err << "Cannot write '" << output << "': " << error << "\n";
		return 1;
	}
	out << "Recovered " << input << " -> " << output << "\n";
	if (lost.isEmpty()) {
		out << "Nothing was left out.\n";
	} else {
		out << "Left out:\n";
		for (const QString &item : std::as_const(lost))
			out << "  " << item << "\n";
	}
	return 0;
}

int resaveProject(QETProject &project, const QString &output)
{
		//A .qetdb (experimental), whatever the setting: tests and conversions
	if (QetDbFile::isDatabasePath(output)) {
		QString error;
		if (!QetDbFile::save(*project.dataBase(), output, &error)) {
			err << "Cannot write '" << output << "': " << error << "\n";
			return 1;
		}
		out << "Re-saved project -> " << output << "\n";
		return 0;
	}
	if (QETProject::isContainerPath(output)) {
		QString error;
		if (!project.writeContainerFile(output, &error)) {
			err << "Cannot write '" << output << "': " << error << "\n";
			return 1;
		}
		out << "Re-saved project -> " << output << "\n";
		return 0;
	}
		//Folios not built (lazy folios) written as read when that is
		//what building them would write (QETProject::toXml())
	QHash<int, QByteArray> kept_folios;
	const QDomDocument doc = project.toXml(&kept_folios);
		//Through a temporary file, as the GUI saves: a run stopped
		//part-way leaves the previous file
	QString error;
	if (!QET::writeXmlText(QETProject::spliceKeptFolios(doc.toString(4), kept_folios), output, &error)) {
		err << "Cannot write '" << output << "': " << error << "\n";
		return 1;
	}
	out << "Re-saved project -> " << output << "\n";
	return 0;
}

/// Stamp title-block fields onto every folio (and the project default), then
/// save.  Each assignment is "key=value".  Standard keys map to the documented
/// title-block fields; "date=today" uses the current date; any other key is
/// stored as a custom title-block field.  Aimed at CI/revision workflows
/// (e.g. set revision + date before exporting a new revision).
int setTitleBlock(QETProject &project, const QString &output,
				  const QStringList &assignments)
{
	if (assignments.isEmpty()) {
		err << "No field assignments given (expected key=value).\n";
		return 2;
	}

	// Parse "key=value" assignments up front so a bad one fails before writing.
	QList<QPair<QString, QString>> fields;
	for (const QString &a : assignments) {
		const int eq = a.indexOf('=');
		if (eq <= 0) {
			err << "Bad assignment '" << a << "' (expected key=value).\n";
			return 2;
		}
		const QString key = a.left(eq);
		const QString val = a.mid(eq + 1);
		if (key.compare("date", Qt::CaseInsensitive) == 0
			&& val.compare("today", Qt::CaseInsensitive) != 0
			&& !QDate::fromString(val, Qt::ISODate).isValid()) {
			err << "Bad date '" << val << "' (expected YYYY-MM-DD or 'today').\n";
			return 2;
		}
		fields << qMakePair(key, val);
	}

	auto apply = [&](TitleBlockProperties &p) {
		for (const auto &f : fields) {
			const QString k = f.first.toLower();
			const QString &v = f.second;
			if      (k == "title")    p.title    = v;
			else if (k == "author")   p.author   = v;
			else if (k == "filename") p.filename = v;
			else if (k == "plant")    p.plant    = v;
			else if (k == "location") p.locmach  = v;
			else if (k == "revision") p.indexrev = v;
			else if (k == "version")  p.version  = v;
			else if (k == "date") {
				p.date = (v.compare("today", Qt::CaseInsensitive) == 0)
						 ? QDate::currentDate()
						 : QDate::fromString(v, Qt::ISODate);
				// An explicit date is only honoured when the folio is in
				// "use the date value" mode (not "now"/"null").
				p.useDate = TitleBlockProperties::UseDateValue;
			}
			else // unknown key -> custom title-block field
				p.context.addValue(f.first, v);
		}
	};

	// Project default (the template applied to new folios).
	TitleBlockProperties def = project.defaultTitleBlockProperties();
	apply(def);
	project.setDefaultTitleBlockProperties(def);

	// Every existing folio's own title block.
	int folios = 0;
	const QList<Diagram *> diagrams = project.diagrams();
	for (Diagram *diagram : diagrams) {
		TitleBlockProperties p =
			diagram->border_and_titleblock.exportTitleBlock();
		apply(p);
		diagram->border_and_titleblock.importTitleBlock(p);
		++folios;
	}

	if (QETProject::isContainerPath(output)) {
		QString error;
		if (!project.writeContainerFile(output, &error)) {
			err << "Cannot write '" << output << "': " << error << "\n";
			return 1;
		}
	} else {
			//Folios not built (lazy folios) written as read when that
			//is what building them would write (QETProject::toXml())
		QHash<int, QByteArray> kept_folios;
		const QDomDocument doc = project.toXml(&kept_folios);
		QString error;
		if (!QET::writeXmlText(QETProject::spliceKeptFolios(doc.toString(4), kept_folios), output, &error)) {
			err << "Cannot write '" << output << "': " << error << "\n";
			return 1;
		}
	}
	out << "Stamped " << fields.size() << " field(s) on "
		<< folios << " folio(s) -> " << output << "\n";
	return 0;
}

/// One CSV field as RFC 4180 has it: quoted when it holds a comma, a quote
/// or a line break, its quotes doubled.
QString rfcCsvField(const QString &value)
{
	if (value.contains(QLatin1Char(',')) || value.contains(QLatin1Char('"'))
			|| value.contains(QLatin1Char('\n')) || value.contains(QLatin1Char('\r')))
	{
		QString escaped = value;
		escaped.replace(QLatin1Char('"'), QStringLiteral("\"\""));
		return QLatin1Char('"') % escaped % QLatin1Char('"');
	}
	return value;
}

/// Run @p sql on the .qetdb @p file itself, without opening the project:
/// on a copy, once it passes the checks of an open, read-only, with the
/// public views made as TEMP views (a file saved before them answers the
/// same). @p print is given the rows.
/// @return 0, 1 with @p error saying why, or 2 when @p print failed and
/// said why itself
template <typename Print>
int queryDatabaseFile(const QString &file, const QString &sql, Print print, QString *error)
{
	QTemporaryDir dir;
	const QString copy = dir.filePath(QStringLiteral("query.qetdb"));
	if (!dir.isValid() || !QFile::copy(file, copy)) {
		*error = QStringLiteral("the file could not be read");
		return 1;
	}
	if (!QetDbFile::isReadable(copy, error)) return 1;
	const QString name = QStringLiteral("qet_query_") + QUuid::createUuid().toString();
	int result = 1;
	{
		QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), name);
		db.setDatabaseName(copy);
		db.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
		if (!db.open()) {
			*error = db.lastError().text();
		} else {
			{
				QSqlQuery views(db);
				for (QString statement : projectDataBase::publicViewStatements()) {
					statement.replace(QStringLiteral("CREATE VIEW "), QStringLiteral("CREATE TEMP VIEW "));
					if (!views.exec(statement)) {
						*error = views.lastError().text();
						break;
					}
				}
			}
				//The potential check's TEMP table and views, made on this
				//connection when the query names one
			static const QRegularExpression potential_objects(
					QStringLiteral("\\b(wire_potential_view|potential_view|potential_member|potential_mismatch)\\b"),
					QRegularExpression::CaseInsensitiveOption);
			if (error->isEmpty() && potential_objects.match(sql).hasMatch())
				projectDataBase::potentialTypeMismatchesOf(db);
			if (error->isEmpty() && projectDataBase::isReadOnlySelect(sql, error)) {
				QSqlQuery query = QETSql::execReadOnly(db, sql, error);
				if (error->isEmpty() && query.lastError().isValid())
					*error = query.lastError().text();
				if (error->isEmpty())
					result = print(file, query) ? 0 : 2;
			}
			db.close();
		}
	}
	QSqlDatabase::removeDatabase(name);
	return result;
}

/// --query: run @p sql on the project database of each of @p files in
/// turn, as qet.query() runs it (read-only, one SELECT), and print every
/// row with the file it came from first: CSV, or a JSON array of objects
/// with @p json. A .qetdb is read itself, without opening the project
/// (QET_QUERY_OPEN_PROJECT=1 opens it as the others). A file that cannot
/// be opened or queried is reported on stderr and the others still run;
/// the exit code says whether any failed.
int queryProjects(const QString &sql, const QStringList &files, bool json)
{
	QStringList columns;
	QJsonArray json_rows;
		//The rows of @p query, run on @p file; false, said on stderr, if
		//they could not all be printed
	const auto print = [&](const QString &file, QSqlQuery &query) {
		const QSqlRecord record = query.record();
		QStringList names{QStringLiteral("project")};
		for (int i = 0 ; i < record.count() ; ++i)
			names << record.fieldName(i);
		if (columns.isEmpty()) {
			columns = names;
			if (!json) {
				QStringList header;
				for (const QString &name : std::as_const(columns))
					header << rfcCsvField(name);
				out << header.join(QLatin1Char(',')) << "\n";
			}
		} else if (names != columns) {
			err << file << ": the query gives other columns than for the first file\n";
			return false;
		}
		int rows = 0;
		bool complete = true;
		while (query.next()) {
				//A query that never stops producing rows (a recursive CTE)
				//stops here, as qet.query() does
			if (++rows > projectDataBase::MaxResultRows) {
				err << file << ": result cut at " << projectDataBase::MaxResultRows
					<< " rows; add a LIMIT or a WHERE clause\n";
				complete = false;
				break;
			}
			if (json) {
				QJsonObject row{{QStringLiteral("project"), file}};
				for (int i = 0 ; i < record.count() ; ++i)
					row.insert(record.fieldName(i), QJsonValue::fromVariant(query.value(i)));
				json_rows.append(row);
			} else {
				QStringList fields{rfcCsvField(file)};
				for (int i = 0 ; i < record.count() ; ++i)
					fields << (query.isNull(i) ? QString() : rfcCsvField(query.value(i).toString()));
				out << fields.join(QLatin1Char(',')) << "\n";
			}
		}
		out.flush();
		return complete;
	};

	const bool open_every_project = qEnvironmentVariableIntValue("QET_QUERY_OPEN_PROJECT") == 1;
	bool failed = false;
	for (const QString &file : files) {
		if (!QFileInfo::exists(file)) {
			err << file << ": not found\n";
			failed = true;
			continue;
		}
		QString error;
		if (QetDbFile::isDatabasePath(file) && !open_every_project) {
			const int result = queryDatabaseFile(file, sql, print, &error);
			if (result == 1) err << file << ": " << error << "\n";
			if (result != 0) failed = true;
			continue;
		}
		QETProject project(file);
		if (project.state() != QETProject::Ok) {
			err << file << ": cannot be opened (state " << project.state() << ")\n";
			failed = true;
			continue;
		}
		QSqlQuery query = project.dataBase()->newQuery(sql, &error);
		if (error.isEmpty() && query.lastError().isValid())
			error = query.lastError().text();
		if (!error.isEmpty()) {
			err << file << ": " << error << "\n";
			failed = true;
			continue;
		}
		if (!print(file, query)) failed = true;
	}
	if (json)
		out << QJsonDocument(json_rows).toJson(QJsonDocument::Indented);
	return failed ? 1 : 0;
}


/// The changes as lines, or as a JSON array with --json
void printChanges(const QList<QetRevision::Change> &changes, const QetRevision::Fingerprint &now,
				  const QetRevision::Fingerprint &before, bool json)
{
	if (!json) {
		for (const QString &line : QetRevision::describe(changes, now, before))
			out << line << "\n";
		out << changes.size() << " change(s)\n";
		return;
	}
	QJsonArray rows;
	for (const QetRevision::Change &c : changes) {
		const QetRevision::Item &i = c.item();
		QJsonObject row;
		row[QStringLiteral("change")] = c.kind == QetRevision::Change::Added ? QStringLiteral("added")
				: c.kind == QetRevision::Change::Removed ? QStringLiteral("removed") : QStringLiteral("changed");
		static const QHash<QChar, QString> kinds{{QLatin1Char('e'), QStringLiteral("symbol")},
			{QLatin1Char('c'), QStringLiteral("wire")}, {QLatin1Char('t'), QStringLiteral("text")},
			{QLatin1Char('s'), QStringLiteral("shape")}, {QLatin1Char('i'), QStringLiteral("image")},
			{QLatin1Char('f'), QStringLiteral("folio")}};
		row[QStringLiteral("kind")] = kinds.value(i.kind);
		row[QStringLiteral("folio")] = i.folio_index;
		row[QStringLiteral("folio_name")] = (c.kind == QetRevision::Change::Removed ? before : now)
				.folio_names.value(i.folio);
		row[QStringLiteral("name")] = i.name;
		row[QStringLiteral("uuid")] = i.uuid;
		rows.append(row);
	}
	out << QJsonDocument(rows).toJson(QJsonDocument::Indented);
}

/// --close-revision <project> <index> [text...] [--no-pdf]
int closeRevision(QETProject &project, const QStringList &rest, bool pdf)
{
	if (rest.size() < 2) {
		err << "Usage: qelectrotech --close-revision <project> <index> [text...] [--no-pdf]\n";
		return 2;
	}
	const ProjectRevisions::CloseResult r = project.revisions()->closeRevision(
				rest.at(1), rest.mid(2).join(QLatin1Char(' ')), pdf, true);
	if (!r.ok) {
		err << "Revision not closed: " << r.error << "\n";
		return 1;
	}
	out << "Revision " << rest.at(1).trimmed() << " closed: " << r.changes
		<< " change(s) since the last revision\n";
	if (!r.pdf.isEmpty()) out << "PDF: " << QDir::toNativeSeparators(r.pdf) << "\n";
	out << "Snapshot: " << QDir::toNativeSeparators(r.snapshot) << "\n";
	return 0;
}

/// --revisions <project>
int listRevisions(QETProject &project, bool json)
{
	ProjectRevisions *revisions = project.revisions();
	const QList<ProjectRevisions::Entry> entries = revisions->entries();
	if (!json) {
		if (entries.isEmpty()) out << "No revision closed yet; the next is " << revisions->proposedIndex() << "\n";
		for (const ProjectRevisions::Entry &e : entries)
			out << e.index << "\t" << e.date << "\t" << e.author << "\t" << e.text
				<< (e.snapshot.isEmpty() ? QString() : QStringLiteral("\t") + e.snapshot) << "\n";
		out << "Marks: " << (revisions->marksShown() ? "on" : "off") << "\n";
	}
	if (!revisions->hasBaseline()) return 0;
	if (!json) out << "Since revision " << revisions->lastIndex() << ":\n";
	const QetRevision::Fingerprint before = revisions->baseline();
	const QetRevision::Fingerprint now = QetRevision::fingerprint(&project);
	printChanges(QetRevision::compare(before, now), now, before, json);
	return 0;
}

/// --compare-revisions <older> <newer> [<output.pdf>]
int compareRevisions(const QStringList &rest, bool json)
{
	if (rest.size() < 2) {
		err << "Usage: qelectrotech --compare-revisions <older> <newer> [<output.pdf>] [--json]\n";
		return 2;
	}
	QETProject newer(rest.at(1));
	if (newer.state() != QETProject::Ok) {
		err << "Failed to open project: " << rest.at(1) << "\n";
		return 1;
	}
	QString older_path = rest.at(0), label;
	if (!QFileInfo::exists(older_path)) {
			//A revision index of the newer project
		for (const ProjectRevisions::Entry &e : newer.revisions()->entries())
			if (e.index == rest.at(0)) older_path = newer.revisions()->snapshotPath(e);
		if (!QFileInfo::exists(older_path)) {
			err << "Neither a file nor a revision of " << rest.at(1) << ": " << rest.at(0) << "\n";
			return 2;
		}
	}
	QString error;
	const QetRevision::Fingerprint before = QetRevision::fingerprintOfFile(older_path, &error);
	if (!error.isEmpty()) {
		err << error << "\n";
		return 1;
	}
	const QetRevision::Fingerprint now = QetRevision::fingerprint(&newer);
	printChanges(QetRevision::compare(before, now), now, before, json);
	if (rest.size() >= 3) {
			//The revision the changes belong to: the newer's own when it is
			//a closed revision as it is (a snapshot), else the next one
		ProjectRevisions *revisions = newer.revisions();
		label = revisions->hasBaseline()
				&& QetRevision::compare(revisions->baseline(), now).isEmpty()
				? revisions->lastIndex() : revisions->proposedIndex();
		newer.revisions()->beginComparison(before, label);
		const int code = exportPdf(newer, rest.at(2));
		newer.revisions()->endComparison();
		return code;
	}
	return 0;
}

} // anonymous namespace

namespace CLIExport {

/// --export-tia-tags: the PLC channels of the project as tag table for
/// TIA Portal (.xlsx for the tag table editor, .xml for Openness)
int exportTiaTags(QETProject &project, const QString &output, const QHash<QString, QString> &opts)
{
	QStringList notes;
	const QVector<TiaPlcTags::Channel> channels = TiaPlcTags::channels(&project, &notes);
	QString error;
	if (!TiaPlcTags::write(output, channels, opts.value("tia-table"), opts.value("tia-culture"), &error)) {
		err << "Cannot write '" << output << "': " << error << "\n";
		return 1;
	}
	int written = 0, counted = 0;
	for (const auto &c : channels) {
		if (c.address.isEmpty()) continue;
		++written;
		if (c.autoAddress) ++counted;
	}
	for (const QString &note : notes) err << "note: " << note << "\n";
	out << "Exported " << written << " PLC tag(s) of " << channels.size() << " channel(s), "
		<< counted << " with an address counted from the module start -> " << output << "\n";
	return 0;
}

/// --compare-tia-tags: a TIA tag table against the PLC channels; the
/// differences as CSV (stdout or a file), --apply OUT.qet takes the names
/// and comments of the tags into the plan and saves it as OUT.qet
int compareTiaTags(QETProject &project, const QString &tags_path, const QString &report,
				   const QHash<QString, QString> &opts)
{
	QString error;
	const QVector<TiaPlcTags::Tag> tags = TiaPlcTags::read(tags_path, &error);
	if (tags.isEmpty() && !error.isEmpty()) {
		err << "Cannot read '" << tags_path << "': " << error << "\n";
		return 1;
	}
	QStringList notes;
	const TiaPlcTags::Comparison cmp = TiaPlcTags::compare(TiaPlcTags::channels(&project, &notes), tags);
	const QString csv = TiaPlcTags::reportCsv(cmp);
	if (report.isEmpty() || report == "-") {
		out << csv;
	} else {
		QSaveFile file(report);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
			err << "Cannot open '" << report << "' for writing.\n";
			return 1;
		}
		file.write(csv.toUtf8());
		file.commit();
	}
	using S = TiaPlcTags::Status;
	err << "Compared " << cmp.channels.size() << " channel(s) with " << tags.size() << " tag(s): "
		<< cmp.count(S::Same) << " same, "
		<< cmp.count(S::NameDiffers) + cmp.count(S::BothDiffer) << " name differs, "
		<< cmp.count(S::CommentDiffers) + cmp.count(S::BothDiffer) << " comment differs, "
		<< cmp.count(S::AddressDiffers) << " address differs, "
		<< cmp.count(S::MissingInTia) << " missing in TIA, "
		<< cmp.count(S::OnlyInTia) << " only in TIA, "
		<< cmp.otherTags << " other tag(s) (no I/Q address)\n";
	const QString apply_to = opts.value("apply");
	if (!apply_to.isEmpty()) {
		const int changed = TiaPlcTags::apply(cmp);
		err << "Took over " << changed << " name(s)/comment(s) from TIA.\n";
		return resaveProject(project, apply_to);
	}
	return 0;
}

bool isExportRequest(const QStringList &args)
{
	for (const QString &a : args)
		if (exportFlags().contains(a))
			return true;
	return false;
}

int run(const QStringList &args)
{
	// --show-terminals is a standalone switch (not tied to a position),
	// so pull it out before the positional project/output arguments are
	// collected below.
	QStringList filtered = args;
	const bool showTerminals = filtered.removeAll("--show-terminals") > 0;
	const bool dxfBlocks = filtered.removeAll("--dxf-blocks") > 0;
	const bool dxfAttributes = filtered.removeAll("--dxf-attributes") > 0;
	const bool svgSmallPictures = filtered.removeAll("--svg-small-pictures") > 0;
	// --no-slaves and --no-junctions leave rows out of --export-bom.
	const bool includeSlaves = filtered.removeAll("--no-slaves") == 0;
	const bool includeJunctions = filtered.removeAll("--no-junctions") == 0;
	// --online, --offline and --erp belong to the purchase list.
	const bool purchaseOnline = filtered.removeAll("--online") > 0;
	const bool purchaseOffline = filtered.removeAll("--offline") > 0;
	const bool purchaseErp = filtered.removeAll("--erp") > 0;
	const bool ignoreLock = filtered.removeAll("--ignore-lock") > 0;
	const bool json = filtered.removeAll("--json") > 0;
	const bool grid = filtered.removeAll("--grid") > 0;
	const bool noPdf = filtered.removeAll("--no-pdf") > 0;

	QString flag;
	QStringList rest;
	for (int i = 0; i < filtered.size(); ++i) {
		if (exportFlags().contains(filtered.at(i))) {
			flag = filtered.at(i);
			for (int j = i + 1; j < filtered.size(); ++j)
				rest << filtered.at(j);
			break;
		}
	}
	const QString format = exportFlags().value(flag);

	// --check-elements operates on an element file/directory, not a project.
	if (format == "check") {
		if (rest.isEmpty()) {
			err << "Usage: qelectrotech --check-elements "
				   "<element.elmt | directory> [--grid]\n";
			return 2;
		}
		return checkElements(rest.at(0), grid);
	}

	// --lock-status tells who has a project open (its lock file beside it),
	// whether that lock is left over (crash on this PC, or no heartbeat for
	// ProjectLock::orphan_minutes) and which OneDrive/SharePoint conflict
	// copies lie beside it. Reads only; the project itself is not opened.
	// Exit code: 0 free, 1 held by a live session, 3 left over, 2 usage.
	if (format == "lockstatus") {
		if (rest.isEmpty()) {
			err << "Usage: qelectrotech --lock-status <project> [--json]\n";
			return 2;
		}
		const QString path = QFileInfo(rest.at(0)).absoluteFilePath();
		const ProjectLock::Holder holder = ProjectLock::readHolder(path);
		const ProjectLock::Holder live = ProjectLock::liveHolder(path);
		const bool crashed = ProjectLock::isStale(holder);
		const bool orphaned = live.valid && ProjectLock::isOrphaned(live);
		const QString state = !holder.valid ? QStringLiteral("free")
				: crashed ? QStringLiteral("crashed")
				: !live.valid ? QStringLiteral("free")
				: orphaned ? QStringLiteral("orphaned")
				: QStringLiteral("held");
		const QStringList copies = ProjectLock::conflictCopies(
					path, holder.valid ? QStringList{holder.host} : QStringList());
		if (json) {
			QJsonObject o;
			o.insert("project", path);
			o.insert("lock_file", ProjectLock::lockPathFor(path));
			o.insert("state", state);
			if (holder.valid) {
				o.insert("user", holder.user);
				o.insert("host", holder.host);
				o.insert("pid", double(holder.pid));
				o.insert("app", holder.app);
				o.insert("since", holder.since.toUTC().toString(Qt::ISODate));
				o.insert("heartbeat", holder.heartbeat.toUTC().toString(Qt::ISODate));
				o.insert("heartbeat_age_s", double(holder.heartbeat.secsTo(QDateTime::currentDateTimeUtc())));
			}
			o.insert("conflict_copies", QJsonArray::fromStringList(copies));
			out << QJsonDocument(o).toJson(QJsonDocument::Indented);
		} else {
			out << "state: " << state << "\n";
			if (holder.valid)
				out << "held by: " << holder.user << " on " << holder.host << " (" << holder.app
					<< ") since " << holder.since.toLocalTime().toString(Qt::ISODate)
					<< ", heartbeat " << holder.heartbeat.secsTo(QDateTime::currentDateTimeUtc()) << " s ago\n";
			for (const QString &c : copies) out << "conflict copy: " << c << "\n";
		}
		out.flush();
		return state == QLatin1String("held") ? 1
			 : (state == QLatin1String("orphaned") || state == QLatin1String("crashed")) ? 3 : 0;
	}

	// --import-eplan turns manufacturer EPLAN DXF data into elements of a
	// collection folder, as the part import of the menu does.
	if (format == "importeplan") {
		QStringList positional;
		QHash<QString, QString> opts;
		for (int i = 0; i < rest.size(); ++i) {
			const QString &a = rest.at(i);
			if (a.startsWith("--") && i + 1 < rest.size())
				opts.insert(a.mid(2), rest.at(++i));
			else
				positional << a;
		}
		if (positional.size() < 2) {
			err << "Usage: qelectrotech --import-eplan <package.zip | folder | file.dxf | file.dwg> "
			       "<collection_dir> [--category C] [--manufacturer M --type T] [--edz part.edz] "
			       "[--wspak part.wspak]\n";
			return 2;
		}
		EplanPartImporter importer;
		const QString source = positional.at(0);
		const QString suffix = QFileInfo(source).suffix();
		const bool single_dxf = suffix.compare("dxf", Qt::CaseInsensitive) == 0;
		const bool edz = suffix.compare("edz", Qt::CaseInsensitive) == 0;
		const bool dwg = suffix.compare("dwg", Qt::CaseInsensitive) == 0;
		bool ok;
		if (dwg) {
			// manufacturer and type may come from the WSCAD Universe package instead
			ok = importer.importDwg(source, opts.value("wspak"), opts.value("manufacturer"),
						opts.value("category", "Allgemein"), opts.value("type"),
						positional.at(1));
		} else if (edz) {
			if (opts.value("manufacturer").isEmpty() || opts.value("type").isEmpty()) {
				err << "--import-eplan: an .edz part needs --manufacturer and --type\n";
				return 2;
			}
			ok = importer.importEdz(source, opts.value("manufacturer"),
						opts.value("category", "Allgemein"), opts.value("type"),
						positional.at(1));
		} else if (single_dxf) {
			if (opts.value("manufacturer").isEmpty() || opts.value("type").isEmpty()) {
				err << "--import-eplan: a single DXF needs --manufacturer and --type\n";
				return 2;
			}
			ok = importer.importDxf(source, opts.value("edz"), opts.value("manufacturer"),
						opts.value("category", "Allgemein"), opts.value("type"),
						positional.at(1));
		} else {
			ok = importer.importPackage(source, opts.value("category"), positional.at(1));
		}
		for (const QString &m : importer.messages())
			out << m << "\n";
		for (const QString &p : importer.writtenPaths())
			out << "-> " << QDir::toNativeSeparators(p) << "\n";
		out.flush();
		if (!ok) {
			err << source << ": " << importer.errorString() << "\n";
			return 1;
		}
		return 0;
	}

	// --fetch-parts works off a list of missing parts: the manufacturer data
	// (EPLAN macro, DXF, STEP) loaded from the portals and imported, as
	// "Part import > Manufacturer portals" does; --portal-* show and keep
	// the accounts of the portals (the secret is read from stdin, never
	// from the command line)
	if (format == "fetchparts") {
		QStringList positional;
		QHash<QString, QString> opts;
		QSet<QString> switches;
		for (int i = 0; i < rest.size(); ++i) {
			const QString &a = rest.at(i);
			if (a == "--dry-run" || a == "--force" || a == "--no-dataportal")
				switches.insert(a.mid(2));
			else if (a.startsWith("--") && i + 1 < rest.size())
				opts.insert(a.mid(2), rest.at(++i));
			else
				positional << a;
		}
		if (positional.isEmpty()) {
			err << "Usage: qelectrotech --fetch-parts <list.txt> [--collection DIR] [--raw-dir DIR] "
			       "[--3d-dir DIR] [--links FILE] [--manufacturer SIE,RIT] [--limit N] [--pause MS] "
			       "[--dry-run] [--force] [--no-dataportal] [--report FILE] [--json]\n";
			return 2;
		}
		QString error;
		const QList<FetchItem> items = PartFetcher::readList(positional.at(0), &error);
		if (items.isEmpty()) {
			err << positional.at(0) << ": " << error << "\n";
			return 1;
		}
		FetchOptions options;
		options.collection_dir = opts.value("collection");
		options.raw_dir = opts.value("raw-dir");
		options.model_dir = opts.value("3d-dir");
		if (opts.contains("links")) options.link_files << opts.value("links");
		const QStringList makers = opts.value("manufacturer").split(',', Qt::SkipEmptyParts);
		for (const QString &m : makers)
			options.makers << m.trimmed();
		options.limit = opts.value("limit").toInt();
		if (opts.contains("pause")) options.pause_ms = qMax(0, opts.value("pause").toInt());
		options.dry_run = switches.contains("dry-run");
		options.force = switches.contains("force");
		options.use_dataportal = !switches.contains("no-dataportal");
		PartFetcher fetcher;
		if (!json)
			QObject::connect(&fetcher, &PartFetcher::progress, [](int done, int total, const QString &line) {
				err << "[" << done << "/" << total << "] " << line << "\n";
				err.flush();
			});
		const QList<FetchResult> results = fetcher.run(items, options);
		const QJsonObject report = PartFetcher::toJson(results);
		QString text;
		for (const FetchResult &r : results) {
			text += QStringLiteral("%1  %2 %3\n").arg(r.status, -8).arg(r.maker, r.order);
			for (const QString &n : r.notes) text += QStringLiteral("          %1\n").arg(n);
		}
		QStringList summary;
		const QJsonObject counts = report.value("summary").toObject();
		for (auto it = counts.constBegin(); it != counts.constEnd(); ++it)
			summary << QStringLiteral("%1 %2").arg(it.value().toInt()).arg(it.key());
		text += QStringLiteral("\n%1 parts: %2; 3Dfindit requests left today: %3\n")
				.arg(results.size()).arg(summary.join(", ")).arg(report.value("3dfindit_left").toInt());
		if (opts.contains("report")) {
			QSaveFile f(opts.value("report"));
			if (f.open(QIODevice::WriteOnly)) {
				f.write(json ? QJsonDocument(report).toJson() : text.toUtf8());
				f.commit();
			}
		}
		if (json) out << QJsonDocument(report).toJson();
		else out << text;
		out.flush();
		return 0;
	}
	if (format == "portalstatus") {
		const QJsonObject status = ManufacturerPortals::statusAll();
		if (json) {
			out << QJsonDocument(status).toJson();
		} else {
			if (!status.value("credential_store").toBool()) out << "No credential store on this system.\n";
			const QJsonArray portals = status.value("portals").toArray();
			for (const QJsonValue &v : portals) {
				const QJsonObject p = v.toObject();
				out << p.value("id").toString() << ": "
				    << (p.value("logged_in").toBool() ? "logged in" : "not logged in");
				if (!p.value("user").toString().isEmpty()) out << " as " << p.value("user").toString();
				if (p.contains("limit"))
					out << ", " << p.value("used").toInt() << " of " << p.value("limit").toInt()
					    << " CAD requests used in 24 h, " << p.value("left").toInt() << " left";
				out << "\n";
			}
		}
		out.flush();
		return 0;
	}
	if (format == "portallogin" || format == "portallogout") {
		const QString id = rest.value(0);
		if (ManufacturerPortals::portal(id).id.isEmpty()) {
			err << "Usage: qelectrotech " << flag << " <dataportal|siemens|rittal|3dfindit|keyence>"
			    << (format == "portallogin" ? " [--user NAME]  (password or token on stdin)" : "") << "\n";
			return 2;
		}
		QString error;
		if (format == "portallogout") {
			if (!PortalCredentials::remove(id, &error)) {
				err << error << "\n";
				return 1;
			}
			out << id << ": logged out\n";
			out.flush();
			return 0;
		}
		PortalAccount account = PortalCredentials::load(id);
		const int user_at = int(rest.indexOf("--user"));
		if (user_at > 0 && user_at + 1 < rest.size()) account.user = rest.at(user_at + 1);
		QTextStream in(stdin);
		const QString secret = in.readLine().trimmed();
		if (!secret.isEmpty()) account.secret = secret;
		if (id == "dataportal") {
			error = ManufacturerPortals::checkDataPortalToken(account.secret);
			if (!error.isEmpty()) {
				err << id << ": " << error << "\n";
				return 1;
			}
		} else if (account.user.isEmpty()) {
			err << id << ": --user is needed\n";
			return 2;
		}
		if (!PortalCredentials::store(id, account, &error)) {
			err << id << ": " << error << "\n";
			return 1;
		}
		out << id << ": account stored in the credential store\n";
		out.flush();
		return 0;
	}
	if (format == "portalrequest") {
		const QString id = rest.value(0);
		const QString order = rest.value(1);
		if (ManufacturerPortals::portal(id).daily_limit <= 0 || order.isEmpty()) {
			err << "Usage: qelectrotech --portal-request 3dfindit <order number>\n";
			return 2;
		}
		QString error;
		if (!ManufacturerPortals::registerRequest(id, order, &error)) {
			err << id << ": " << error << "\n";
			return 1;
		}
		out << id << ": " << ManufacturerPortals::requestsLeft(id) << " requests left\n";
		out.flush();
		return 0;
	}

	// --recover reads what it can of a damaged .qetz, which a project
	// cannot be opened from.
	if (format == "recover") {
		if (rest.size() < 2) {
			err << "Usage: qelectrotech --recover <damaged.qetz> <output.qet>\n";
			return 2;
		}
		return recoverProject(rest.at(0), rest.at(1));
	}

	// --history lists the saves a .qetdb keeps; --history-on and
	// --history-off make it keep them or drop them (QETDB-S6-SCOPE.md)
	if (format == "history" || format == "historyon" || format == "historyoff") {
		if (rest.isEmpty() || !QetDbFile::isDatabasePath(rest.at(0))) {
			err << "Usage: qelectrotech " << flag << " <project.qetdb>\n";
			return 2;
		}
		QString error;
		if (format == "history") {
			const QStringList lines = QetDbFile::history(rest.at(0), &error);
			if (lines.isEmpty()) {
				err << rest.at(0) << ": " << error << "\n";
				return 1;
			}
			for (const QString &line : lines)
				out << line << "\n";
			out.flush();
			return 0;
		}
		if (!QetDbFile::setHistory(rest.at(0), format == "historyon", &error)) {
			err << "Cannot change the history of " << rest.at(0) << ": " << error << "\n";
			return 1;
		}
		return 0;
	}

	// --history-diff says what changed between two of a .qetdb's revisions
	// (QETDB-S6-SCOPE.md S6-3)
	if (format == "historydiff") {
		bool from_number = false, to_number = false;
		const qint64 from = rest.value(1).toLongLong(&from_number);
		const qint64 to = rest.value(2).toLongLong(&to_number);
		if (rest.size() < 3 || !from_number || !to_number || !QetDbFile::isDatabasePath(rest.at(0))) {
			err << "Usage: qelectrotech --history-diff <project.qetdb> <revision> <later revision>\n";
			return 2;
		}
		QString error;
		const QStringList lines = QetDbFile::historyDiff(rest.at(0), from, to, &error);
		if (lines.isEmpty()) {
			err << rest.at(0) << ": " << error << "\n";
			return 1;
		}
		for (const QString &line : lines)
			out << line << "\n";
		out.flush();
		return 0;
	}

	// --history-keep keeps only a .qetdb's last saves (QETDB-S6-SCOPE.md S6-6)
	if (format == "historykeep") {
		bool number = false;
		const int saves = rest.value(1).toInt(&number);
		if (rest.size() < 2 || !number || !QetDbFile::isDatabasePath(rest.at(0))) {
			err << "Usage: qelectrotech --history-keep <project.qetdb> <saves, 0 for all>\n";
			return 2;
		}
		QString error;
		if (!QetDbFile::setHistoryLimit(rest.at(0), saves, &error)) {
			err << "Cannot limit the history of " << rest.at(0) << ": " << error << "\n";
			return 1;
		}
		return 0;
	}

	// --qetdb-repair writes a .qetdb with its damaged pages rebuilt from the
	// repair data it keeps, never the file itself (QETDB-S4C-PLAN.md P5).
	// Exit 0: a sound file written; 1: not repairable; 3: another program
	// changed it, written only with --back-to-save
	if (format == "qetdbrepair") {
		const bool back = rest.contains(QStringLiteral("--back-to-save"));
		rest.removeAll(QStringLiteral("--back-to-save"));
		if (rest.size() < 2 || !QetDbFile::isDatabasePath(rest.at(0)) || !QetDbFile::isDatabasePath(rest.at(1))
				|| QFileInfo(rest.at(0)).absoluteFilePath() == QFileInfo(rest.at(1)).absoluteFilePath()) {
			err << "Usage: qelectrotech --qetdb-repair <damaged.qetdb> <repaired.qetdb> [--back-to-save]\n";
			return 2;
		}
		QString error;
		QetDbFile::Repair found;
		const bool ok = QetDbFile::repair(rest.at(0), rest.at(1), &found, &error, back);
		out << "repair data: " << (found.parity ? (found.scanned ? "yes, read from its pages" : "yes") : "none") << "\n";
		if (found.edited) out << "another program changed it since its save\n";
		out << "damaged pages: " << found.bad_pages << "\n";
		if (found.header) out << "header rebuilt\n";
		if (found.parity_damaged) out << "its repair data was damaged: written again\n";
		if (found.beyond) out << "groups beyond repair: " << found.beyond << "\n";
		if (ok)
			out << "written: " << rest.at(1)
				<< (found.back_to_save ? " (back to the saved state)"
									   : found.repaired() ? " (repaired)" : " (nothing to repair)") << "\n";
		out.flush();
		if (ok) return 0;
		err << "Not written: " << error << "\n";
		return found.edited && !back ? 3 : 1;
	}

	// --last-change says which save last changed a symbol, a wire or a
	// folio (QETDB-S6-SCOPE.md S6-5)
	if (format == "lastchange") {
		if (rest.size() < 2 || !QetDbFile::isDatabasePath(rest.at(0))) {
			err << "Usage: qelectrotech --last-change <project.qetdb> <uuid>\n";
			return 2;
		}
		QString error;
		const QString line = QetDbFile::lastChange(rest.at(0), rest.at(1), &error);
		if (!error.isEmpty()) {
			err << rest.at(0) << ": " << error << "\n";
			return 1;
		}
		out << (line.isEmpty() ? QStringLiteral("not changed by a save it keeps") : line) << "\n";
		out.flush();
		return 0;
	}

	// --revision writes a .qetdb as it was at one of its revisions;
	// --export-delivery writes it without its history (QETDB-S6-SCOPE.md)
	if (format == "revision" || format == "delivery") {
		const int needed = format == "revision" ? 3 : 2;
		bool number = true;
		const qint64 ord = format == "revision" ? rest.value(1).toLongLong(&number) : 0;
		if (rest.size() < needed || !number || !QetDbFile::isDatabasePath(rest.at(0))
				|| !QetDbFile::isDatabasePath(rest.at(needed - 1))) {
			err << (format == "revision"
					? "Usage: qelectrotech --revision <project.qetdb> <number> <output.qetdb>\n"
					: "Usage: qelectrotech --export-delivery <project.qetdb> <output.qetdb>\n");
			return 2;
		}
		QString error;
		bool same_digest = false;
		const bool ok = format == "revision"
				? QetDbFile::writeRevision(rest.at(0), ord, rest.at(2), &error, &same_digest)
				: QetDbFile::exportDelivery(rest.at(0), rest.at(1), &error);
		if (!ok) {
			err << "Cannot write " << rest.at(needed - 1) << " from " << rest.at(0) << ": " << error << "\n";
			return 1;
		}
		if (format == "revision")
			out << "Revision " << ord << " written to " << rest.at(2) << "; content digest "
				<< (same_digest ? "the one it was saved with" : "differs from the one it was saved with")
				<< "\n";
		out.flush();
		return 0;
	}

	// --from-database writes a project from the working copy a QElectroTech
	// that ended left behind, which a project cannot be opened from.
	if (format == "fromdb") {
		if (rest.size() < 2) {
			err << "Usage: qelectrotech --from-database <working-copy.sqlite> <output.qet>\n";
			return 2;
		}
		QString error;
		if (!projectDataBase::writeWorkingCopy(rest.at(0), rest.at(1), &error)) {
			err << "Cannot write " << rest.at(1) << " from " << rest.at(0) << ": " << error << "\n";
			return 1;
		}
		return 0;
	}

	// --simulate-reference checks the simulation against analytical solutions
	// and an optional folder of OpenModelica reference runs; no project needed.
	if (format == "simreference")
		return runSimulationReference(rest.value(0), out);

	// --simulate-catalog prints the model catalog (R00): every fluid function, its role and model
	if (format == "simcatalog") {
		out << QString::fromUtf8(SimulationCatalog::json());
		out.flush();
		return 0;
	}

	// --query runs one query on many projects, each opened in turn.
	if (format == "query") {
		if (rest.size() < 2) {
			err << "Usage: qelectrotech --query <SQL> <project>... [--json]\n";
			return 2;
		}
		return queryProjects(rest.at(0), rest.mid(1), json);
	}

	// --compare-revisions opens two projects
	if (format == "comparerevisions")
		return compareRevisions(rest, json);
	// --export-marking takes options with values, read before the
	// project is opened
	if (format == "marking")
		return exportMarking(rest);
	// The THM label print: a job file, the printer's state, sending
	if (format == "thmprn")
		return exportThmJob(rest);
	if (format == "thmstatus")
		return thmStatus(rest, json);
	if (format == "thmsend")
		return thmSend(rest);
	if (format == "thmprinters")
		return thmPrinters(json);
	if (format == "thmload")
		return thmLoad(rest);
	if (format == "thmtransport")
		return thmTransport(rest);

	const QString input = rest.value(0);
	if (input.isEmpty()) {
		err << "Usage: qelectrotech " << flag << " <project.qet> <output>\n";
		return 2;
	}
	if (!QFileInfo::exists(input)) {
		err << "Project not found: " << input << "\n";
		return 2;
	}

	QETProject project(input);
	if (project.state() != QETProject::Ok) {
		err << "Failed to open project: " << input
			<< " (state " << project.state() << ")\n";
		return 1;
	}

	// --simulate runs the pneumatic/hydraulic/electric simulation headless
	if (format == "simulate")
		return simulateProject(project, rest.mid(1));

	if (format == "revisions")
		return listRevisions(project, json);
	if (format == "closerevision" || format == "revisionmarks") {
		if (!ignoreLock) {
			const ProjectLock::Holder holder = ProjectLock::liveHolder(input);
			if (holder.valid) {
				err << "'" << input << "' is open by " << holder.user << " on " << holder.host
					<< ". Not writing it; close it there or pass --ignore-lock.\n";
				return 1;
			}
		}
		if (format == "closerevision")
			return closeRevision(project, rest, !noPdf);
		const QString state = rest.value(1);
		if (state != QLatin1String("on") && state != QLatin1String("off")) {
			err << "Usage: qelectrotech --revision-marks <project> on|off\n";
			return 2;
		}
		project.revisions()->setMarksShown(state == QLatin1String("on"));
		const QETResult saved = project.write();
		if (!saved.isOk()) {
			err << saved.errorMessage() << "\n";
			return 1;
		}
		out << "Revision marks " << state << "\n";
		return 0;
	}
	// --check-project: JSON to stdout, or to an optional output file
	if (format == "checkproject")
		return checkProject(project, rest.value(1));
	if (format == "fetchcommercial")
		return fetchCommercial(project, !purchaseOffline);

	// --info writes JSON to stdout, or to an optional output file.
	if (format == "info")
		return exportInfo(project, rest.value(1));

	const QString output = rest.value(1);
	if (output.isEmpty()) {
		err << "Usage: qelectrotech " << flag
			<< " <project.qet> <output>\n";
		return 2;
	}
	if (format == "pdf")
		return exportPdf(project, output, showTerminals);
	if (format == "dxf")
		return exportDxf(project, output, showTerminals, dxfBlocks, dxfAttributes);
	if (format == "cables" || format == "wires")
		return exportCsv(project, format, output);
	if (format == "bom")
		return exportBom(project, output, includeSlaves, includeJunctions);
	if (format == "purchase")
		return exportPurchase(project, output, purchaseOnline, purchaseErp);
	if (format == "wiring")
		return exportWiring(project, output);
	if (format == "nets")
		return exportNets(project, output);
	if (format == "links")
		return exportLinks(project, output);
	if (format == "tiaexport" || format == "tiacompare") {
		QStringList positional;
		QHash<QString, QString> opts;
		for (int i = 1; i < rest.size(); ++i) {
			const QString &a = rest.at(i);
			if (a.startsWith("--") && i + 1 < rest.size())
				opts.insert(a.mid(2), rest.at(++i));
			else
				positional << a;
		}
		if (format == "tiaexport")
			return exportTiaTags(project, positional.value(0), opts);
		return compareTiaTags(project, positional.value(0), positional.value(1), opts);
	}
	if (format == "devicetypes")
		return exportDeviceTypeCheck(project, output);
	if (format == "devicetypelist" || format == "devicetypebrief")
		return exportDeviceTypeList(project, output, format == "devicetypebrief");
		//The verbs that write a project file leave alone one that someone
		//has open (their lock beside it), unless told to: the read-only
		//verbs above never look at locks
	if ((format == "resave" || format == "settb") && !ignoreLock) {
		const ProjectLock::Holder holder = ProjectLock::liveHolder(output);
		if (holder.valid) {
			err << "'" << output << "' is open by " << holder.user << " on "
				<< holder.host << " (lock file " << ProjectLock::lockPathFor(output)
				<< "). Not writing it; close it there, write to another file, "
				   "or pass --ignore-lock.\n";
			return 1;
		}
	}
	if (format == "resave")
		return resaveProject(project, output);
	if (format == "settb")
		return setTitleBlock(project, output, rest.mid(2));
	return exportImages(project, format, output, showTerminals,
						svgSmallPictures);
}

int exportProjectPdf(QETProject &project, const QString &output)
{
	return exportPdf(project, output, false);
}

} // namespace CLIExport
