// SPDX-License-Identifier: GPL-2.0-or-later
/*
	--export-pdf marks the target of a jump as EPLAN does from 2.9 on, and
	the way EPLAN's own PDFs do it: each link to an element leads on, after
	its GoTo, to a JavaScript action "highlight(page index, [L, B, R, T]);".
	The document JavaScript "generalScript" in the catalog's /Names
	/JavaScript defines highlight(), which adds a red-bordered button over
	the target, lets it blink and removes it again. The frame lies inside
	the zoomed rectangle of the jump. Nothing else is added: no form, no
	hidden annotation. A PDF/A export (--pdf-a) has no JavaScript at all,
	as in EPLAN.
*/
#include <QDir>
#include <QFile>
#include <QMap>
#include <QProcess>
#include <QRegularExpression>
#include <QTemporaryDir>
#include <QtTest>
#include "qettesttimeout.h"

class tst_pdfjumpmark : public QObject
{
	Q_OBJECT

	QTemporaryDir m_dir;

	QByteArray exportPdf(const QStringList &extra, const QString &name)
	{
		const QString home = m_dir.filePath(name + QStringLiteral("_home"));
		const QString tmp = m_dir.filePath(name + QStringLiteral("_tmp"));
		const QString out = m_dir.filePath(name + QStringLiteral(".pdf"));
		QDir().mkpath(home);
		QDir().mkpath(tmp);
		QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
		env.insert(QStringLiteral("QT_QPA_PLATFORM"), QStringLiteral("offscreen"));
		env.insert(QStringLiteral("HOME"), home);
		env.insert(QStringLiteral("XDG_CONFIG_HOME"), home + QStringLiteral("/.config"));
		env.insert(QStringLiteral("XDG_DATA_HOME"), home + QStringLiteral("/.local/share"));
		env.insert(QStringLiteral("TMPDIR"), tmp);

		QProcess proc;
		proc.setProcessEnvironment(env);
		proc.start(QStringLiteral(QET_TEST_BINARY_PATH),
				   QStringList{QStringLiteral("--export-pdf"), project(), out} + extra);
		if (!proc.waitForFinished(QetTest::timeout(120000)) || proc.exitCode() != 0)
			return {};
		QFile file(out);
		if (!file.open(QIODevice::ReadOnly))
			return {};
		return file.readAll();
	}

	static QString project()
	{
		return QStringLiteral(QET_EXAMPLES_DIR) + QStringLiteral("/industrial.qet");
	}

	// Object number -> its text, up to endobj.
	static QMap<int, QString> objects(const QByteArray &pdf)
	{
		QMap<int, QString> objs;
		static const QRegularExpression obj(QStringLiteral(R"((?:^|\n)(\d+) 0 obj\n([\s\S]*?)\nendobj)"));
		auto it = obj.globalMatch(QString::fromLatin1(pdf));
		while (it.hasNext()) {
			const auto m = it.next();
			objs.insert(m.captured(1).toInt(), m.captured(2));
		}
		return objs;
	}

	static QList<int> refs(const QString &text)
	{
		static const QRegularExpression ref(QStringLiteral(R"((\d+) 0 R)"));
		QList<int> out;
		auto it = ref.globalMatch(text);
		while (it.hasNext()) out << it.next().captured(1).toInt();
		return out;
	}

	static QList<double> numbers(const QString &text)
	{
		QList<double> out;
		const QString plain = QString(text).replace(QLatin1Char(','), QLatin1Char(' '));
		for (const QString &n : plain.simplified().split(' ', Qt::SkipEmptyParts))
			out << n.toDouble();
		return out;
	}

	// A PDF literal string with its escaped brackets and backslashes undone.
	static QString unescaped(QString text)
	{
		text.replace(QLatin1String("\\("), QLatin1String("("));
		text.replace(QLatin1String("\\)"), QLatin1String(")"));
		text.replace(QLatin1String("\\\\"), QLatin1String("\\"));
		return text;
	}

private slots:
	void initTestCase()
	{
		QVERIFY(m_dir.isValid());
		QVERIFY(QFile::exists(QStringLiteral(QET_TEST_BINARY_PATH)));
		QVERIFY2(QFile::exists(project()), "examples/industrial.qet not found");
	}

	void everyJumpCallsHighlight()
	{
		const QByteArray pdf = exportPdf({}, QStringLiteral("marks"));
		QVERIFY2(!pdf.isEmpty(), "--export-pdf failed");
		const QMap<int, QString> objs = objects(pdf);

		// The pages in order, for the page index handed to highlight()
		static const QRegularExpression kidsRe(QStringLiteral(R"(/Type /Pages[\s\S]*?/Kids\s*\[([^\]]*)\])"));
		const QList<int> pages = refs(kidsRe.match(QString::fromLatin1(pdf)).captured(1));
		QVERIFY(pages.size() > 1);

		// Every link to an element: GoTo, then /Next highlight(index, rect)
		static const QRegularExpression linkRe(QStringLiteral(
			R"(/S /GoTo\n/D \[(\d+) 0 R /FitR ([-\d ]+)\]\n/Next << /Type /Action /S /JavaScript /JS \((highlight[\s\S]*?)\) >>)"));
		static const QRegularExpression callRe(QStringLiteral(
			R"(^highlight\((\d+), \[(-?\d+), (-?\d+), (-?\d+), (-?\d+)\]\);$)"));
		int calls = 0;
		for (auto it = objs.cbegin(); it != objs.cend(); ++it) {
			if (!it.value().contains(QLatin1String("/Subtype /Link"))) continue;
			const auto m = linkRe.match(it.value());
			if (!m.hasMatch()) continue;
			++calls;
			const int page = m.captured(1).toInt();
			const QList<double> fitr = numbers(m.captured(2));
			const QString call = unescaped(m.captured(3));
			const auto c = callRe.match(call);
			QVERIFY2(c.hasMatch(), qPrintable(call));

			// the page index is that of the page jumped to
			QCOMPARE(c.captured(1).toInt(), pages.indexOf(page));

			// the frame lies inside the zoomed rectangle
			const QList<double> rect = numbers(call.mid(call.indexOf('[') + 1,
														call.indexOf(']') - call.indexOf('[') - 1));
			QCOMPARE(rect.size(), 4);
			QCOMPARE(fitr.size(), 4);
			QVERIFY(rect[0] < rect[2] && rect[1] < rect[3]);
			QVERIFY2(rect[0] >= fitr[0] && rect[1] >= fitr[1]
					 && rect[2] <= fitr[2] && rect[3] <= fitr[3],
					 qPrintable(QStringLiteral("frame %1 outside the zoom %2")
								.arg(call, m.captured(2))));
		}
		QVERIFY2(calls >= 2, "expected several links which call highlight()");

		// The document JavaScript, under /Names /JavaScript of the catalog
		int catalog = -1;
		for (auto it = objs.cbegin(); it != objs.cend(); ++it)
			if (it.value().contains(QLatin1String("/Type /Catalog"))) catalog = it.key();
		QVERIFY(catalog != -1);
		static const QRegularExpression namesRe(QStringLiteral(R"(/Names (\d+) 0 R)"));
		const auto namesMatch = namesRe.match(objs.value(catalog));
		const QString names = namesMatch.hasMatch() ? objs.value(namesMatch.captured(1).toInt())
													 : objs.value(catalog);
		static const QRegularExpression scriptRe(QStringLiteral(
			R"(/JavaScript << /Names \[\(generalScript\) (\d+) 0 R\] >>)"));
		const auto scriptMatch = scriptRe.match(names);
		QVERIFY2(scriptMatch.hasMatch(), qPrintable(names));
		const QString script = unescaped(objs.value(scriptMatch.captured(1).toInt()));
		QVERIFY(script.contains(QLatin1String("/S /JavaScript")));
		QVERIFY(script.contains(QLatin1String("function highlight(page, coordinates)")));
		QVERIFY(script.contains(QLatin1String("addField('Target', 'button', page, coordinates)")));
		QVERIFY(script.contains(QLatin1String("f.strokeColor = color.red")));
		QVERIFY(script.contains(QLatin1String("app.setInterval('blinker()', 500)")));
		QVERIFY(script.contains(QLatin1String("app.setTimeOut('finish()', 5000)")));
		QVERIFY(script.contains(QLatin1String("removeField('Target')")));

		// nothing else: no form, no annotation waiting hidden on the pages
		QVERIFY(!pdf.contains("/AcroForm"));
		QVERIFY(!pdf.contains("/Subtype /Widget"));
		QVERIFY(!pdf.contains("/S /Hide"));
	}

	void pdfAHasNoFrames()
	{
		const QByteArray pdf = exportPdf({QStringLiteral("--pdf-a")}, QStringLiteral("pdfa"));
		QVERIFY2(!pdf.isEmpty(), "--export-pdf --pdf-a failed");
		QVERIFY(pdf.contains("pdfaid:part"));
		QVERIFY(pdf.contains("/S /GoTo"));         // still jumps
		QVERIFY(!pdf.contains("/JavaScript"));
		QVERIFY(!pdf.contains("/Next"));
		QVERIFY(!pdf.contains("highlight("));
	}
};

QTEST_MAIN(tst_pdfjumpmark)
#include "tst_pdfjumpmark.moc"
