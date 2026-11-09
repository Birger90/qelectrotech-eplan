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
#ifndef PARTFETCHER_H
#define PARTFETCHER_H

#include <QHash>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QStringList>

class QNetworkAccessManager;

/// one missing part of a list
struct FetchItem
{
	QString maker;         ///< as in the list: SIE, RIT, or a name
	QString order;         ///< order number as in the list
	QString description;
};

/// where to and what
struct FetchOptions
{
	QString collection_dir;    ///< company collection, empty: QETApp's
	QString raw_dir;           ///< raw files, empty: <data>/herstellerdaten
	QString model_dir;         ///< STEP models, empty: <data>/3D
	QStringList link_files;    ///< extra SiePortal link files
	QStringList makers;        ///< only these makers (codes or names), empty: all
	int limit = 0;             ///< at most so many parts, 0: all
	int pause_ms = 500;        ///< between two downloads of a server
	bool dry_run = false;      ///< only say what would be done
	bool force = false;        ///< import again what is there
	bool use_dataportal = true;
};

/// what came of one part
struct FetchResult
{
	QString maker;             ///< folder name, e.g. Siemens
	QString order;
	QString status;            ///< done, partial, present, missing, failed, planned
	QStringList notes;
	QStringList written;       ///< elements and models written
	QString step_file;
	QJsonObject toJson() const;
};

/**
	@brief Fetches the manufacturer data of missing parts and takes it into
	the own data: EPLAN macro (EDZ) and DXF into the company collection
	(EplanPartImporter), STEP into the 3D folder as <maker>/<order>.stp,
	the files as loaded into <raw>/<maker>/<order>/.

	Sources, in this order, only official interfaces and public files:
	- the files already in the raw folder;
	- Siemens: the links of static.siemens.com/mimes/imagedb_edz|dxf|stp of
	  the SiePortal product page, from the link files
	  (<raw>/Siemens/sieportal_links.txt and FetchOptions::link_files, lines
	  "<order number> <url>"), made in the browser by
	  tools/herstellerportale/sieportal_links.js;
	- the EPLAN Data Portal API: the category (product group) without a
	  login, the DXF package with the Personal Access Token.
	3Dfindit has no API: parts without a STEP model are reported with the
	requests left of the day.

	Network calls are synchronous (a local event loop), as in the script
	API; progress() reports every part.
*/
class PartFetcher : public QObject
{
	Q_OBJECT

	public:
		explicit PartFetcher(QObject *parent = nullptr);
		~PartFetcher() override;

		/// the parts of a list: a list of missing parts by manufacturer
		/// ("=== SIE (n Artikel) ===" and "  <order>  DXF ..." lines), or
		/// plain lines "<maker>;<order>" or "<order>" (maker of the last header)
		static QList<FetchItem> readList(const QString &path, QString *error = nullptr);
		/// the SiePortal links of a link file: order key -> urls
		static QHash<QString, QStringList> readLinks(const QString &path);
		/// 3RT2015-1BB41 -> 3RT20151BB41
		static QString orderKey(const QString &order);
		/// SIE -> Siemens, RIT -> Rittal
		static QString makerName(const QString &maker);
		/// a url QElectroTech may load for Siemens: a public static file
		static bool isSiemensFileUrl(const QString &url);
		/// the default link file of Siemens
		static QString siemensLinkFile(const QString &raw_dir = QString());

		QList<FetchResult> run(const QList<FetchItem> &items, const FetchOptions &options);
		FetchResult fetch(const FetchItem &item, const FetchOptions &options);
		void cancel() { m_cancel = true; }

		static QJsonObject toJson(const QList<FetchResult> &results);

	signals:
		void progress(int done, int total, const QString &line);

	private:
		bool download(const QString &url, const QString &file, int pause_ms, QString *error);
		bool dataPortalLookup(const QString &order, const QString &maker, QString *part_id,
				      QString *category, QString *maker_long, QString *error);
		bool dataPortalPackage(const QString &part_id, const QString &file, QString *error);
		void loadLinks(const FetchOptions &options);
		static QString existingSymbolFolder(const QString &collection, const QString &maker, const QString &order);

		QNetworkAccessManager *m_manager = nullptr;
		QHash<QString, QStringList> m_links;
		QString m_links_key;
		QHash<QString, qint64> m_last_request;	///< host -> time of the last download (ms), for the pause
		bool m_cancel = false;
};

#endif // PARTFETCHER_H
