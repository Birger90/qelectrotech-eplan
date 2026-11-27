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

	Ported from KoperCad, crate novo-safety (MIT), see iso13849.h.
*/
#include "iso13849.h"

#include <QHash>
#include <QJsonArray>
#include <algorithm>
#include <cmath>
#include <limits>

namespace Iso13849 {

namespace {

const double kHoursPerYear = 8760.0;
const double kMissionH = 175200.0;   // 20 years
const double kMttrH = 8.0;
const double kBeta = 0.02;           // common cause with a passed CCF score
const double kBetaFailed = 0.1;
const int kCcfRequired = 65;
const double kMttfdCap = 100.0;

Pl maxPl(Category c)
{
	switch (c) {
		case Category::B: return Pl::B;
		case Category::C1: return Pl::C;
		case Category::C2:
		case Category::C3: return Pl::D;
		case Category::C4: return Pl::E;
	}
	return Pl::None;
}

int dcRank(double percent)
{
	if (percent >= 99.0) return 3;
	if (percent >= 90.0) return 2;
	if (percent >= 60.0) return 1;
	return 0;
}

double pfhdModel(Category cat, double mttfd_years, double dc, bool ccf_ok)
{
	const double lambda_d = 1.0 / (mttfd_years * kHoursPerYear);
	switch (cat) {
		case Category::B:
		case Category::C1: return lambda_d;
		case Category::C2: return lambda_d * (1.0 - dc);
		case Category::C3:
		case Category::C4: {
			const double beta = ccf_ok ? kBeta : kBetaFailed;
			const double ldd = dc * lambda_d, ldu = (1.0 - dc) * lambda_d;
			const double t_ce = (ldu / lambda_d) * (kMissionH / 2.0 + kMttrH) + (ldd / lambda_d) * kMttrH;
			const double a = (1.0 - beta / 2.0) * ldd + (1.0 - beta) * ldu;
			const double b = (1.0 - beta) * lambda_d;
			return 2.0 * a * b * t_ce + beta * lambda_d;
		}
	}
	return lambda_d;
}

QString categoryLabel(Category c)
{
	switch (c) {
		case Category::B: return QStringLiteral("B");
		case Category::C1: return QStringLiteral("1");
		case Category::C2: return QStringLiteral("2");
		case Category::C3: return QStringLiteral("3");
		case Category::C4: return QStringLiteral("4");
	}
	return QString();
}

} // namespace

double Component::years() const
{
	if (mttfd_years > 0) return mttfd_years;
	if (nop <= 0) return std::numeric_limits<double>::infinity();
	return b10d / (0.1 * nop);
}

QString label(Pl pl)
{
	switch (pl) {
		case Pl::None: return QStringLiteral("below a");
		case Pl::A: return QStringLiteral("a");
		case Pl::B: return QStringLiteral("b");
		case Pl::C: return QStringLiteral("c");
		case Pl::D: return QStringLiteral("d");
		case Pl::E: return QStringLiteral("e");
	}
	return QString();
}

Pl fromPfhd(double pfhd)
{
		// ISO 13849-1 table 2
	if (pfhd < 1e-7) return Pl::E;
	if (pfhd < 1e-6) return Pl::D;
	if (pfhd < 3e-6) return Pl::C;
	if (pfhd < 1e-5) return Pl::B;
	if (pfhd < 1e-4) return Pl::A;
	return Pl::None;
}

Category categoryFrom(const QString &text, bool *ok)
{
	const QString t = text.trimmed().toUpper().remove(QStringLiteral("KAT")).remove(QStringLiteral("CAT")).trimmed();
	if (ok) *ok = true;
	if (t == QLatin1String("B")) return Category::B;
	if (t == QLatin1String("1")) return Category::C1;
	if (t == QLatin1String("2")) return Category::C2;
	if (t == QLatin1String("3")) return Category::C3;
	if (t == QLatin1String("4")) return Category::C4;
	if (ok) *ok = false;
	return Category::B;
}

double operationsPerYear(double days, double hours, double cycle_s)
{
	return cycle_s > 0 ? days * hours * 3600.0 / cycle_s : 0;
}

double channelMttfd(const QList<Component> &parts)
{
	double s = 0;
	for (const Component &c : parts) s += 1.0 / c.years();
	return s > 0 ? 1.0 / s : std::numeric_limits<double>::infinity();
}

double dcAvg(const QList<QPair<double, double>> &mttfd_dc)
{
	double denom = 0, num = 0;
	for (const auto &p : mttfd_dc) {denom += 1.0 / p.first; num += p.second / p.first;}
	return denom > 0 ? num / denom : 0;
}

SubsystemResult calculate(const Subsystem &sub)
{
	SubsystemResult r;
	const double m1 = channelMttfd(sub.channel1);
	const double m2 = sub.channel2.isEmpty() ? m1 : channelMttfd(sub.channel2);
	double m = m1;
	if ((sub.category == Category::C3 || sub.category == Category::C4) && std::abs(m1 - m2) > 1e-12)
		m = 2.0 / 3.0 * (m1 + m2 - 1.0 / (1.0 / m1 + 1.0 / m2));   // annex D.2
	if (m > kMttfdCap) m = kMttfdCap;
	r.mttfd_years = m;
	if (m >= 30) r.mttfd_level = QStringLiteral("high");
	else if (m >= 10) r.mttfd_level = QStringLiteral("medium");
	else if (m >= 3) r.mttfd_level = QStringLiteral("low");
	else r.problems << QStringLiteral("MTTFd %1 years is below 3 years: not allowed").arg(m, 0, 'f', 1);
	const int dc = dcRank(sub.dc_percent);
	static const char *dc_names[] = {"none", "low", "medium", "high"};
	r.dc_level = QString::fromLatin1(dc_names[dc]);

	const bool ccf_ok = sub.ccf_score >= kCcfRequired;
	const bool needs_ccf = sub.category == Category::C2 || sub.category == Category::C3 || sub.category == Category::C4;
	if (needs_ccf && !ccf_ok)
		r.problems << QStringLiteral("CCF score %1 is below the required %2").arg(sub.ccf_score).arg(kCcfRequired);
	if ((sub.category == Category::B || sub.category == Category::C1) && dc != 0)
		r.problems << QStringLiteral("categories B and 1 have no diagnostics: DCavg must be none");
	if ((sub.category == Category::C2 || sub.category == Category::C3) && dc < 1)
		r.problems << QStringLiteral("category %1 needs DCavg of at least low (60 percent)").arg(categoryLabel(sub.category));
	if (sub.category == Category::C4 && dc < 3)
		r.problems << QStringLiteral("category 4 needs DCavg high (99 percent)");
	if (sub.category == Category::C1 && m < 30)
		r.problems << QStringLiteral("category 1 needs high MTTFd (30 years or more)");

	r.pfhd = std::min(1.0, pfhdModel(sub.category, m, std::clamp(sub.dc_percent / 100.0, 0.0, 0.999), ccf_ok));
	r.pl_from_pfhd = fromPfhd(r.pfhd);
	r.pl = std::min(r.pl_from_pfhd, maxPl(sub.category));
	if (!r.problems.isEmpty()) r.pl = Pl::None;
	return r;
}

Pl requiredPl(const QString &s, const QString &f, const QString &p)
{
	const QString key = s.trimmed().toUpper() + f.trimmed().toUpper() + p.trimmed().toUpper();
	static const QHash<QString, Pl> graph = {
		{QStringLiteral("S1F1P1"), Pl::A}, {QStringLiteral("S1F1P2"), Pl::B},
		{QStringLiteral("S1F2P1"), Pl::B}, {QStringLiteral("S1F2P2"), Pl::C},
		{QStringLiteral("S2F1P1"), Pl::C}, {QStringLiteral("S2F1P2"), Pl::D},
		{QStringLiteral("S2F2P1"), Pl::D}, {QStringLiteral("S2F2P2"), Pl::E}};
	return graph.value(key, Pl::None);
}

QJsonObject evaluate(const QJsonObject &function)
{
	auto components = [](const QJsonArray &list) {
		QList<Component> out;
		for (const QJsonValue &v : list) {
			const QJsonObject o = v.toObject();
			Component c;
			c.name = o.value(QStringLiteral("name")).toString();
			c.mttfd_years = o.value(QStringLiteral("mttfd")).toDouble();
			c.b10d = o.value(QStringLiteral("b10d")).toDouble();
			c.nop = o.value(QStringLiteral("nop")).toDouble();
			if (c.nop <= 0 && o.contains(QStringLiteral("cycle_s")))
				c.nop = operationsPerYear(o.value(QStringLiteral("days")).toDouble(220),
										  o.value(QStringLiteral("hours")).toDouble(8),
										  o.value(QStringLiteral("cycle_s")).toDouble());
			out << c;
		}
		return out;
	};
	QJsonArray results;
	double pfhd = 0;
	Pl lowest = Pl::E;
	bool any = false;
	for (const QJsonValue &v : function.value(QStringLiteral("subsystems")).toArray()) {
		const QJsonObject o = v.toObject();
		Subsystem sub;
		sub.name = o.value(QStringLiteral("name")).toString();
		bool ok = false;
		const QString category = o.value(QStringLiteral("category")).toVariant().toString();
		sub.category = categoryFrom(category, &ok);
		sub.channel1 = components(o.value(QStringLiteral("channel1")).toArray());
		sub.channel2 = components(o.value(QStringLiteral("channel2")).toArray());
		sub.dc_percent = o.value(QStringLiteral("dc")).toDouble();
		sub.ccf_score = o.value(QStringLiteral("ccf")).toInt();
		SubsystemResult r = calculate(sub);
		if (!ok) r.problems.prepend(QStringLiteral("unknown category \"%1\" (B, 1, 2, 3 or 4)").arg(category));
		if (sub.channel1.isEmpty()) r.problems.prepend(QStringLiteral("channel 1 has no component"));
		if (!r.problems.isEmpty()) r.pl = Pl::None;
		pfhd += r.pfhd;
		lowest = std::min(lowest, r.pl);
		any = true;
		results.append(QJsonObject{{QStringLiteral("name"), sub.name},
								   {QStringLiteral("category"), categoryLabel(sub.category)},
								   {QStringLiteral("mttfd_years"), std::round(r.mttfd_years * 10) / 10},
								   {QStringLiteral("mttfd_level"), r.mttfd_level},
								   {QStringLiteral("dc_level"), r.dc_level},
								   {QStringLiteral("pfhd"), r.pfhd},
								   {QStringLiteral("pl"), label(r.pl)},
								   {QStringLiteral("problems"), QJsonArray::fromStringList(r.problems)}});
	}
	const Pl pl = any ? std::min(fromPfhd(pfhd), lowest) : Pl::None;
	QJsonObject out{{QStringLiteral("name"), function.value(QStringLiteral("name"))},
					{QStringLiteral("subsystems"), results},
					{QStringLiteral("pfhd"), pfhd},
					{QStringLiteral("pl"), label(pl)},
					{QStringLiteral("note"), QStringLiteral("Planning estimate (KoperCad model, not table K.1 of "
															"ISO 13849-1); verify with SISTEMA or the table.")}};
	if (function.contains(QStringLiteral("s"))) {
		const Pl required = requiredPl(function.value(QStringLiteral("s")).toString(),
									   function.value(QStringLiteral("f")).toString(),
									   function.value(QStringLiteral("p")).toString());
		out.insert(QStringLiteral("required_pl"), label(required));
		out.insert(QStringLiteral("fulfilled"), required != Pl::None && pl >= required);
	}
	return out;
}

} // namespace Iso13849
