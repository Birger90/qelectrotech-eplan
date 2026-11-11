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
#include "panel/enclosuresize.h"

#include <QtTest>

/**
	Finding an enclosure by its width, height and depth: the sizes are
	read from the description of the part (the Data Portal has no field
	for them), and the list holds the enclosures of at least the size
	asked for, the closest fit first.
*/
class tst_EnclosureSize : public QObject
{
	Q_OBJECT

	private slots:
		void fromText_data();
		void fromText();
		void plateFromText_data();
		void plateFromText();
		void excess();
		void choose();
		void chooseWithoutSize();
};

void tst_EnclosureSize::fromText_data()
{
	QTest::addColumn<QString>("text");
	QTest::addColumn<double>("width");
	QTest::addColumn<double>("height");
	QTest::addColumn<double>("depth");

	QTest::newRow("BHT") << QStringLiteral("Anreih-Schranksystem, BHT: 600x2000x600 mm, Stahlblech") << 600.0 << 2000.0 << 600.0;
	QTest::newRow("B x H x T") << QStringLiteral("Schranksystem, B x H x T: 1200x2000x800 mm, zweitürig") << 1200.0 << 2000.0 << 800.0;
	QTest::newRow("words, blanks") << QStringLiteral("Einzelschrank, Breite/Höhe/Tiefe: 1600 x 1800 x 400, Anzahl Türen: 2") << 1600.0 << 1800.0 << 400.0;
	QTest::newRow("WHD") << QStringLiteral("Bayed enclosure system, WHD: 1600x1800x500 mm, with mounting plate") << 1600.0 << 1800.0 << 500.0;
	QTest::newRow("folder name") << QStringLiteral("AB.1234567 - Schrank, BHT_ 600x2000x600 mm, eintürig") << 600.0 << 2000.0 << 600.0;
		// parts of the American market name the height first
	QTest::newRow("HWD") << QStringLiteral("Junction box, HWD 200X150x120 mm, carbon steel") << 150.0 << 200.0 << 120.0;
	QTest::newRow("hwd:") << QStringLiteral("Wallmount 12 HE hwd: 600x500x400") << 500.0 << 600.0 << 400.0;
	QTest::newRow("H x W x D") << QStringLiteral("Box H x W x D: 300 x 200 x 150 mm") << 200.0 << 300.0 << 150.0;
	QTest::newRow("Höhe/Breite/Tiefe") << QStringLiteral("Kasten Höhe/Breite/Tiefe: 300 x 200 x 150") << 200.0 << 300.0 << 150.0;
		// an enclosure on its base, and a size with a fraction
	QTest::newRow("base") << QStringLiteral("Serverschrank, BHT: 800x2000+100x800 mm, 42 HE") << 800.0 << 2100.0 << 800.0;
	QTest::newRow("fraction") << QStringLiteral("Gehäuse, BHT: 182x180x111,5 mm") << 182.0 << 180.0 << 111.5;
		// no three sizes: nothing is guessed
	QTest::newRow("two sizes") << QStringLiteral("Sichttür, für BH: 600x1800 mm") << 0.0 << 0.0 << 0.0;
	QTest::newRow("two heights") << QStringLiteral("Schrank, BHT: 220x350(H1)x437(H2)x155 mm") << 0.0 << 0.0 << 0.0;
	QTest::newRow("none") << QStringLiteral("Floormount Enclosure") << 0.0 << 0.0 << 0.0;
}

void tst_EnclosureSize::fromText()
{
	QFETCH(QString, text);
	QFETCH(double, width);
	QFETCH(double, height);
	QFETCH(double, depth);
	const EnclosureSize s = EnclosureSize::fromText(text);
	QCOMPARE(s.width, width);
	QCOMPARE(s.height, height);
	QCOMPARE(s.depth, depth);
	QCOMPARE(s.isNull(), width == 0);
}

void tst_EnclosureSize::plateFromText_data()
{
	QTest::addColumn<QString>("text");
	QTest::addColumn<double>("width");
	QTest::addColumn<double>("height");

	QTest::newRow("BH") << QStringLiteral("Montageplatte, für Klemmenkästen und Bus-Gehäuse, BH: 275x585 mm") << 275.0 << 585.0;
	QTest::newRow("B x H") << QStringLiteral("Montageplatte für Anreihschrank, B x H: 899 x 1696 mm") << 899.0 << 1696.0;
	QTest::newRow("bare") << QStringLiteral("Montageplatte, für Basicgehäuse, 600 x 1200 mm, Aluminium, 3 mm") << 600.0 << 1200.0;
	QTest::newRow("module") << QStringLiteral("Montageplatten-Modul, 1 BE (250 mm), 2 HE (300 mm), Montageplatte BH: 188x226 mm") << 188.0 << 226.0;
		// the size of the enclosure the plate is for is not the plate's
	QTest::newRow("of the enclosure") << QStringLiteral("Montageplatte, für Klemmenkästen, für BH: 800x300 mm") << 0.0 << 0.0;
	QTest::newRow("three sizes") << QStringLiteral("Schrank, BHT: 600x2000x600 mm") << 0.0 << 0.0;
	QTest::newRow("none") << QStringLiteral("Zwischenstück, H: 1800 mm") << 0.0 << 0.0;
}

void tst_EnclosureSize::plateFromText()
{
	QFETCH(QString, text);
	QFETCH(double, width);
	QFETCH(double, height);
	const EnclosureSize s = EnclosureSize::plateFromText(text);
	QCOMPARE(s.width, width);
	QCOMPARE(s.height, height);
	QCOMPARE(s.depth, 0.0);
	QCOMPARE(EnclosureSize::seriesOf(QStringLiteral("kx.1570800")), QStringLiteral("KX"));
	QCOMPARE(EnclosureSize::seriesOf(QStringLiteral("1570800")), QString());
}

void tst_EnclosureSize::excess()
{
	EnclosureSize have;
	have.width = 800; have.height = 2000; have.depth = 600;

	EnclosureSize ask;
	QCOMPARE(ask.excess(have), 0.0);                 // nothing asked for: everything fits
	ask.width = 800;
	QCOMPARE(ask.excess(have), 0.0);                 // exact
	ask.width = 700; ask.height = 1800;
	QCOMPARE(ask.excess(have), 300.0);               // 100 wider, 200 higher
	ask.depth = 800;
	QVERIFY(ask.excess(have) < 0);                   // not deep enough
	ask = EnclosureSize();
	ask.height = 2000;
	QVERIFY(ask.excess(EnclosureSize()) < 0);        // a part that names no size
}

void tst_EnclosureSize::choose()
{
	const QStringList texts = {
		QStringLiteral("Schrank A, BHT: 600x2000x600 mm, Stahlblech"),         // 0
		QStringLiteral("Schrank B, BHT: 800x2000x600 mm, Stahlblech"),         // 1
		QStringLiteral("Schrank C, BHT: 800x2000x800 mm, Edelstahl"),          // 2
		QStringLiteral("Schrank D, BHT: 1000x2000x600 mm, Stahlblech"),        // 3
		QStringLiteral("Schrank E, BHT: 800x1800x500 mm, Stahlblech"),         // 4
		QStringLiteral("Enclosure F, HWD 2000x800x600 mm"),                    // 5: 800 wide, 2000 high
		QStringLiteral("Zubehör ohne Maße"),                                   // 6
		QString(),                                                             // 7: left out
	};
	EnclosureSize ask;
	ask.width = 800; ask.height = 2000; ask.depth = 600;
	QVector<EnclosureSize> sizes;
		// the exact ones first (in the order given), then the next larger
	QCOMPARE(ask.choose(texts, QString(), &sizes), (QVector<int>{1, 5, 2, 3}));
	QCOMPARE(sizes.size(), texts.size());
	QCOMPARE(sizes.at(5).width, 800.0);
	QCOMPARE(sizes.at(5).height, 2000.0);

		// one size only: a minimum for that one, the others are free
	ask = EnclosureSize();
	ask.depth = 600;
	QCOMPARE(ask.choose(texts, QString()), (QVector<int>{0, 1, 5, 3, 2}));

		// with words: every one of them, upper or lower case
	ask = EnclosureSize();
	ask.width = 800;
	QCOMPARE(ask.choose(texts, QStringLiteral("schrank STAHLBLECH")), (QVector<int>{4, 1, 3}));

		// larger than all there is
	ask.width = 1200;
	QVERIFY(ask.choose(texts, QString()).isEmpty());
}

void tst_EnclosureSize::chooseWithoutSize()
{
	const QStringList texts = {
		QStringLiteral("Zubehör ohne Maße"),
		QStringLiteral("Schrank D, BHT: 1000x2000x600 mm"),
		QStringLiteral("Schrank A, BHT: 600x2000x600 mm"),
		QStringLiteral("Schrank A2, BHT: 600x1800x400 mm"),
	};
		// all of them, small to large, what names no size last
	QCOMPARE(EnclosureSize().choose(texts, QString()), (QVector<int>{3, 2, 1, 0}));
}

QTEST_APPLESS_MAIN(tst_EnclosureSize)
#include "tst_enclosuresize.moc"
