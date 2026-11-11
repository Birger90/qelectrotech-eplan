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
#include "enclosuresize.h"

#include <QRegularExpression>

#include <algorithm>

namespace {
		/// "482,6" or "2000+100" (an enclosure on its base): the sum
	double number(const QString &value, const QString &added)
	{
		QString v = value;
		v.replace(QLatin1Char(','), QLatin1Char('.'));
		return v.toDouble() + added.toDouble();
	}
}

EnclosureSize EnclosureSize::fromText(const QString &text)
{
		//three numbers joined by "x": "800x1400x500", "1600 x 1800 x 400"
	static const QRegularExpression triple(QStringLiteral(
		"(\\d{2,4}(?:[.,]\\d+)?)(?:\\s*\\+\\s*(\\d{2,3}))?\\s*[x×]\\s*"
		"(\\d{2,4}(?:[.,]\\d+)?)(?:\\s*\\+\\s*(\\d{2,3}))?\\s*[x×]\\s*"
		"(\\d{2,4}(?:[.,]\\d+)?)(?:\\s*\\+\\s*(\\d{2,3}))?"),
		QRegularExpression::CaseInsensitiveOption);
		//what stands before them says the order: height first in "HWD",
		//"H x W x D", "Höhe/Breite/Tiefe"; otherwise width first
	static const QRegularExpression height_first(QStringLiteral(
		"(?:(?<![a-zäöü])h\\s*[x/]?\\s*[wb]\\s*[x/]?\\s*[dt](?![a-zäöü])[\\s:.]*"
		"|(?:h[öo]he\\s*[x/]\\s*breite|height\\s*[x/]\\s*width)[^0-9]{0,12})$"),
		QRegularExpression::CaseInsensitiveOption);

	EnclosureSize s;
	const QRegularExpressionMatch m = triple.match(text);
	if (!m.hasMatch()) return s;
	const double a = number(m.captured(1), m.captured(2));
	const double b = number(m.captured(3), m.captured(4));
	s.depth = number(m.captured(5), m.captured(6));
	const QString before = text.left(m.capturedStart()).right(30);
	if (height_first.match(before).hasMatch()) {
		s.height = a;
		s.width = b;
	} else {
		s.width = a;
		s.height = b;
	}
	return s;
}

double EnclosureSize::excess(const EnclosureSize &of) const
{
	double sum = 0;
	const double asked[] = {width, height, depth};
	const double given[] = {of.width, of.height, of.depth};
	for (int i = 0; i < 3; ++i) {
		if (asked[i] <= 0) continue;
			//half a millimetre of slack: "482,6" against 482.6
		if (given[i] <= 0 || given[i] < asked[i] - 0.5) return -1;
		sum += qMax(0.0, given[i] - asked[i]);
	}
	return sum;
}

QVector<int> EnclosureSize::choose(const QStringList &texts, const QString &words,
								   QVector<EnclosureSize> *sizes) const
{
	struct Hit {int index; double excess; EnclosureSize size;};
	static const QRegularExpression blank(QStringLiteral("\\s+"));
	const QStringList needles = words.split(blank, Qt::SkipEmptyParts);
	if (sizes) sizes->fill(EnclosureSize(), texts.size());
	QVector<Hit> hits;
	for (int i = 0; i < texts.size(); ++i) {
		const QString &text = texts.at(i);
		if (text.isEmpty()) continue;
		const EnclosureSize size = fromText(text);
		if (sizes) (*sizes)[i] = size;
		bool all = true;
		for (const QString &n : needles)
			if (!text.contains(n, Qt::CaseInsensitive)) {all = false; break;}
		if (!all) continue;
		const double e = excess(size);
		if (e < 0) continue;
		hits.append({i, e, size});
	}
	std::stable_sort(hits.begin(), hits.end(), [](const Hit &a, const Hit &b) {
		if (a.excess != b.excess) return a.excess < b.excess;
		if (a.size.isNull() != b.size.isNull()) return b.size.isNull();
		if (a.size.width != b.size.width) return a.size.width < b.size.width;
		if (a.size.height != b.size.height) return a.size.height < b.size.height;
		return a.size.depth < b.size.depth;
	});
	QVector<int> out;
	out.reserve(hits.size());
	for (const Hit &h : hits) out.append(h.index);
	return out;
}
