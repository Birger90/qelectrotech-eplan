// SPDX-License-Identifier: GPL-2.0-or-later
/*
	--export-pdf marks the target of a jump as EPLAN does from 2.9 on, in a
	way Chrome's and Edge's viewer (PDFium) show as well as Acrobat: each
	target has a hidden red frame, a read-only button, and an invisible push
	button lies over each link, with the same actions as the link: GoTo,
	then a Hide action which hides every frame, one which shows the frame of
	the target (both by field name), and a JavaScript which lets it blink
	and hides it after 5 s. PDFium runs only the GoTo of a link but every
	action of a button, and finds a field of a Hide action by name only.
	The frame lies inside the zoomed rectangle. A PDF/A export (--pdf-a)
	has no frames and no JavaScript, as in EPLAN.
*/
#include <QDir>
#include <QFile>
#include <QMap>
#include <QProcess>
#include <QRegularExpression>
#include <QSet>
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
		for (const QString &n : text.simplified().split(' ', Qt::SkipEmptyParts))
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

	void everyJumpShowsItsFrame()
	{
		const QByteArray pdf = exportPdf({}, QStringLiteral("marks"));
		QVERIFY2(!pdf.isEmpty(), "--export-pdf failed");
		const QMap<int, QString> objs = objects(pdf);

		// The form: the frames (hidden, red, read-only) and the buttons
		int catalog = -1;
		for (auto it = objs.cbegin(); it != objs.cend(); ++it)
			if (it.value().contains(QLatin1String("/Type /Catalog"))) catalog = it.key();
		QVERIFY(catalog != -1);
		static const QRegularExpression fieldsRe(QStringLiteral(R"(/AcroForm << /Fields \[([^\]]*)\])"));
		const auto fieldsMatch = fieldsRe.match(objs.value(catalog));
		QVERIFY2(fieldsMatch.hasMatch(), "no /AcroForm in the catalog");
		static const QRegularExpression nameRe(QStringLiteral(R"(/T \((qet(?:mark|jump)\d+)\))"));
		static const QRegularExpression rectRe(QStringLiteral(R"(/Rect \[([^\]]*)\])"));
		QMap<QString, int> frameOf;            // name -> object
		QList<int> buttons;
		for (int w : refs(fieldsMatch.captured(1))) {
			const QString widget = objs.value(w);
			QVERIFY2(widget.contains(QLatin1String("/Subtype /Widget")), qPrintable(widget));
			QVERIFY(widget.contains(QLatin1String("/FT /Btn")));
			const QString name = nameRe.match(widget).captured(1);
			if (name.startsWith(QLatin1String("qetmark"))) {
				QVERIFY(widget.contains(QLatin1String("/Ff 65537\n")));     // read-only
				QVERIFY(widget.contains(QLatin1String("/F 2\n")));          // hidden
				QVERIFY(widget.contains(QLatin1String("/MK << /BC [1 0 0] >>")));  // red
				frameOf.insert(name, w);
			} else {
				QVERIFY2(name.startsWith(QLatin1String("qetjump")), qPrintable(widget));
				QVERIFY(widget.contains(QLatin1String("/Ff 65536\n")));     // push button
				// jumps on press: PDFium hands a release to the button
				// focused last, which swallowed the next jump
				QVERIFY2(widget.contains(QLatin1String("/AA << /D <<")), qPrintable(widget));
				QVERIFY(!widget.contains(QLatin1String("\n/A <<")));
				buttons << w;
			}
		}
		QVERIFY(!frameOf.isEmpty());

		// The annotations of each page, in order
		static const QRegularExpression annotsRe(QStringLiteral(R"(/Annots (\d+) 0 R)"));
		QMap<int, QList<int>> annotsOfPage;    // page object -> its annotations
		for (auto it = objs.cbegin(); it != objs.cend(); ++it)
			if (it.value().contains(QLatin1String("/Type /Page\n")))
				annotsOfPage.insert(it.key(), refs(objs.value(annotsRe.match(it.value()).captured(1).toInt())));

		// Every link with a frame: GoTo -> hide all -> show its frame by
		// name -> JavaScript
		static const QRegularExpression linkRe(QStringLiteral(
			R"(/S /GoTo\n/D \[(\d+) 0 R /FitR ([-\d ]+)\]\n/Next \[(\d+) 0 R << /S /Hide /T \((qetmark\d+)\) /H false >> << /S /JavaScript /JS \(([\s\S]*?)\) >>\])"));
		int chains = 0;
		int hideAll = -1;
		QSet<QString> shown;
		QSet<int> buttonsUsed;
		for (auto it = objs.cbegin(); it != objs.cend(); ++it) {
			if (!it.value().contains(QLatin1String("/Subtype /Link"))) continue;
			const auto m = linkRe.match(it.value());
			if (!m.hasMatch()) continue;
			++chains;
			const int page = m.captured(1).toInt();
			const QList<double> fitr = numbers(m.captured(2));
			const int all = m.captured(3).toInt();
			const QString name = m.captured(4);
			const QString js = unescaped(m.captured(5));

			// one shared action hides every frame
			if (hideAll == -1) hideAll = all;
			QCOMPARE(all, hideAll);

			// the frame shown is on the page jumped to, inside the zoom
			QVERIFY2(frameOf.contains(name), qPrintable(name));
			const int widget = frameOf.value(name);
			const QString w = objs.value(widget);
			QVERIFY2(w.contains(QStringLiteral("/P %1 0 R").arg(page)), qPrintable(w));
			const QList<double> rect = numbers(rectRe.match(w).captured(1));
			QCOMPARE(rect.size(), 4);
			QCOMPARE(fitr.size(), 4);
			QVERIFY2(rect[0] >= fitr[0] && rect[1] >= fitr[1]
					 && rect[2] <= fitr[2] && rect[3] <= fitr[3],
					 qPrintable(QStringLiteral("frame %1 outside the zoom %2")
								.arg(w, m.captured(2))));
			QVERIFY(annotsOfPage.value(page).contains(widget));

			// the JavaScript shows this frame, blinks it, hides it after 5 s
			const QString field = QStringLiteral("this.getField('%1')").arg(name);
			QVERIFY2(js.contains(field + QLatin1String(".display = display.visible;")), qPrintable(js));
			QVERIFY(js.contains(QLatin1String("app.setInterval(")));
			QVERIFY(js.contains(QLatin1String(", 500);")));
			QVERIFY(js.contains(QLatin1String("app.setTimeOut(")));
			QVERIFY(js.contains(field + QLatin1String(".display = display.hidden;\", 5000);")));
			QVERIFY(!js.contains(QLatin1String("addField")));   // PDFium does not run it
			shown.insert(name);

			// a button with the same rectangle and actions lies on top of the
			// link, on the link's page
			const QString linkRect = rectRe.match(it.value()).captured(1).simplified();
			int button = -1;
			for (int b : std::as_const(buttons))
				if (!buttonsUsed.contains(b)
					&& rectRe.match(objs.value(b)).captured(1).simplified() == linkRect
					&& objs.value(b).contains(m.captured(0))) {
					button = b;
					break;
				}
			QVERIFY2(button != -1, qPrintable(QStringLiteral("no button over link %1").arg(it.key())));
			buttonsUsed.insert(button);
			bool onTop = false;
			for (auto p = annotsOfPage.cbegin(); p != annotsOfPage.cend(); ++p) {
				const int l = p.value().indexOf(it.key());
				if (l == -1) continue;
				onTop = p.value().indexOf(button) > l
					&& objs.value(button).contains(QStringLiteral("/P %1 0 R").arg(p.key()));
			}
			QVERIFY2(onTop, qPrintable(QStringLiteral("button %1 not on top of link %2").arg(button).arg(it.key())));
		}
		QVERIFY2(chains >= 2, "expected several links with a frame");
		QCOMPARE(shown, QSet<QString>(frameOf.keyBegin(), frameOf.keyEnd()));   // no frame which no link shows
		QCOMPARE(buttonsUsed.size(), buttons.size());                           // no button without its link

		// the shared Hide action names every frame
		const QString hide = objs.value(hideAll);
		QVERIFY(hide.contains(QLatin1String("/S /Hide")));
		QVERIFY(hide.contains(QLatin1String("/H true")));
		QSet<QString> hidden;
		static const QRegularExpression stringRe(QStringLiteral(R"(\((qetmark\d+)\))"));
		auto names = stringRe.globalMatch(hide);
		while (names.hasNext()) hidden.insert(names.next().captured(1));
		QCOMPARE(hidden, shown);
	}

	void pdfAHasNoFrames()
	{
		const QByteArray pdf = exportPdf({QStringLiteral("--pdf-a")}, QStringLiteral("pdfa"));
		QVERIFY2(!pdf.isEmpty(), "--export-pdf --pdf-a failed");
		QVERIFY(pdf.contains("pdfaid:part"));
		QVERIFY(pdf.contains("/S /GoTo"));         // still jumps
		QVERIFY(!pdf.contains("/Subtype /Widget"));
		QVERIFY(!pdf.contains("/JavaScript"));
		QVERIFY(!pdf.contains("/Next"));
		QVERIFY(!pdf.contains("/AcroForm"));
	}
};

QTEST_MAIN(tst_pdfjumpmark)
#include "tst_pdfjumpmark.moc"
