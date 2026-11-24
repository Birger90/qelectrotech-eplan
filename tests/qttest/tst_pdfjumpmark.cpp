// SPDX-License-Identifier: GPL-2.0-or-later
/*
	--export-pdf marks the target of a jump as EPLAN does from 2.9 on: each
	link to an element leads on, after its GoTo, to a Hide action which
	hides every frame, a second which shows the frame of its target, and a
	JavaScript which lets that frame blink. The frame is a hidden read-only
	push button on the target page, inside the zoomed rectangle. A PDF/A
	export (--pdf-a) has no frames and no JavaScript, as in EPLAN.
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

		// The form and its push buttons
		int catalog = -1;
		for (auto it = objs.cbegin(); it != objs.cend(); ++it)
			if (it.value().contains(QLatin1String("/Type /Catalog"))) catalog = it.key();
		QVERIFY(catalog != -1);
		static const QRegularExpression fieldsRe(QStringLiteral(R"(/AcroForm << /Fields \[([^\]]*)\])"));
		const auto fieldsMatch = fieldsRe.match(objs.value(catalog));
		QVERIFY2(fieldsMatch.hasMatch(), "no /AcroForm in the catalog");
		const QList<int> fieldList = refs(fieldsMatch.captured(1));
		const QSet<int> fields(fieldList.cbegin(), fieldList.cend());
		QVERIFY(!fields.isEmpty());
		for (int w : fields) {
			const QString widget = objs.value(w);
			QVERIFY2(widget.contains(QLatin1String("/Subtype /Widget")), qPrintable(widget));
			QVERIFY(widget.contains(QLatin1String("/FT /Btn")));
			QVERIFY(widget.contains(QLatin1String("/F 2\n")));          // hidden
			QVERIFY(widget.contains(QLatin1String("/MK << /BC [1 0 0] >>")));  // red
			QVERIFY(widget.contains(QLatin1String("/T (qetmark")));
		}

		// Every link with a frame
		static const QRegularExpression linkRe(QStringLiteral(
			R"(/Subtype /Link[\s\S]*?/S /GoTo\n/D \[(\d+) 0 R /FitR ([-\d ]+)\]\n/Next \[(\d+) 0 R << /S /Hide /T (\d+) 0 R /H false >> << /S /JavaScript /JS \(([\s\S]*?)\) >>\])"));
		int chains = 0;
		int hideAll = -1;
		QSet<int> shown;
		for (auto it = objs.cbegin(); it != objs.cend(); ++it) {
			if (!it.value().contains(QLatin1String("/Subtype /Link"))) continue;
			const auto m = linkRe.match(it.value());
			if (!m.hasMatch()) continue;
			++chains;
			const int page = m.captured(1).toInt();
			const QList<double> fitr = numbers(m.captured(2));
			const int all = m.captured(3).toInt();
			const int widget = m.captured(4).toInt();
			QString js = m.captured(5);   // a PDF string: brackets escaped
			js.replace(QLatin1String("\\("), QLatin1String("("));
			js.replace(QLatin1String("\\)"), QLatin1String(")"));

			// one shared action hides every frame
			if (hideAll == -1) hideAll = all;
			QCOMPARE(all, hideAll);

			// the frame shown is on the page jumped to, inside the zoom
			QVERIFY(fields.contains(widget));
			const QString w = objs.value(widget);
			QVERIFY2(w.contains(QStringLiteral("/P %1 0 R").arg(page)), qPrintable(w));
			static const QRegularExpression rectRe(QStringLiteral(R"(/Rect \[([^\]]*)\])"));
			const QList<double> rect = numbers(rectRe.match(w).captured(1));
			QCOMPARE(rect.size(), 4);
			QCOMPARE(fitr.size(), 4);
			QVERIFY2(rect[0] >= fitr[0] && rect[1] >= fitr[1]
					 && rect[2] <= fitr[2] && rect[3] <= fitr[3],
					 qPrintable(QStringLiteral("frame %1 outside the zoom %2")
								.arg(w, m.captured(2))));

			// the frame is on that page's /Annots, before its links
			static const QRegularExpression annotsRe(QStringLiteral(R"(/Annots (\d+) 0 R)"));
			const int annots = annotsRe.match(objs.value(page)).captured(1).toInt();
			const QList<int> onPage = refs(objs.value(annots));
			QVERIFY(onPage.contains(widget));

			// the JavaScript blinks this frame and no other
			const QString name = QRegularExpression(QStringLiteral(R"(/T \((qetmark\d+)\))"))
									 .match(w).captured(1);
			QVERIFY(!name.isEmpty());
			QVERIFY(js.contains(QStringLiteral("getField('%1')").arg(name)));
			QVERIFY(js.contains(QLatin1String("app.setTimeOut")));
			QVERIFY(js.contains(QLatin1String("app.clearTimeOut")));
			shown.insert(widget);
		}
		QVERIFY2(chains >= 2, "expected several links with a frame");
		QCOMPARE(shown, fields);   // no frame which no link shows

		// the shared Hide action names every frame
		const QString hide = objs.value(hideAll);
		QVERIFY(hide.contains(QLatin1String("/S /Hide")));
		QVERIFY(hide.contains(QLatin1String("/H true")));
		const QList<int> hiddenList = refs(hide);
		QCOMPARE(QSet<int>(hiddenList.cbegin(), hiddenList.cend()), fields);
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
