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

	The model is ported from KoperCad, crate novo-safety (pl.rs,
	subsystem.rs, components.rs), https://github.com/Herepiggypiggypiggy/KoperCad,
	Copyright the KoperCad authors, licensed MIT (chosen of MIT OR
	Apache-2.0); the MIT notice is in sources/safety/LICENSE-KoperCad-MIT.
*/
#ifndef ISO13849_H
#define ISO13849_H

#include <QJsonObject>
#include <QList>
#include <QPair>
#include <QString>
#include <QStringList>

/**
	@brief The performance level of a safety function after ISO 13849-1,
	the simplified way (formulas: FORMELN.md, S rows):

	- a component's MTTFd in years from its own MTTFd or from B10d and the
	  operations per year: MTTFd = B10d / (0.1 · nop) (annex C.2), nop =
	  days · hours · 3600 / cycle time;
	- a channel: 1/MTTFd = Σ 1/MTTFd_i, capped at 100 years; two unequal
	  channels (categories 3 and 4) are made symmetric (annex D.2);
	- DCavg, the CCF score (65 needed for categories 2 to 4) and the
	  category give the PFHd and from it the PL, limited by the category;
	- subsystems in series: their PFHd add, the lowest PL limits;
	- the required PL from the risk graph S, F, P (annex A).

	The PFHd of categories 2 to 4 is the closed-form estimate of
	KoperCad, not the table K.1 of the standard: a planning aid, the
	verification is done with SISTEMA or the table.
*/
namespace Iso13849
{
	enum class Pl {None, A, B, C, D, E};
	enum class Category {B, C1, C2, C3, C4};

	struct Component {
		QString name;
		double mttfd_years = 0;   ///< 0: from b10d and nop
		double b10d = 0;          ///< cycles until 10 % fail dangerously
		double nop = 0;           ///< operations per year
		double years() const;
	};

	struct Subsystem {
		QString name;
		Category category = Category::B;
		QList<Component> channel1, channel2;   ///< channel2 empty: as channel1
		double dc_percent = 0;
		int ccf_score = 0;
	};

	struct SubsystemResult {
		double mttfd_years = 0;
		QString mttfd_level;     ///< low, medium, high, empty below 3 years
		QString dc_level;        ///< none, low, medium, high
		double pfhd = 0;
		Pl pl_from_pfhd = Pl::None;
		Pl pl = Pl::None;        ///< after the category limit and the rules
		QStringList problems;
	};

	QString label(Pl pl);
	Pl fromPfhd(double pfhd);
	Category categoryFrom(const QString &text, bool *ok = nullptr);
	double operationsPerYear(double days, double hours, double cycle_s);
	double channelMttfd(const QList<Component> &parts);
	double dcAvg(const QList<QPair<double, double>> &mttfd_dc);
	SubsystemResult calculate(const Subsystem &subsystem);
		/// S1/S2, F1/F2, P1/P2 -> required PL (annex A); Pl::None when invalid
	Pl requiredPl(const QString &s, const QString &f, const QString &p);

	/**
		A safety function as JSON in, its result as JSON out:
		{"name":..., "s":"S2", "f":"F2", "p":"P1",
		 "subsystems":[{"name":"Sensor", "category":"3", "dc":90, "ccf":75,
						"channel1":[{"name":"B1", "mttfd":150} or
									{"name":"K1", "b10d":2e6, "nop":52800} or
									{"name":"S1", "b10d":1e5, "days":220, "hours":16, "cycle_s":60}],
						"channel2":[...]}]}
		-> {"subsystems":[{"name", "mttfd_years", "mttfd_level", "dc_level",
		   "pfhd", "pl", "problems"}], "pfhd", "pl", "required_pl", "fulfilled",
		   "note"}
	*/
	QJsonObject evaluate(const QJsonObject &function);
}

#endif // ISO13849_H
