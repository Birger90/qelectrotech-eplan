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
#ifndef PANELIMPORT_H
#define PANELIMPORT_H

#include "enclosuresize.h"
#include "paneldata.h"
#include "../import/eplandxf/dataportalclient.h"

#include <QDialog>
#include <QMap>
#include <QStringList>
#include <QVector>
#include <QWidget>

class QLabel;
class QLineEdit;
class QPushButton;
class QTreeWidget;

/**
	@brief Mounting plates from outside: a DXF of a plate (one's own, or
	the panel layout view of a manufacturer's plate from the EPLAN Data
	Portal) read as a contour in millimetres, and the folder of plate
	templates ("panel-templates" in the documents folder) the "insert
	mounting surface" dialog offers.
*/
class PanelImport
{
	public:
			/// lines, arcs and circles of a DXF (blocks resolved, texts
			/// left out), moved so the lower left corner is 0,0
		static PanelOutline readDxf(const QByteArray &dxf, QString *error = nullptr);
		static PanelOutline readDxfFile(const QString &path, QString *error = nullptr);

			/// a Data Portal DXF package: the panel layout view with the
			/// most holes (the front of the plate), as DXF text
		static QByteArray plateViewFromPackage(const QByteArray &zip, QString *error = nullptr);

			/// all panel layout views of a package by their letter (A front,
			/// B mounting plate of an enclosure, C/D top/bottom, E/F sides)
		static QMap<QString, QByteArray> viewsFromPackage(const QByteArray &zip);

			/// an enclosure from the Data Portal: its views and sizes
		struct Enclosure
		{
			QString folder, name, description;
			double width = 0, height = 0, depth = 0;   ///< from "BHT: 800x1400x500 mm"
			PanelOutline front, plate;
			bool isNull() const {return front.isEmpty();}
		};
		static QString enclosuresDirectory();
		static QStringList enclosureFolders();
			/// the stored enclosures of at least @p size, the closest fit
			/// first; all of them, by name, when @p size is null
		static QStringList enclosureFolders(const EnclosureSize &size);
			/// the sizes of a stored enclosure from its description, without
			/// reading its drawings
		static EnclosureSize enclosureSize(const QString &folder);
			/// the mounting plate views (B.dxf) of the stored enclosures:
			/// plate templates that came with an enclosure
		static QStringList enclosurePlateFiles();
		static Enclosure readEnclosure(const QString &folder);
		static QString saveEnclosure(const QString &name, const QString &description,
									 const QMap<QString, QByteArray> &views, QString *error = nullptr);

			/// the panel layout view (front, A) of a device from the EPLAN
			/// Data Portal, found by its order number: read from the cache
			/// (panel-templates/devices) or downloaded with the user's token.
			/// Blocks while it downloads (a few seconds); empty when the
			/// portal has none or only a raster picture.
		static PanelOutline fetchDeviceView(const QString &order_number, const QString &manufacturer,
											QString *error = nullptr);
		static QString devicesDirectory();
			/// the panel layout view of a part from the element collections
			/// (company, then user): an element of that order number whose
			/// name says "Aufbau" (layout). Imported views are 2.5 px per mm
			/// (EPLAN layout macros); a view with width_mm/height_mm is
			/// scaled to them. Empty when there is none.
		static PanelOutline collectionDeviceView(const QString &order_number, const QString &manufacturer);
			/// the device itself out of a manufacturer's drawing that holds
			/// more (a drawing frame, side views, dimensions): the connected
			/// part with the most detail and all that lies inside it
		static PanelOutline frontView(const PanelOutline &drawing);
		static QByteArray deviceViewFromPackage(const QByteArray &zip);

		static QString templatesDirectory();
		static QStringList templateFiles();
		static QString templateName(const QString &file);
			/// store a DXF as template; @return its file
		static QString saveTemplate(const QString &name, const QByteArray &dxf, QString *error = nullptr);
};

/**
	@brief Three fields for the width, height and depth of an enclosure in
	millimetres; an empty field stands for "any".
*/
class EnclosureSizeFields : public QWidget
{
	Q_OBJECT

	public:
		explicit EnclosureSizeFields(QWidget *parent = nullptr);
		EnclosureSize size() const;
			/// does not emit changed()
		void setSize(const EnclosureSize &size);

	signals:
		void changed();

	protected:
		void keyPressEvent(QKeyEvent *event) override;

	private:
		QLineEdit *m_width = nullptr;
		QLineEdit *m_height = nullptr;
		QLineEdit *m_depth = nullptr;
};

/**
	@brief Search the EPLAN Data Portal for mounting plates (Rittal by
	default), show them with their sizes, and store the chosen one's
	front view as a plate template.

	For enclosures it lists the Data Portal's product group "enclosure"
	of the manufacturer instead of the hits of a text search, with the
	sizes read from the descriptions, and narrows that list at once by
	the width, height and depth asked for and by words: the Data Portal
	itself cannot search by size.
*/
class PanelTemplateDialog : public QDialog
{
	Q_OBJECT

	public:
		explicit PanelTemplateDialog(QWidget *parent = nullptr, bool enclosure = false,
									 const EnclosureSize &size = EnclosureSize());
			/// the sizes asked for when the dialog closed
		EnclosureSize sizeFilter() const;
			/// the stored plate template (DXF), or the enclosure folder
		QString templateFile() const {return m_file;}

	private:
		void runSearch();
		void loadGroup(int offset);
		void applyFilter();
		void showParts(const QVector<DataPortalPart> &parts, int total, int offset);
		void download();
		void downloaded(const QByteArray &data, const QString &file_name);
		void failed(const QString &message);

		DataPortalClient *m_client = nullptr;
		QLineEdit *m_search = nullptr;
		QLineEdit *m_manufacturer = nullptr;
		QTreeWidget *m_list = nullptr;
		QLabel *m_status = nullptr;
		QPushButton *m_take = nullptr;
		QString m_file;
		QString m_pending_name;
		QString m_pending_description;
		bool m_enclosure = false;
		EnclosureSizeFields *m_size = nullptr;
		QVector<DataPortalPart> m_parts;   ///< the product group, as far as loaded
		QString m_group_key;               ///< manufacturer and language of m_parts
};

#endif // PANELIMPORT_H
