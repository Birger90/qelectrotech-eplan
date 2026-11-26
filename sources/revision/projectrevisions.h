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
#ifndef PROJECTREVISIONS_H
#define PROJECTREVISIONS_H

#include <QHash>
#include <QList>
#include <QObject>
#include <QPolygonF>
#include <QRectF>
#include <QStringList>

class Diagram;
class QDomElement;
class QETProject;
class QPainter;

/**
	A simple revision management after EPLAN's model (eplan-wissen,
	revision.md): closing a revision stamps its index, date and text in the
	title blocks, keeps a snapshot of the project beside it and a
	fingerprint of every drawn item in the project itself; what differs
	from that fingerprint is marked on the folios (a revision cloud, a
	stroke along a wire) and listed.

	Items are matched by uuid, then -- for plans built again with new
	uuids -- by EPLAN's "alternative identification": a symbol by its
	folio, type and label, a wire by the symbols and terminals it joins.
*/
namespace QetRevision {

	/// One drawn item as a revision remembers it
	struct Item {
		QChar kind;           ///< e symbol, c wire, t text, s shape, i image
		QString uuid;
		QString folio;        ///< uuid of its folio
		int folio_index = -1;
		QString key;          ///< identity without uuid (alternative identification)
		QByteArray hash;      ///< what it looks like, without uuids
		QByteArray ends;      ///< what it is joined or linked to, by uuid
		QRectF rect;          ///< where it is drawn
		QPolygonF path;       ///< a wire's path
		QString name;         ///< how a person names it: -K1, X1:1 - K1:A1, a text
	};

	/// Every drawn item of a project or folio, by uuid
	struct Fingerprint {
		QHash<QString, Item> items;
		QStringList folios;              ///< folio uuids, in order
		QHash<QString, QString> folio_names; ///< folio uuid -> "3 - Title"
		QHash<QString, QByteArray> folio_hashes; ///< title block, without index and date
		bool isEmpty() const { return folios.isEmpty() && items.isEmpty(); }
	};

	/// One difference between two fingerprints
	struct Change {
		enum Kind { Added, Removed, Changed };
		Kind kind = Changed;
		Item before;	///< for Removed and Changed
		Item now;		///< for Added and Changed
		const Item &item() const { return kind == Removed ? before : now; }
	};

	Fingerprint fingerprint(const QDomElement &diagram, int index, Diagram *live = nullptr);
	Fingerprint fingerprint(QETProject *project);
	Fingerprint fingerprintOfFile(const QString &path, QString *error = nullptr);
	QList<Change> compare(const Fingerprint &before, const Fingerprint &now);

	QByteArray encode(const Fingerprint &fingerprint);
	Fingerprint decode(const QByteArray &data);

	QString kindName(QChar kind);
	QString changeName(Change::Kind kind);
	/// "Blatt 3: Symbol -K1 geändert", one per change, sorted by folio
	QStringList describe(const QList<Change> &changes, const Fingerprint &now,
						 const Fingerprint &before);
	/// The index after @p index: A -> B, Z -> AA, 3 -> 4, 03 -> 04, "" -> A
	QString nextIndex(const QString &index);
}

/**
	The revisions of one project. Their table is QET's revision list
	(QETProject::revisions(), Project properties > Revisions); this class
	adds closing a revision, the fingerprint of the last one, whether its
	changes are marked, and each revision's snapshot and PDF -- kept in an
	<eplan_revisions> block of the project, which every format keeps as it
	is (foreign blocks).
*/
class ProjectRevisions : public QObject
{
	Q_OBJECT
	public:
		struct Entry {
			QString index, date, author, text, snapshot, pdf;
		};

		explicit ProjectRevisions(QETProject *project);

		QList<Entry> entries();
		bool hasBaseline();
		QetRevision::Fingerprint baseline();
		QString lastIndex();
		QString proposedIndex();

		bool marksShown();
		void setMarksShown(bool on);

			/// What differs from the last revision, the whole project
		QList<QetRevision::Change> changes();
			/// What differs on @p diagram, kept until the next edit
		QList<QetRevision::Change> changesOn(Diagram *diagram);

			/// Draw the marks of @p diagram's changes
		void paint(Diagram *diagram, QPainter *painter);

		struct CloseResult {
			bool ok = false;
			QString error, snapshot, pdf;
			int changes = 0;
		};
			/// Close a revision: title blocks, PDF of the changes, snapshot,
			/// new fingerprint, and the project saved
		CloseResult closeRevision(const QString &index, const QString &text,
								  bool write_pdf = true, bool save = true);

			/// The folder the snapshots and PDFs go to
		QString folder() const;
			/// Absolute path of an entry's snapshot
		QString snapshotPath(const Entry &entry) const;

			/// Compare against @p before instead of the last revision until
			/// endComparison(), marks labelled @p label (for a PDF of two states)
		void beginComparison(const QetRevision::Fingerprint &before, const QString &label);
		void endComparison();

			/// Write a PDF of the project, every folio, with the marks it shows
		static bool exportPdf(QETProject *project, const QString &path, QString *error);

			/// Read the block again (after the project was read)
		void reload();
			/// QET's revision list changed (QETProject::setRevisions)
		void listChanged();

	signals:
		void revisionsChanged();

	private:
		void load();
		void save();
		void invalidate();
		void repaint();
		QString markLabel();
		void updateTitleBlockTable(bool force);

		QETProject *m_project;
			/// What closing a revision left beside the project, by index
		struct Files { QString snapshot, pdf; };
		QHash<QString, Files> m_files;
		bool m_updating = false;
		QetRevision::Fingerprint m_baseline;
		bool m_marks = false;
		bool m_loaded = false;
			// A comparison of two states
		bool m_comparing = false;
		QetRevision::Fingerprint m_compare_baseline;
		QString m_compare_label;
		bool m_compare_marks = false;
		QHash<QString, QList<QetRevision::Change>> m_cache; // folio uuid
		bool m_computing = false;
};

#endif // PROJECTREVISIONS_H
