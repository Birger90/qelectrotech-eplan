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
#ifndef TERMINALARTICLECATALOG_H
#define TERMINALARTICLECATALOG_H

#include <QString>
#include <QStringList>
#include <QVector>

/**
	@brief Terminal blocks one can order: manufacturer, order number and
	what is needed to draw and list them (cross-section, width, clamp,
	colour).

	A small catalogue of common rail-mounted terminals is built in. More
	come from CSV files in the folder "terminal-catalogs" of the data
	directory (one file per manufacturer or per company list), so a team
	shares its own choice through the data directory on the server:

	manufacturer;order_number;designation;kind;section;width;clamp;color;conductors;levels
	WAGO;2002-1201;TOPJOB S feed-through terminal;feed-through;2.5;5.2;push-in;Gray;2;1

	kind: feed-through, pe, double-deck, fuse, disconnect
	clamp: push-in, spring, screw
	color: a colour name of the terminal strip palette (Gray, Blue,
	Green/Yellow ...)
*/
namespace TerminalArticleCatalog
{
	enum class Kind { FeedThrough, ProtectiveEarth, DoubleDeck, Fuse, Disconnect };
	enum class Clamp { PushIn, Spring, Screw };

	struct Article
	{
		QString manufacturer;
		QString order_number;
		QString designation;
		Kind kind = Kind::FeedThrough;
		double section = 0;		///< mm²
		double width = 0;		///< mm
		Clamp clamp = Clamp::PushIn;
		QString color;			///< palette name, "Gray"
		int conductors = 2;		///< connection points
		int levels = 1;
		QString source;			///< "built-in", the catalogue file name or "online"
		QString type_number;	///< manufacturer's type, "PT 2,5"
		QString dataportal_id;	///< id of the part in the EPLAN Data Portal
		QString picture_url;	///< product picture, known after a Data Portal search

			///"WAGO 2002-1201", what the article field of a terminal holds
		QString articleText() const {
			return manufacturer + QLatin1Char(' ') + order_number;
		}
		bool isValid() const { return !order_number.isEmpty(); }
	};

		///Every article: the built-in ones, then those of the catalogue files
	QVector<Article> articles();
		///Read the catalogue files again (after the user changed one)
	void reload();
		///The article written as "Manufacturer order-number", or an invalid one
	Article find(const QString &article_text);
		///The folder of the catalogue files; created on demand with a template
	QString catalogDirectory(bool create = false);

		///An article read from the texts of a Data Portal part (designation
		///and description); what they do not say stays empty
	Article fromDataPortal(const QString &manufacturer, const QString &order_number,
						   const QString &type_number, const QString &designation,
						   const QString &description, const QString &dataportal_id);
		///Keep an article found online in "Data Portal.csv" of the catalogue folder
	bool remember(const Article &article);
		///The Data Portal short name of a manufacturer ("Phoenix Contact" -> "PXC")
	QString dataPortalShortName(const QString &manufacturer);
		///The EPLAN product group of terminal blocks in the Data Portal
	inline QString dataPortalTerminalGroup() { return QStringLiteral("200040005"); }

	QString kindName(Kind kind);
	QString clampName(Clamp clamp);
		///The colour as the terminal strip stores it ("#9e9e9e/#...")
	QString colorSpec(const Article &article);
		///The entry of the drawing catalogue (TerminalCatalog) closest to the article
	QString drawingCatalogEntry(const Article &article);
		///The names of the levels as the type of a terminal of several
		///levels gives them, lowest level first: "PTI 2,5-PE/L/NT" ->
		///{"PE", "L", "N"}; empty when the type names none
	QStringList levelNames(const Article &article);
		///The level (0 = lowest) of a terminal of several levels which is
		///protective earth, -1 when it has none: an installation terminal
		///"L/N" carries no PE, a "PE/L/N" one does
	int peLevel(const Article &article);
}

#endif // TERMINALARTICLECATALOG_H
