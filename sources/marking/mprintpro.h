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
#ifndef MPRINTPRO_H
#define MPRINTPRO_H

#include "markingexport.h"

#include <QHash>
#include <QList>
#include <QString>
#include <QStringList>

/**
	@brief Markers straight to Weidmüller M-Print PRO and its printers
	(PrintJet CONNECT for MultiCards, THM MultiMark for labels and
	continuous material).

	M-Print PRO imports a file onto one material from its command line
	(user manual 2025-01, p. 223, "Importieren oder Drucken einer Datei"):
		MPrintPRO.exe "file" [-p] -ImportFilter:CSV -MatNo:<material number>
	The CSV filter it runs is the script csvfilter.mis M-Print PRO ships:
	';' between fields, '"' around text, Windows text, column 1 the
	project text and column 2 the marker text, no header line and no
	quantity column. So every material gets a file of its own, a line per
	marker, and the program is started once per file; -p prints it
	without showing the window. Sources in docs/beschriftung-export.md.
*/
namespace MPrintPro
{
	enum Printer {
		PrintJetConnect,  ///< UV inkjet, MultiCard frames
		ThmMultiMark      ///< thermal transfer, labels and continuous material
	};

		/// What a material is chosen for: the marking export's kinds,
		/// the strip's own marker apart from its terminals
	enum Slot { Wire, Terminal, Strip, Device, Cable, SlotCount };

	struct Material {
		QString number;  ///< Weidmüller material number
		QString name;    ///< as M-Print PRO's product reference names it
	};

	struct Settings {
		Printer printer = PrintJetConnect;
			/// Material number per slot; empty: the printer's default
		QString materials[SlotCount];
		QString exe;           ///< MPrintPRO.exe; empty: looked for
		bool print = false;    ///< -p: print at once
	};

	QString printerKey(Printer printer);
	bool printerFromKey(const QString &key, Printer *printer);
	QString printerTitle(Printer printer);
	QString slotKey(Slot slot);     ///< Draht, Klemme, Klemmleiste, Geraet, Kabel
	QString slotTitle(Slot slot);
	Slot slotOf(const MarkingExport::Item &item);

		/// The materials offered for a slot on a printer, the default first
	QList<Material> catalog(Printer printer, Slot slot);
	QString materialName(const QString &number);
		/// The settings' material for @p slot, else the printer's default
	QString material(const Settings &settings, Slot slot);
		/// Slot key -> material number, for MarkingExport::write()
	QHash<QString, QString> materialsByKey(const Settings &settings);

	Settings load();
	void save(const Settings &settings);
		/// "Draht=1852490000,Klemme=1609801044" (keys as slotKey() or
		/// wires/terminals/strips/devices/cables)
	bool parseMaterials(const QString &text, Settings *settings, QString *error);

		/// MPrintPRO.exe: the settings' path when it exists, else the
		/// usual install folders; empty when none is found
	QString findExe(const Settings &settings);

	struct Job {
		QString file;
		QString material;
		QString material_name;
		int markers = 0;
	};
		/// One CSV per material in @p dir, named after @p base
	QList<Job> writeJobs(const QList<MarkingExport::Item> &items,
						 const Settings &settings, const QString &dir,
						 const QString &base, QString *error = nullptr);
		/// The bytes of one job's file: no header, one line per marker
	QByteArray jobFile(const QList<MarkingExport::Item> &items);
	QStringList arguments(const Job &job, bool print);
		/// Start M-Print PRO once per job
	bool open(const QList<Job> &jobs, const Settings &settings,
			  QString *error = nullptr);
}

#endif // MPRINTPRO_H
