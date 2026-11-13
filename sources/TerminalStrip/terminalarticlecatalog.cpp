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
#include "terminalarticlecatalog.h"

#include "terminalcatalog.h"
#include "terminalcolor.h"
#include <qetapp.h>

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QRegularExpression>
#include <QTextStream>

namespace
{
	using namespace TerminalArticleCatalog;

	struct Row
	{
		const char *manufacturer, *order, *designation;
		Kind kind; double section, width; Clamp clamp;
		const char *color; int conductors, levels;
	};

		//Common rail-mounted terminals. The order numbers are those of
		//the manufacturers' catalogues; check them before ordering.
	const Row built_in_rows[] = {
		//WAGO TOPJOB S
		{"WAGO", "2001-1201", "TOPJOB S Durchgangsklemme 1,5 mm², 2-Leiter", Kind::FeedThrough, 1.5, 4.2, Clamp::PushIn, "Gray", 2, 1},
		{"WAGO", "2001-1204", "TOPJOB S Durchgangsklemme 1,5 mm², 2-Leiter, blau", Kind::FeedThrough, 1.5, 4.2, Clamp::PushIn, "Blue", 2, 1},
		{"WAGO", "2001-1207", "TOPJOB S Schutzleiterklemme 1,5 mm², 2-Leiter", Kind::ProtectiveEarth, 1.5, 4.2, Clamp::PushIn, "Green/Yellow", 2, 1},
		{"WAGO", "2001-1301", "TOPJOB S Durchgangsklemme 1,5 mm², 3-Leiter", Kind::FeedThrough, 1.5, 4.2, Clamp::PushIn, "Gray", 3, 1},
		{"WAGO", "2001-1401", "TOPJOB S Durchgangsklemme 1,5 mm², 4-Leiter", Kind::FeedThrough, 1.5, 4.2, Clamp::PushIn, "Gray", 4, 1},
		{"WAGO", "2002-1201", "TOPJOB S Durchgangsklemme 2,5 mm², 2-Leiter", Kind::FeedThrough, 2.5, 5.2, Clamp::PushIn, "Gray", 2, 1},
		{"WAGO", "2002-1204", "TOPJOB S Durchgangsklemme 2,5 mm², 2-Leiter, blau", Kind::FeedThrough, 2.5, 5.2, Clamp::PushIn, "Blue", 2, 1},
		{"WAGO", "2002-1207", "TOPJOB S Schutzleiterklemme 2,5 mm², 2-Leiter", Kind::ProtectiveEarth, 2.5, 5.2, Clamp::PushIn, "Green/Yellow", 2, 1},
		{"WAGO", "2002-1301", "TOPJOB S Durchgangsklemme 2,5 mm², 3-Leiter", Kind::FeedThrough, 2.5, 5.2, Clamp::PushIn, "Gray", 3, 1},
		{"WAGO", "2002-1304", "TOPJOB S Durchgangsklemme 2,5 mm², 3-Leiter, blau", Kind::FeedThrough, 2.5, 5.2, Clamp::PushIn, "Blue", 3, 1},
		{"WAGO", "2002-1307", "TOPJOB S Schutzleiterklemme 2,5 mm², 3-Leiter", Kind::ProtectiveEarth, 2.5, 5.2, Clamp::PushIn, "Green/Yellow", 3, 1},
		{"WAGO", "2002-1401", "TOPJOB S Durchgangsklemme 2,5 mm², 4-Leiter", Kind::FeedThrough, 2.5, 5.2, Clamp::PushIn, "Gray", 4, 1},
		{"WAGO", "2002-1404", "TOPJOB S Durchgangsklemme 2,5 mm², 4-Leiter, blau", Kind::FeedThrough, 2.5, 5.2, Clamp::PushIn, "Blue", 4, 1},
		{"WAGO", "2002-1407", "TOPJOB S Schutzleiterklemme 2,5 mm², 4-Leiter", Kind::ProtectiveEarth, 2.5, 5.2, Clamp::PushIn, "Green/Yellow", 4, 1},
		{"WAGO", "2002-2201", "TOPJOB S Doppelstock-Durchgangsklemme 2,5 mm²", Kind::DoubleDeck, 2.5, 5.2, Clamp::PushIn, "Gray", 4, 2},
		{"WAGO", "2004-1201", "TOPJOB S Durchgangsklemme 4 mm², 2-Leiter", Kind::FeedThrough, 4, 6.2, Clamp::PushIn, "Gray", 2, 1},
		{"WAGO", "2004-1204", "TOPJOB S Durchgangsklemme 4 mm², 2-Leiter, blau", Kind::FeedThrough, 4, 6.2, Clamp::PushIn, "Blue", 2, 1},
		{"WAGO", "2004-1207", "TOPJOB S Schutzleiterklemme 4 mm², 2-Leiter", Kind::ProtectiveEarth, 4, 6.2, Clamp::PushIn, "Green/Yellow", 2, 1},
		{"WAGO", "2006-1201", "TOPJOB S Durchgangsklemme 6 mm², 2-Leiter", Kind::FeedThrough, 6, 7.5, Clamp::PushIn, "Gray", 2, 1},
		{"WAGO", "2006-1204", "TOPJOB S Durchgangsklemme 6 mm², 2-Leiter, blau", Kind::FeedThrough, 6, 7.5, Clamp::PushIn, "Blue", 2, 1},
		{"WAGO", "2006-1207", "TOPJOB S Schutzleiterklemme 6 mm², 2-Leiter", Kind::ProtectiveEarth, 6, 7.5, Clamp::PushIn, "Green/Yellow", 2, 1},
		{"WAGO", "2010-1201", "TOPJOB S Durchgangsklemme 10 mm², 2-Leiter", Kind::FeedThrough, 10, 10.0, Clamp::PushIn, "Gray", 2, 1},
		{"WAGO", "2010-1204", "TOPJOB S Durchgangsklemme 10 mm², 2-Leiter, blau", Kind::FeedThrough, 10, 10.0, Clamp::PushIn, "Blue", 2, 1},
		{"WAGO", "2010-1207", "TOPJOB S Schutzleiterklemme 10 mm², 2-Leiter", Kind::ProtectiveEarth, 10, 10.0, Clamp::PushIn, "Green/Yellow", 2, 1},
		{"WAGO", "2016-1201", "TOPJOB S Durchgangsklemme 16 mm², 2-Leiter", Kind::FeedThrough, 16, 12.0, Clamp::PushIn, "Gray", 2, 1},
		{"WAGO", "2016-1204", "TOPJOB S Durchgangsklemme 16 mm², 2-Leiter, blau", Kind::FeedThrough, 16, 12.0, Clamp::PushIn, "Blue", 2, 1},
		{"WAGO", "2016-1207", "TOPJOB S Schutzleiterklemme 16 mm², 2-Leiter", Kind::ProtectiveEarth, 16, 12.0, Clamp::PushIn, "Green/Yellow", 2, 1},
		//Phoenix Contact PT (push-in)
		{"Phoenix Contact", "3209510", "PT 2,5 Durchgangsreihenklemme", Kind::FeedThrough, 2.5, 5.2, Clamp::PushIn, "Gray", 2, 1},
		{"Phoenix Contact", "3209523", "PT 2,5 BU Durchgangsreihenklemme, blau", Kind::FeedThrough, 2.5, 5.2, Clamp::PushIn, "Blue", 2, 1},
		{"Phoenix Contact", "3209536", "PT 2,5-PE Schutzleiterklemme", Kind::ProtectiveEarth, 2.5, 5.2, Clamp::PushIn, "Green/Yellow", 2, 1},
		{"Phoenix Contact", "3211757", "PT 4 Durchgangsreihenklemme", Kind::FeedThrough, 4, 6.2, Clamp::PushIn, "Gray", 2, 1},
		{"Phoenix Contact", "3211760", "PT 4 BU Durchgangsreihenklemme, blau", Kind::FeedThrough, 4, 6.2, Clamp::PushIn, "Blue", 2, 1},
		{"Phoenix Contact", "3211766", "PT 4-PE Schutzleiterklemme", Kind::ProtectiveEarth, 4, 6.2, Clamp::PushIn, "Green/Yellow", 2, 1},
		//Phoenix Contact ST (spring cage)
		{"Phoenix Contact", "3031212", "ST 2,5 Durchgangsreihenklemme", Kind::FeedThrough, 2.5, 5.2, Clamp::Spring, "Gray", 2, 1},
		{"Phoenix Contact", "3031225", "ST 2,5 BU Durchgangsreihenklemme, blau", Kind::FeedThrough, 2.5, 5.2, Clamp::Spring, "Blue", 2, 1},
		{"Phoenix Contact", "3031238", "ST 2,5-PE Schutzleiterklemme", Kind::ProtectiveEarth, 2.5, 5.2, Clamp::Spring, "Green/Yellow", 2, 1},
		{"Phoenix Contact", "3031364", "ST 4 Durchgangsreihenklemme", Kind::FeedThrough, 4, 6.2, Clamp::Spring, "Gray", 2, 1},
		{"Phoenix Contact", "3031377", "ST 4 BU Durchgangsreihenklemme, blau", Kind::FeedThrough, 4, 6.2, Clamp::Spring, "Blue", 2, 1},
		{"Phoenix Contact", "3031380", "ST 4-PE Schutzleiterklemme", Kind::ProtectiveEarth, 4, 6.2, Clamp::Spring, "Green/Yellow", 2, 1},
		//Phoenix Contact UT (screw)
		{"Phoenix Contact", "3044076", "UT 2,5 Durchgangsreihenklemme", Kind::FeedThrough, 2.5, 5.2, Clamp::Screw, "Gray", 2, 1},
		{"Phoenix Contact", "3044089", "UT 2,5 BU Durchgangsreihenklemme, blau", Kind::FeedThrough, 2.5, 5.2, Clamp::Screw, "Blue", 2, 1},
		{"Phoenix Contact", "3044092", "UT 2,5-PE Schutzleiterklemme", Kind::ProtectiveEarth, 2.5, 5.2, Clamp::Screw, "Green/Yellow", 2, 1},
		{"Phoenix Contact", "3044102", "UT 4 Durchgangsreihenklemme", Kind::FeedThrough, 4, 6.2, Clamp::Screw, "Gray", 2, 1},
		{"Phoenix Contact", "3044115", "UT 4 BU Durchgangsreihenklemme, blau", Kind::FeedThrough, 4, 6.2, Clamp::Screw, "Blue", 2, 1},
		{"Phoenix Contact", "3044128", "UT 4-PE Schutzleiterklemme", Kind::ProtectiveEarth, 4, 6.2, Clamp::Screw, "Green/Yellow", 2, 1},
		{"Phoenix Contact", "3044131", "UT 6 Durchgangsreihenklemme", Kind::FeedThrough, 6, 8.2, Clamp::Screw, "Gray", 2, 1},
		{"Phoenix Contact", "3044160", "UT 10 Durchgangsreihenklemme", Kind::FeedThrough, 10, 10.2, Clamp::Screw, "Gray", 2, 1},
		//Weidmüller A-Reihe (push-in)
		{"Weidmüller", "1521850000", "A2C 2.5 Durchgangsreihenklemme", Kind::FeedThrough, 2.5, 5.1, Clamp::PushIn, "Gray", 2, 1},
		{"Weidmüller", "1521880000", "A2C 2.5 BL Durchgangsreihenklemme, blau", Kind::FeedThrough, 2.5, 5.1, Clamp::PushIn, "Blue", 2, 1},
		{"Weidmüller", "1521680000", "A2C 2.5 PE Schutzleiterklemme", Kind::ProtectiveEarth, 2.5, 5.1, Clamp::PushIn, "Green/Yellow", 2, 1},
		//Weidmüller W-Reihe (screw)
		{"Weidmüller", "1020000000", "WDU 2.5 Durchgangsreihenklemme", Kind::FeedThrough, 2.5, 5.1, Clamp::Screw, "Gray", 2, 1},
		{"Weidmüller", "1020080000", "WDU 2.5 BL Durchgangsreihenklemme, blau", Kind::FeedThrough, 2.5, 5.1, Clamp::Screw, "Blue", 2, 1},
		{"Weidmüller", "1010000000", "WPE 2.5 Schutzleiterklemme", Kind::ProtectiveEarth, 2.5, 5.1, Clamp::Screw, "Green/Yellow", 2, 1},
		{"Weidmüller", "1020100000", "WDU 4 Durchgangsreihenklemme", Kind::FeedThrough, 4, 6.1, Clamp::Screw, "Gray", 2, 1},
		{"Weidmüller", "1020180000", "WDU 4 BL Durchgangsreihenklemme, blau", Kind::FeedThrough, 4, 6.1, Clamp::Screw, "Blue", 2, 1},
		{"Weidmüller", "1010100000", "WPE 4 Schutzleiterklemme", Kind::ProtectiveEarth, 4, 6.1, Clamp::Screw, "Green/Yellow", 2, 1},
		{"Weidmüller", "1020200000", "WDU 6 Durchgangsreihenklemme", Kind::FeedThrough, 6, 7.9, Clamp::Screw, "Gray", 2, 1},
		{"Weidmüller", "1010200000", "WPE 6 Schutzleiterklemme", Kind::ProtectiveEarth, 6, 7.9, Clamp::Screw, "Green/Yellow", 2, 1},
		{"Weidmüller", "1020300000", "WDU 10 Durchgangsreihenklemme", Kind::FeedThrough, 10, 9.9, Clamp::Screw, "Gray", 2, 1},
		{"Weidmüller", "1010300000", "WPE 10 Schutzleiterklemme", Kind::ProtectiveEarth, 10, 9.9, Clamp::Screw, "Green/Yellow", 2, 1},
	};

	const char *kind_keys[]  = {"feed-through", "pe", "double-deck", "fuse", "disconnect"};
	const char *clamp_keys[] = {"push-in", "spring", "screw"};

	Kind kindFromKey(const QString &key)
	{
		const auto k_ = key.trimmed().toLower();
		for (int i = 0; i < 5; ++i) {
			if (k_ == QLatin1String(kind_keys[i])) return Kind(i);
		}
		if (k_ == QLatin1String("protective-earth") || k_ == QLatin1String("ground")) {
			return Kind::ProtectiveEarth;
		}
		return Kind::FeedThrough;
	}

	Clamp clampFromKey(const QString &key)
	{
		const auto k_ = key.trimmed().toLower();
		for (int i = 0; i < 3; ++i) {
			if (k_ == QLatin1String(clamp_keys[i])) return Clamp(i);
		}
		return Clamp::PushIn;
	}

	double number(const QString &text)
	{
		QString t_ = text.trimmed();
		t_.replace(QLatin1Char(','), QLatin1Char('.'));
		return t_.toDouble();
	}

	QVector<Article> readFile(const QString &path)
	{
		QVector<Article> list_;
		QFile file_(path);
		if (!file_.open(QIODevice::ReadOnly | QIODevice::Text)) {
			return list_;
		}
		QTextStream in_(&file_);
		in_.setEncoding(QStringConverter::Utf8);
		QStringList header_;
		while (!in_.atEnd())
		{
			const QString line_ = in_.readLine();
			if (line_.trimmed().isEmpty() || line_.startsWith(QLatin1Char('#'))) {
				continue;
			}
			QStringList cells_ = line_.split(QLatin1Char(';'));
			for (auto &cell_ : cells_) {
				cell_ = cell_.trimmed();
				if (cell_.size() >= 2 && cell_.startsWith(QLatin1Char('"')) && cell_.endsWith(QLatin1Char('"'))) {
					cell_ = cell_.mid(1, cell_.size() - 2);
				}
			}
			if (header_.isEmpty()) {
				header_ = cells_;
				for (auto &h_ : header_) {
					h_ = h_.toLower().remove(QChar(0xFEFF));
				}
				continue;
			}
			const auto cell = [&](const char *name) {
				const int i_ = header_.indexOf(QLatin1String(name));
				return i_ >= 0 ? cells_.value(i_) : QString();
			};
			Article a_;
			a_.manufacturer = cell("manufacturer");
			a_.order_number = cell("order_number");
			if (a_.order_number.isEmpty()) {
				continue;
			}
			a_.designation = cell("designation");
			a_.kind = kindFromKey(cell("kind"));
			a_.section = number(cell("section"));
			a_.width = number(cell("width"));
			a_.clamp = clampFromKey(cell("clamp"));
			a_.color = cell("color");
			a_.conductors = qMax(2, cell("conductors").toInt());
			a_.levels = qMax(1, cell("levels").toInt());
			a_.type_number = cell("type_number");
			a_.dataportal_id = cell("dataportal_id");
			a_.source = QFileInfo(path).fileName();
			list_ << a_;
		}
		return list_;
	}

	QVector<Article> &cache()
	{
		static QVector<Article> cache_;
		return cache_;
	}
	bool &loaded()
	{
		static bool loaded_ = false;
		return loaded_;
	}
}

QVector<TerminalArticleCatalog::Article> TerminalArticleCatalog::articles()
{
	if (!loaded()) {
		reload();
	}
	return cache();
}

void TerminalArticleCatalog::reload()
{
	auto &list_ = cache();
	list_.clear();
	for (const auto &row_ : built_in_rows)
	{
		Article a_;
		a_.manufacturer = QString::fromUtf8(row_.manufacturer);
		a_.order_number = QString::fromUtf8(row_.order);
		a_.designation = QString::fromUtf8(row_.designation);
		a_.kind = row_.kind;
		a_.section = row_.section;
		a_.width = row_.width;
		a_.clamp = row_.clamp;
		a_.color = QString::fromUtf8(row_.color);
		a_.conductors = row_.conductors;
		a_.levels = row_.levels;
		a_.source = QStringLiteral("built-in");
		list_ << a_;
	}

	const auto key_ = [](const Article &a) {
		return a.manufacturer.toLower() + QChar(0x1f) + a.order_number;
	};
	QHash<QString, int> index_;
	for (int i = 0; i < list_.size(); ++i) {
		index_.insert(key_(list_.at(i)), i);
	}

	const QDir dir_(catalogDirectory(false));
	if (dir_.exists())
	{
		for (const auto &name_ : dir_.entryList({QStringLiteral("*.csv")}, QDir::Files, QDir::Name))
		{
			for (const auto &a_ : readFile(dir_.filePath(name_)))
			{
					//A file corrects an article seen before: same number, the later wins
				const QString k_ = key_(a_);
				const auto it_ = index_.constFind(k_);
				if (it_ != index_.constEnd()) {
					list_[it_.value()] = a_;
				} else {
					index_.insert(k_, int(list_.size()));
					list_ << a_;
				}
			}
		}
	}
	loaded() = true;
}

TerminalArticleCatalog::Article TerminalArticleCatalog::find(const QString &article_text)
{
	const auto text_ = article_text.simplified();
	if (text_.isEmpty()) {
		return Article();
	}
	for (const auto &a_ : articles())
	{
		if (a_.articleText().compare(text_, Qt::CaseInsensitive) == 0
			|| a_.order_number.compare(text_, Qt::CaseInsensitive) == 0) {
			return a_;
		}
	}
	return Article();
}

QString TerminalArticleCatalog::catalogDirectory(bool create)
{
	const QString path_ = QETApp::dataDir() + QStringLiteral("/terminal-catalogs");
	if (create)
	{
		QDir().mkpath(path_);
		const QString template_ = path_ + QStringLiteral("/Vorlage.csv.txt");
		if (QDir(path_).entryList({QStringLiteral("*.csv")}, QDir::Files).isEmpty()
			&& !QFileInfo::exists(template_))
		{
			QFile file_(template_);
			if (file_.open(QIODevice::WriteOnly | QIODevice::Text))
			{
				QTextStream out_(&file_);
				out_.setEncoding(QStringConverter::Utf8);
				out_ << "# Eigene Klemmen: diese Datei als <Name>.csv speichern (Endung .txt weg).\n"
					 << "# kind: feed-through, pe, double-deck, fuse, disconnect | clamp: push-in, spring, screw\n"
					 << "# color: Gray, Blue, Green/Yellow, Brown, Black, Red, Orange, Yellow, Green, Violet, White\n"
					 << "manufacturer;order_number;designation;kind;section;width;clamp;color;conductors;levels\n"
					 << QString::fromUtf8("WAGO;2002-1201;TOPJOB S Durchgangsklemme 2,5 mm²;feed-through;2.5;5.2;push-in;Gray;2;1\n");
			}
		}
	}
	return path_;
}

QString TerminalArticleCatalog::kindName(Kind kind)
{
	switch (kind) {
		case Kind::FeedThrough:     return QCoreApplication::translate("TerminalArticleCatalog", "Feed-through");
		case Kind::ProtectiveEarth: return QCoreApplication::translate("TerminalArticleCatalog", "Protective earth");
		case Kind::DoubleDeck:      return QCoreApplication::translate("TerminalArticleCatalog", "Double-deck");
		case Kind::Fuse:            return QCoreApplication::translate("TerminalArticleCatalog", "Fuse");
		case Kind::Disconnect:      return QCoreApplication::translate("TerminalArticleCatalog", "Disconnect");
	}
	return QString();
}

QString TerminalArticleCatalog::clampName(Clamp clamp)
{
	switch (clamp) {
		case Clamp::PushIn: return QCoreApplication::translate("TerminalArticleCatalog", "Push-in");
		case Clamp::Spring: return QCoreApplication::translate("TerminalArticleCatalog", "Spring cage");
		case Clamp::Screw:  return QCoreApplication::translate("TerminalArticleCatalog", "Screw");
	}
	return QString();
}

QString TerminalArticleCatalog::colorSpec(const Article &article)
{
	QColor main_, stripe_;
	if (!TerminalColor::parse(article.color, &main_, &stripe_)) {
		return QString();
	}
	return TerminalColor::toSpec(main_, stripe_);
}

QString TerminalArticleCatalog::drawingCatalogEntry(const Article &article)
{
	const TerminalCatalog::WidthEntry *best_ = nullptr;
	double best_diff_ = 0.25;	//no entry further than a quarter millimetre
	for (const auto &entry_ : TerminalCatalog::widthEntries())
	{
		const auto maker_ = QString::fromUtf8(entry_.manufacturer);
		if (!article.manufacturer.startsWith(maker_.section(QLatin1Char(' '), 0, 0), Qt::CaseInsensitive)) {
			continue;
		}
			//Only the same clamp: the drawing shows screw heads or
			//push-in openings from this entry
		const bool same_clamp_ =
			(entry_.clamp == TerminalCatalog::Clamp::Screw  && article.clamp == Clamp::Screw) ||
			(entry_.clamp == TerminalCatalog::Clamp::PushIn && article.clamp == Clamp::PushIn) ||
			(entry_.clamp == TerminalCatalog::Clamp::Spring && article.clamp == Clamp::Spring);
		if (!same_clamp_) {
			continue;
		}
		const double diff_ = qAbs(entry_.mm - article.width);
		if (diff_ < best_diff_) {
			best_diff_ = diff_;
			best_ = &entry_;
		}
	}
	return best_ ? QString::fromUtf8(best_->label) : QString();
}

namespace
{
	double germanNumber(const QString &text)
	{
		QString t_ = text.trimmed();
		t_.replace(QLatin1Char(','), QLatin1Char('.'));
		return t_.toDouble();
	}

		///A part of a comma separated list made of the pattern alone ("2,5 mm²")
	QString listPart(const QString &text, const QString &pattern)
	{
		const QRegularExpression re_(QStringLiteral("^\\s*") + pattern + QStringLiteral("\\s*$"));
			//The parts are separated by ", "; "2,5" without a space is a number
		for (const auto &part_ : text.split(QRegularExpression(QStringLiteral(",\\s+"))))
		{
			const auto m_ = re_.match(part_);
			if (m_.hasMatch()) {
				return m_.captured(1);
			}
		}
		return QString();
	}

	QString colourOf(const QString &designation, const QString &description)
	{
		static const QVector<QPair<QString, QString>> colours_ {
			{QStringLiteral("gr[üu]n[-/ ]?gelb|gn/?ge|gnye"), QStringLiteral("Green/Yellow")},
			{QStringLiteral("schwarz/gelb"), QStringLiteral("#303030/#ffd800")},
			{QStringLiteral("dunkelbeige|beige"), QStringLiteral("#cdbb93")},
			{QStringLiteral("hellgrau|grau"), QStringLiteral("Gray")},
			{QStringLiteral("blau"), QStringLiteral("Blue")},
			{QStringLiteral("schwarz"), QStringLiteral("Black")},
			{QStringLiteral("rot"), QStringLiteral("Red")},
			{QStringLiteral("orange"), QStringLiteral("Orange")},
			{QStringLiteral("violett"), QStringLiteral("Violet")},
			{QStringLiteral("wei[ßs]"), QStringLiteral("White")},
			{QStringLiteral("braun"), QStringLiteral("Brown")},
			{QStringLiteral("pink"), QStringLiteral("#e91e63")},
			{QStringLiteral("gelb"), QStringLiteral("Yellow")},
			{QStringLiteral("gr[üu]n"), QStringLiteral("Green")}
		};
		QStringList candidates_;
		const auto m_ = QRegularExpression(QStringLiteral("Farbe:\\s*([^,]+)")).match(description);
		if (m_.hasMatch()) {
			candidates_ << m_.captured(1);
		}
		candidates_ << designation.split(QRegularExpression(QStringLiteral(",\\s+")));
		for (auto candidate_ : candidates_)
		{
			candidate_ = candidate_.trimmed().toLower();
			for (const auto &c_ : colours_) {
				if (QRegularExpression(QStringLiteral("^(?:") + c_.first + QStringLiteral(")$")).match(candidate_).hasMatch()) {
					return c_.second;
				}
			}
		}
		return QString();
	}

	int intOf(const QString &pattern, const QString &text)
	{
		const auto m_ = QRegularExpression(pattern).match(text);
		return m_.hasMatch() ? m_.captured(1).toInt() : 0;
	}
}

TerminalArticleCatalog::Article TerminalArticleCatalog::fromDataPortal(
		const QString &manufacturer, const QString &order_number, const QString &type_number,
		const QString &designation, const QString &description, const QString &dataportal_id)
{
	Article a_;
	a_.manufacturer = manufacturer;
	a_.order_number = order_number;
	a_.type_number = type_number;
	a_.dataportal_id = dataportal_id;
	a_.source = QStringLiteral("online");

	const QString short_ = designation.section(QStringLiteral(" / "), -1).section(QLatin1Char(','), 0, 0).trimmed();
	a_.designation = (!type_number.isEmpty() && type_number != order_number)
			? type_number + QStringLiteral(" – ") + short_ : short_;

	const QString all_ = (designation + QLatin1Char(' ') + description).toLower();
	if (all_.contains(QStringLiteral("schutzleiter"))) a_.kind = Kind::ProtectiveEarth;
	else if (all_.contains(QStringLiteral("sicherung"))) a_.kind = Kind::Fuse;
	else if (all_.contains(QStringLiteral("trennklemme")) || all_.contains(QStringLiteral("messtrenn"))
			 || all_.contains(QStringLiteral("trenn-reihenklemme"))) a_.kind = Kind::Disconnect;
	else if (all_.contains(QStringLiteral("doppelstock")) || all_.contains(QStringLiteral("etagen"))
			 || all_.contains(QStringLiteral("mehrstock"))) a_.kind = Kind::DoubleDeck;

	if (all_.contains(QStringLiteral("push-in")) || all_.contains(QStringLiteral("push in"))) a_.clamp = Clamp::PushIn;
	else if (all_.contains(QStringLiteral("zugfeder")) || all_.contains(QStringLiteral("cage clamp"))
			 || all_.contains(QStringLiteral("federkraft"))) a_.clamp = Clamp::Spring;
	else if (all_.contains(QStringLiteral("schraub")) || all_.contains(QStringLiteral("bolzen"))) a_.clamp = Clamp::Screw;

	const auto rated_ = QRegularExpression(QStringLiteral("Bemessungsquerschnitt:\\s*([\\d,]+)\\s*mm")).match(description);
	if (rated_.hasMatch()) {
		a_.section = germanNumber(rated_.captured(1));
	} else {
		QString s_ = listPart(designation, QStringLiteral("(\\d+(?:[.,]\\d+)?)\\s*mm²"));
		if (s_.isEmpty()) s_ = listPart(description, QStringLiteral("(\\d+(?:[.,]\\d+)?)\\s*mm²"));
		if (!s_.isEmpty()) a_.section = germanNumber(s_);
	}
	const auto width_ = QRegularExpression(QStringLiteral("Breite:\\s*([\\d,]+)\\s*mm")).match(description);
	if (width_.hasMatch()) a_.width = germanNumber(width_.captured(1));

	a_.color = colourOf(designation, description);
	const QString text_ = designation + QStringLiteral(", ") + description;
	const int conductors_ = intOf(QStringLiteral("Anzahl (?:der )?Anschlüsse:\\s*(\\d+)"), text_);
	a_.conductors = conductors_ ? conductors_ : qMax(2, intOf(QStringLiteral("(\\d)-Leiter"), text_));
	int levels_ = intOf(QStringLiteral("Anzahl der Etagen:\\s*(\\d+)"), text_);
	if (!levels_) {
			//"1., 2. und 3. Etage"
		auto it_ = QRegularExpression(QStringLiteral("(\\d)\\.\\s*(?:und\\s*)?Etage")).globalMatch(text_);
		while (it_.hasNext()) levels_ = qMax(levels_, it_.next().captured(1).toInt());
	}
	a_.levels = levels_ ? levels_ : (a_.kind == Kind::DoubleDeck ? 2 : 1);
	return a_;
}

bool TerminalArticleCatalog::remember(const Article &article)
{
	if (!article.isValid()) {
		return false;
	}
	const QString dir_ = catalogDirectory(true);
	QFile file_(dir_ + QStringLiteral("/Data Portal.csv"));
	const bool new_file_ = !file_.exists();
	if (!file_.open(QIODevice::Append | QIODevice::Text)) {
		return false;
	}
	QTextStream out_(&file_);
	out_.setEncoding(QStringConverter::Utf8);
	if (new_file_) {
		out_ << "manufacturer;order_number;designation;kind;section;width;clamp;color;conductors;levels;type_number;dataportal_id\n";
	}
	const auto clean_ = [](QString text) { return text.replace(QLatin1Char(';'), QLatin1Char(',')); };
	out_ << clean_(article.manufacturer) << ';' << clean_(article.order_number) << ';'
		 << clean_(article.designation) << ';' << QLatin1String(kind_keys[int(article.kind)]) << ';'
		 << (article.section > 0 ? QString::number(article.section) : QString()) << ';'
		 << (article.width > 0 ? QString::number(article.width) : QString()) << ';'
		 << QLatin1String(clamp_keys[int(article.clamp)]) << ';' << clean_(article.color) << ';'
		 << article.conductors << ';' << article.levels << ';'
		 << clean_(article.type_number) << ';' << clean_(article.dataportal_id) << '\n';
	file_.close();
	reload();
	return true;
}

QString TerminalArticleCatalog::dataPortalShortName(const QString &manufacturer)
{
	const auto m_ = manufacturer.toLower();
	if (m_.startsWith(QStringLiteral("phoenix"))) return QStringLiteral("PXC");
	if (m_.startsWith(QStringLiteral("weidm"))) return QStringLiteral("WEI");
	if (m_.startsWith(QStringLiteral("wago"))) return QStringLiteral("WAGO");
	return QString();
}
