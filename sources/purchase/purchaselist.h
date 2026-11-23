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
#ifndef PURCHASELIST_H
#define PURCHASELIST_H

#include <QCoreApplication>
#include <QHash>
#include <QJsonObject>
#include <QList>
#include <QString>
#include <QStringList>

class QETProject;

/**
	@brief The purchase list: the parts list of a project the way the
	purchasing department and an ERP system read it.

	One line per article (manufacturer, part number and unit the same),
	however many elements use it, as BomExport::groupedQuery() groups the
	parts list; and one line per device that has no article yet, marked
	"article open", so nothing is left out of the order.

	The commercial data of an article come from three places, the later
	overriding the earlier:
	 1. the elements themselves: unit price and supplier fields;
	 2. the article data cache (artikeldaten.csv in the purchase folder),
	    filled from the EPLAN Data Portal and the EDZ packages of the
	    company data folder, see PurchaseFetch;
	 3. the company price list (preisliste.csv in the same folder),
	    which the company keeps itself: its own purchase prices, delivery
	    times and suppliers.
	Nothing is guessed: a value no source gives stays empty.

	The purchase folder is "einkauf" in the company data folder
	(the folder above the company collection), or the folder named by the
	environment variable QET_PURCHASE_DIR.
*/
class PurchaseList
{
	Q_DECLARE_TR_FUNCTIONS(PurchaseList)

	public:
			/// One line of the purchase list
		struct Item
		{
			QString supplier;        ///< price list, else the elements' supplier field
			QString manufacturer;
			QString part_number;     ///< manufacturer_reference of the elements
			QString name;            ///< description, else designation
			QStringList references;  ///< labels of the elements using it
			QString unit;            ///< quantity unit ("Stk", "m"...)
			double quantity = 0;     ///< what the project needs
			double packaging = 0;    ///< packaging unit (VPE); 0: unknown
			double price = -1;       ///< price per price_unit; < 0: unknown
			double price_unit = 1;   ///< the price is for this many units
			QString currency;        ///< of the price, empty when there is none
			QString delivery_time;   ///< as written in the price list
			QString gtin;            ///< EAN / GTIN
			QString lifecycle;       ///< Data Portal lifecycle ("Active"...)
			QString price_source;    ///< where the price comes from
			bool open = false;       ///< a device without an article

				/// quantity rounded up to whole packaging units
			double orderQuantity() const;
				/// price of the order quantity; < 0 when there is no price
			double total() const;
				/// supplier, else manufacturer: what the list is grouped by
			QString groupName() const;
		};

			/// One article of the cache or of the price list
		struct ArticleData
		{
			QString manufacturer;
			QString part_number;
			QString gtin;
			QString gtin_type;
			double packaging = 0;
			QString unit;
			double price = -1;
			double price_unit = 0;
			QString currency;
			QString delivery_time;
			QString supplier;
			QString lifecycle;
			QString source;          ///< "dataportal", "edz", "dataportal: not found"...
			QString date;            ///< ISO date of the lookup
		};

		struct Summary
		{
			int articles = 0;        ///< lines with an article
			int open = 0;            ///< devices without an article
			int priced = 0;          ///< articles with a price
			int with_gtin = 0;
			double total = 0;        ///< sum of the known totals
			QStringList currencies;
			QJsonObject toJson() const;
		};

			/// folder of the price list and the cache
		static QString purchaseDir();
		static QString priceListPath();
		static QString cachePath();
			/// writes an empty price list (header only) when there is none
		static bool ensurePriceList(QString *error = nullptr);

			/// the key an article is looked up by: part number, upper case,
			/// spaces removed, a leading "SIE." style prefix removed
		static QString key(const QString &part_number);

		static QList<ArticleData> loadArticles(const QString &path, QString *error = nullptr);
		static bool saveCache(const QList<ArticleData> &articles, QString *error = nullptr);
			/// articles by key(); the later of two with the same key wins
		static QHash<QString, ArticleData> byKey(const QList<ArticleData> &articles);

			/// the articles and open devices of @p project, with every
			/// source of commercial data merged in, sorted
		static QList<Item> collect(QETProject *project, QString *error = nullptr);
			/// sorted by groupName(), part number; open ones last
		static void sort(QList<Item> &items);
		static Summary summarize(const QList<Item> &items);

			/// the list for purchasing: grouped by supplier, a sum line
			/// after each group and a grand total, the open devices last.
			/// CSV with ';' and decimal commas, as a German spreadsheet
			/// opens it.
		static QByteArray toCsv(const QList<Item> &items);
			/// the same as an Excel workbook (.xlsx): prices editable,
			/// totals and sums as formulas
		static QByteArray toXlsx(const QList<Item> &items, QString *error = nullptr);
			/// neutral CSV for an ERP import: one record per article,
			/// fixed English column names, '.' decimals, no sum lines
		static QByteArray toErpCsv(const QList<Item> &items);

			/// writes @p output: .xlsx as workbook, anything else as CSV;
			/// with @p erp also <name>_ERP.csv beside it
		static bool write(const QList<Item> &items, const QString &output, bool erp,
						  QStringList *written = nullptr, QString *error = nullptr);
		static QString erpPath(const QString &output);

			/// a number as the price list or an element field writes it
			/// ("12,50", "1.234,50", "12.5 €"); < 0 when it is none
		static double parseNumber(const QString &text);
			/// what EPLAN writes in identcode: "4011209780774 EAN" -> number, type
		static void splitIdentcode(const QString &identcode, QString *gtin, QString *type);
			/// the column headings of the purchasing list
		static QStringList headers();
		static QStringList erpHeaders();
};

#endif // PURCHASELIST_H
