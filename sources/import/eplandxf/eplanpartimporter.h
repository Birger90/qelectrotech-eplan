/*
	Copyright 2006-2026 The QElectroTech Team
	This file is part of QElectroTech.

	QElectroTech is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	(at your option) any later version.

	QElectroTech is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with QElectroTech. If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef EPLANPARTIMPORTER_H
#define EPLANPARTIMPORTER_H

#include <QMap>
#include <QString>
#include <QStringList>

/**
	@brief Imports manufacturer EPLAN DXF data as QET elements.

	Two sources:
	- an EPLAN Data Portal DXF package (MAC_<part>.zip or its unpacked
	  folder): commercialdata.csv with the part data and one DXF per
	  variant and representation; every variant becomes an element;
	- a single EPLAN DXF, e.g. a manufacturer download; a drawing with
	  black-box frames (one per part of a multi-part device) gives one
	  element per frame. Part data may come from the matching .edz.

	Elements are written to <collection>/<manufacturer>/<category>/<type>/,
	each folder with a qet_directory file. UI-decoupled like EdzImporter.
*/
class EplanPartImporter
{
	public:
		bool importPackage(const QString &path, const QString &category,
				   const QString &collection_dir);
		bool importDxf(const QString &dxf_path, const QString &edz_path,
			       const QString &manufacturer, const QString &category,
			       const QString &type, const QString &collection_dir);
		/// an EPLAN part (.edz) as a generic element, in the same folder layout
		bool importEdz(const QString &edz_path, const QString &manufacturer,
			       const QString &category, const QString &type,
			       const QString &collection_dir);

		/// display name of a manufacturer ("SIE" -> "Siemens")
		static QString manufacturerName(const QString &name);

		/// part data (commercialdata.csv columns) of a package, for the dialog
		static QMap<QString, QString> packageInfo(const QString &path,
							  QString *error = nullptr,
							  const QString &macro = QString());

		QStringList writtenPaths() const { return m_written; }
		QStringList messages() const { return m_messages; }
		QString errorString() const { return m_error; }

	private:
		QStringList m_written;
		QStringList m_messages;
		QString m_error;
};

#endif // EPLANPARTIMPORTER_H
