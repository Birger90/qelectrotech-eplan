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
#ifndef PURCHASEFETCH_H
#define PURCHASEFETCH_H

#include "purchaselist.h"

#include <QCoreApplication>
#include <functional>

/**
	@brief Fills the article data cache of the purchase list.

	What the sources really give (checked 20.11.2026 against the live
	Data Portal API and the EDZ packages of the data folder):
	 - Data Portal, GET /parts, no token: identcode (EAN or GTIN with its
	   kind) and the lifecycle state. No price, delivery time or
	   packaging unit.
	 - Data Portal, GET /download/commercial_data/part/{id}, with token:
	   a CSV of 14 master data columns (numbers, descriptions, links);
	   no price, delivery time, packaging unit or EAN either. Not used.
	 - EDZ package (part.xml): P_ARTICLE_IDENTCODE, PACKAGINGQUANTITY,
	   QUANTITYUNIT, PRICEUNIT and the price fields, which the
	   manufacturers leave at 0 -- a 0 price is taken for none.
	A delivery time is in none of them: it comes from the price list.

	An article looked up less than 30 days ago is not looked up again,
	nor one the Data Portal did not know.
*/
class PurchaseFetch
{
	Q_DECLARE_TR_FUNCTIONS(PurchaseFetch)

	public:
		struct Result
		{
			int articles = 0;     ///< articles of the list
			int cached = 0;       ///< known from an earlier lookup
			int from_portal = 0;
			int from_edz = 0;
			int not_found = 0;
			QString error;        ///< the Data Portal could not be asked
			QJsonObject toJson() const;
		};

			/// @return false to stop
		using Progress = std::function<bool(int done, int total, const QString &part)>;

			/// looks up the articles of @p items that the cache does not
			/// know yet: EDZ packages of the data folder always, the Data
			/// Portal when @p online. Saves the cache.
		static Result fetch(const QList<PurchaseList::Item> &items, bool online,
							const Progress &progress = Progress());

			/// the article in the EDZ package @p edz_path, false when it
			/// gives nothing commercial
		static bool readEdz(const QString &edz_path, PurchaseList::ArticleData *article);
};

#endif // PURCHASEFETCH_H
