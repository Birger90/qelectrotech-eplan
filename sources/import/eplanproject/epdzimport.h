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
#ifndef EPDZIMPORT_H
#define EPDZIMPORT_H

#include <QString>
#include <QStringList>

/**
	@brief Reads an EPLAN export archive (.epdz, as EPLAN eView and the
	"export for viewing" write it) into a QElectroTech project: the part
	of "File > Import EPLAN project" for .epdz files, beside the native
	.zw1 project.

	An .epdz is a 7-Zip archive with a SQLite manifest (manifest.db), the
	pages as SVG, AutomationML and the 3D installation spaces (.e3d). It
	holds drawings, not an editable schematic; so the import gives:
	- one folio per page, in the order of the manifest, with its title,
	  page number and structure (= plant, + location) in the title block
	  and the page drawn as a picture filling the folio;
	- beside the .qet: <name>_Geraete.csv (the functions: designation and
	  their properties 20001, 20100, 20193 of the manifest),
	  <name>_Verbindungen.csv (source, target, potential, colour, section,
	  length of every connection) and the folder <name>_3D with the .e3d
	  installation spaces, which the 3D view reads.

	The table layout follows the format notes of ecad-view
	(https://github.com/Canary-ECAD/ecad-view, docs/formats.md and
	packages/e3d-core/src/manifest.ts, MIT); the code is our own.
*/
namespace EpdzImport
{
	struct Result {
		bool ok = false;
		QString error;
		QStringList notes;     ///< what was read, for the log of the dialog
		int pages = 0, functions = 0, connections = 0, spaces = 0;
	};
	Result run(const QString &epdz, const QString &target_qet);
}

#endif // EPDZIMPORT_H
