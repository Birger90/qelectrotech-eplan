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
#ifndef CLI_EXPORT_H
#define CLI_EXPORT_H

#include <QStringList>

class QETProject;

/**
	@brief Headless command-line export.

	Implements the long-requested batch/headless export
	(qelectrotech.org bugtracker #171, GitHub #309): render a project's
	diagrams to files without opening the GUI.

	Detected and handled in main() before the GUI is created.
*/
namespace CLIExport {

	/**
		@brief True if @p args request a CLI export
		(i.e. contain one of the export options).
	*/
	bool isExportRequest(const QStringList &args);

	/**
		@brief Run the CLI export described by @p args.
		@return process exit code (0 on success).

		Usage:
		  qelectrotech --export-pdf     <project.qet> <output.pdf> [--show-terminals]
		  qelectrotech --export-png     <project.qet> <output_dir> [--show-terminals]
		  qelectrotech --export-svg     <project.qet> <output_dir> [--show-terminals] [--svg-small-pictures]
		  qelectrotech --export-dxf     <project.qet> <output_dir> [--show-terminals] [--dxf-blocks] [--dxf-attributes]
		  qelectrotech --export-cables  <project.qet> <output.csv>
		  qelectrotech --export-wires   <project.qet> <output.csv>
		  qelectrotech --export-bom     <project.qet> <output.csv> [--no-slaves] [--no-junctions]
		  qelectrotech --export-purchase <project.qet> <output.xlsx|output.csv> [--online] [--erp]
		  qelectrotech --fetch-commercial <project.qet> [--offline]
		  qelectrotech --export-wiring  <project.qet> <output.csv>
		  qelectrotech --export-marking <project.qet> <output.csv> [--format phoenix|weidmueller|wago|neutral] [--kinds wires,terminals,devices,cables] [--folios N[-M]] [--plant P] [--location L] [--short] [--one-end]
		  qelectrotech --export-nets    <project.qet> <output.json>
		  qelectrotech --export-links   <project.qet> <output.csv>
		  qelectrotech --export-tia-tags  <project.qet> <tags.xlsx | tags.xml> [--tia-table NAME] [--tia-culture de-DE]
		  qelectrotech --compare-tia-tags <project.qet> <tags.xlsx | tags.xml> [report.csv | -] [--apply output.qet]
		  qelectrotech --check-device-types <project.qet> <output.csv>
		  qelectrotech --check-project  <project.qet> [output.json]   (exit 3 when errors are found)
		  qelectrotech --export-device-types <project.qet> <output.csv>
		  qelectrotech --export-device-types-brief <project.qet> <output.csv>
		  qelectrotech --info           <project.qet> [output.json]
		  qelectrotech --check-elements <element.elmt | directory> [--grid]
		  qelectrotech --import-eplan   <package.zip | folder | file.dxf> <collection_dir> [--category C] [--manufacturer M --type T] [--edz part.edz]
		  qelectrotech --fetch-parts    <list.txt> [--collection DIR] [--raw-dir DIR] [--3d-dir DIR] [--links FILE] [--manufacturer SIE,RIT] [--limit N] [--dry-run] [--force] [--report FILE] [--json]
		  qelectrotech --portal-status  [--json]
		  qelectrotech --portal-login   <portal> [--user NAME]   (password or token on stdin)
		  qelectrotech --portal-logout  <portal>
		  qelectrotech --portal-request 3dfindit <order number>
		  qelectrotech --resave         <project.qet> <output.qet>
		  qelectrotech --recover        <damaged.qetz> <output.qet>
		  qelectrotech --lock-status    <project> [--json]   (who has it open, left-over lock, OneDrive conflict copies)
		  qelectrotech --set-titleblock <project.qet> <output.qet> key=value...
		  qelectrotech --query          <SQL> <project>... [--json]

		PDF: one multi-page document (one diagram per page).
		PNG/SVG/DXF: one file per diagram, named <output_dir>/<NN>_<title>.<ext>.
		DXF: the same file the export dialog writes with its default options
		      (the export settings of the preferences).
		--show-terminals: also paint terminal markers (red stroke + blue
		      docking dot) and terminal names, as the interactive editor
		      does; off by default, matching the GUI export dialog's
		      default. For DXF, draws the terminal markers. Has no effect on
		      the list export modes.
		--dxf-blocks: write each symbol as a DXF block, placed with one
		      INSERT per use, so a CAD program selects it as one object.
		--dxf-attributes: as --dxf-blocks, and a symbol's texts (its label,
		      function...) are attributes of its INSERT. LibreCAD does not
		      show attributes.
		--svg-small-pictures: in an SVG, each picture at most four times the
		      size it is drawn at instead of its full resolution: a much
		      smaller file, still sharp when printed.
		cables: wiring list (one row per conductor) as CSV.
		wires: list of distinct wire numbers as CSV.
		bom: bill of materials (one row per element) as CSV.
		--no-slaves: leave the contact blocks (slave elements) out of the
		      bill of materials.
		--no-junctions: leave the junctions out of the bill of materials:
		      terminal-type elements with no label, designation,
		      manufacturer or manufacturer reference.
		marking: wire, terminal, device and cable markers as a file for the
		      marking software of Phoenix Contact (MARKING system / PROJECT
		      complete), Weidmüller (M-Print PRO) or WAGO (Smart Script), or a
		      neutral CSV; the columns of each are in docs/beschriftung-export.md.
		      --kinds chooses what gets a marker (default all), --folios a
		      folio or a range, --plant/--location a part of the structure,
		      --short writes -K1 instead of =plant+location-K1, --one-end one
		      marker per wire and cable instead of one per end.
		wiring: from-to wiring list (one row per conductor) as CSV, read from
		        the project database. Same logical list as `cables`, which
		        reads the document XML instead; the two are meant to agree,
		        so diffing them checks that the database still describes the
		        project.
		nets: electrical nets (connected-terminal groups) as JSON.
		links: element cross-references (coil/contact) as CSV, with
		       unresolved links flagged.
		export-tia-tags: the PLC channels (IO tables of the PLC modules) as
		      PLC tag table for Siemens TIA Portal: .xlsx in the format of
		      the tag table editor's Import (sheets PLCTags and
		      TagTableProperties), .xml as SimaticML for TIA Portal
		      Openness. --tia-table names the tag table (default QET_EA),
		      --tia-culture the language of the comments in the .xml.
		compare-tia-tags: read a TIA tag table (.xlsx exported by TIA
		      Portal, or SimaticML .xml) and list per channel whether
		      address, name and comment agree, as CSV: same, name,
		      comment, name+comment, address, missing, only_tia. With
		      --apply the tag names become the symbolic addresses and the
		      tag comments the function texts of the channels with the
		      same address, saved as output.qet.
		info: structural project summary as JSON (stdout, or a file) —
		      per-page element / conductor counts and unconnected terminals.
		check-elements: validate .elmt file(s) against the element schema.
		import-eplan: build elements from manufacturer EPLAN DXF data (an
		      EPLAN Data Portal DXF package, or a single DXF with
		      --manufacturer and --type) into
		      <collection_dir>/<manufacturer>/<category>/<type>/.
		--grid: check-elements also warns about a symbol that does not line
		      up with the folio grid: its insertion point on no connection
		      point, its frame (hotspot, width, height) or a connection
		      point off the grid of 10.
		resave: load and rewrite the project XML (round-trip integrity).
		--ignore-lock: resave and set-titleblock leave a file alone when a lock
		      file beside it (.~<name>.lock, made by a QElectroTech that has
		      the project open) says someone else has it; this writes it
		      anyway. The verbs that only read a project never look at locks,
		      and none of them takes one. A lock made on this machine by a
		      process that is gone is stale and does not count.
		query: run one read-only SELECT on the project database of each
		      project in turn (.qet, .qetz or .qetdb) and print every row,
		      the project it came from first, as CSV (RFC 4180, a header
		      row) or, with --json, as a JSON array of objects. The views
		      qet_folios, qet_devices, qet_wires, qet_links, qet_cables,
		      qet_device_list, qet_terminals, qet_plc_io, qet_cable_cores and
		      qet_locations keep their names and columns from one version to
		      the next. A project that cannot be opened or queried is
		      reported on stderr, the others still run, and the exit code
		      is 1. A .qetdb is read itself, without opening the project:
		      seconds for hundreds.
		      E.g. --query "SELECT label, folio FROM qet_devices
		      WHERE article = 'LC1D09'" *.qet
		set-titleblock: stamp title-block fields onto every folio, then save.
		      Keys: title, author, date (or date=today), plant, location,
		      revision, version, filename; any other key becomes a custom
		      field.  E.g. --set-titleblock in.qet out.qet revision=B date=today
		Revisions (revision/projectrevisions.h, EPLAN's revision management):
		  --close-revision <project> <index> [text...] [--no-pdf]
		      close a revision: index in every title block, the row in the
		      revision table (%{rev1_index}, %{rev1_date}, %{rev1_text},
		      %{rev1_author}, newest first), a PDF with the changes since
		      the last revision marked and a snapshot in
		      <name>_Revisionen/, then the project saved in place.
		  --revisions <project>  the revision table and what changed since
		      the last revision.
		  --revision-marks <project> on|off  show the marks or not (saved).
		  --compare-revisions <older> <newer> [<output.pdf>] [--json]
		      what changed from one state to another, one line each; with
		      an output, a PDF of the newer with the changes marked.
		      <older> is a project file or a revision index of <newer>.
	*/
	int run(const QStringList &args);

		/// Write every folio of @p project to the PDF @p output, as
		/// --export-pdf does. @return 0 on success
	int exportProjectPdf(QETProject &project, const QString &output);

}

#endif // CLI_EXPORT_H
