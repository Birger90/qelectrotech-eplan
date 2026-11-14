// SPDX-License-Identifier: GPL-2.0-or-later
#include <QtTest>

#include <QDomDocument>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QProcessEnvironment>
#include <QTemporaryDir>
#include "qettesttimeout.h"

// A placed cylinder can have its own bore, rod and stroke in place of those
// of its symbol. They are a <fluidOverride> child of the element, written
// only when set, and the project database holds them (the element_value
// rows), whether the folio was built or read from the document. A cylinder
// without one, as in every project made before, saves as it was.
//
// Runs the real binary on a small project written here: one cylinder with
// its own values, one without.
namespace {

const QString uuid_own = QStringLiteral("{11111111-1111-4111-8111-111111111111}");
const QString uuid_plain = QStringLiteral("{22222222-2222-4222-8222-222222222222}");

QString definition()
{
	return QStringLiteral(
				"<collection><category name=\"import\"><category name=\"t\">"
				"<element name=\"cylinder.elmt\">"
				"<definition width=\"30\" hotspot_y=\"21\" version=\"0.100.0\" type=\"element\""
				" link_type=\"simple\" hotspot_x=\"15\" height=\"42\">"
				"<uuid uuid=\"{33333333-3333-4333-8333-333333333333}\"/>"
				"<names><name lang=\"en\">cylinder</name></names>"
				"<informations/>"
				"<fluid kind=\"cylinder\"><cylinder action=\"double\" bore_mm=\"32\""
				" rod_mm=\"12\" stroke_mm=\"100\"/></fluid>"
				"<description><rect x=\"-5\" y=\"-6\" width=\"10\" height=\"12\" antialias=\"true\""
				" style=\"line-style:normal;line-weight:normal;filling:none;color:black\"/>"
				"</description></definition></element></category></category></collection>");
}

QString placed(const QString &uuid, int x, const QString &own)
{
	return QStringLiteral(
				"<element type=\"embed://import/t/cylinder.elmt\" uuid=\"%1\" x=\"%2\" y=\"100\" z=\"10\""
				" orientation=\"0\"><terminals/><inputs/>%3</element>").arg(uuid).arg(x).arg(own);
}

QString project(const QString &own)
{
	return QStringLiteral(
				"<project title=\"fluid\" version=\"0.100.1\">"
				"<diagram title=\"f\" cols=\"8\" rows=\"4\" colsize=\"60\" rowsize=\"80\">"
				"<elements>%1%2</elements><conductors/></diagram>%3</project>")
			.arg(placed(uuid_own, 100, own), placed(uuid_plain, 300, QString()), definition());
}

const QString own_values = QStringLiteral(
			"<fluidOverride bore_mm=\"50\" stroke_mm=\"250.5\"/>");

} // namespace

class tst_fluidoverride : public QObject
{
	Q_OBJECT

	QTemporaryDir m_dir;
	int m_run = 0;

	QProcessEnvironment environment(bool eager)
	{
		const QString home = m_dir.filePath(QStringLiteral("home%1").arg(m_run++));
		QDir().mkpath(home);
		QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
		env.insert(QStringLiteral("QT_QPA_PLATFORM"), QStringLiteral("offscreen"));
		env.insert(QStringLiteral("QET_ENABLE_SCRIPTING"), QStringLiteral("1"));
		env.insert(QStringLiteral("HOME"), home);
		env.insert(QStringLiteral("XDG_CONFIG_HOME"), home + QStringLiteral("/config"));
		env.insert(QStringLiteral("XDG_DATA_HOME"), home + QStringLiteral("/data"));
		if (eager) env.insert(QStringLiteral("QET_EAGER_FOLIOS"), QStringLiteral("1"));
		else env.remove(QStringLiteral("QET_EAGER_FOLIOS"));
		return env;
	}

	QString writeProject(const QString &own)
	{
		const QString in = m_dir.filePath(QStringLiteral("in%1.qet").arg(m_run));
		QFile f(in);
		if (!f.open(QIODevice::WriteOnly)) return {};
		f.write(project(own).toUtf8());
		return in;
	}

	// The placed element @p uuid after the project is opened and saved
	// again, or a null element
	QDomElement savedElement(const QString &own, bool eager, const QString &uuid)
	{
		const QString in = writeProject(own);
		const QString out = m_dir.filePath(QStringLiteral("out%1.qet").arg(m_run));
		QProcess proc;
		proc.setProcessEnvironment(environment(eager));
		proc.start(QStringLiteral(QET_TEST_BINARY_PATH), {QStringLiteral("--resave"), in, out});
		if (!proc.waitForFinished(QetTest::timeout(120000))) return {};
		QFile f(out);
		if (!f.open(QIODevice::ReadOnly)) return {};
		static QDomDocument document;
		if (!document.setContent(&f)) return {};
		const QDomNodeList elements = document.elementsByTagName(QStringLiteral("element"));
		for (int i = 0 ; i < elements.size() ; ++i) {
			const QDomElement element = elements.at(i).toElement();
			if (element.attribute(QStringLiteral("uuid")) == uuid)
				return element;
		}
		return {};
	}

	// The element_value rows of the cylinders' own values, as
	// "uuid|name|value"
	QStringList databaseRows(const QString &own, bool eager)
	{
		const QString in = writeProject(own);
		const QString js = m_dir.filePath(QStringLiteral("script%1.js").arg(m_run));
		QFile s(js);
		if (!s.open(QIODevice::WriteOnly)) return {};
		s.write("qet.log('ROWS ' + JSON.stringify(qet.query("
				"\"SELECT element_uuid, name, value FROM element_value "
				"WHERE section LIKE 'fluidOverride%' ORDER BY element_uuid, name\")));");
		s.close();
		QProcess proc;
		proc.setProcessEnvironment(environment(eager));
		proc.start(QStringLiteral(QET_TEST_BINARY_PATH), {QStringLiteral("--run"), js, in});
		if (!proc.waitForFinished(QetTest::timeout(120000))) return {};
		const QString out = QString::fromUtf8(proc.readAllStandardOutput() + proc.readAllStandardError());
		QStringList rows;
		for (const QString &line : out.split(QLatin1Char('\n'))) {
			const int i = line.indexOf(QStringLiteral("ROWS "));
			if (i < 0) continue;
			for (const QJsonValue &v : QJsonDocument::fromJson(line.mid(i + 5).toUtf8()).array()) {
				const QJsonObject o = v.toObject();
				rows << o.value(QStringLiteral("element_uuid")).toString() + QLatin1Char('|')
						+ o.value(QStringLiteral("name")).toString() + QLatin1Char('|')
						+ o.value(QStringLiteral("value")).toString();
			}
		}
		return rows;
	}

private slots:
	void initTestCase()
	{
		QVERIFY(m_dir.isValid());
		QVERIFY(QFile::exists(QStringLiteral(QET_TEST_BINARY_PATH)));
	}

	void valuesAreSaved_data()
	{
		QTest::addColumn<bool>("eager");
		QTest::newRow("built") << true;
		QTest::newRow("from the document") << false;
	}

	// What the cylinder was given is saved again as it was
	void valuesAreSaved()
	{
		QFETCH(bool, eager);
		const QDomElement element = savedElement(own_values, eager, uuid_own);
		QVERIFY(!element.isNull());
		const QDomElement own = element.firstChildElement(QStringLiteral("fluidOverride"));
		QVERIFY(!own.isNull());
		QCOMPARE(own.attribute(QStringLiteral("bore_mm")).toDouble(), 50.0);
		QCOMPARE(own.attribute(QStringLiteral("stroke_mm")).toDouble(), 250.5);
		QVERIFY(!own.hasAttribute(QStringLiteral("rod_mm")));
	}

	void nothingIsWrittenWhenUnset_data()
	{
		valuesAreSaved_data();
	}

	// A cylinder with the values of its symbol saves exactly as before
	void nothingIsWrittenWhenUnset()
	{
		QFETCH(bool, eager);
		const QDomElement element = savedElement(own_values, eager, uuid_plain);
		QVERIFY(!element.isNull());
		QVERIFY(element.firstChildElement(QStringLiteral("fluidOverride")).isNull());
	}

	// Values of 0 are no values: nothing is kept
	void zeroIsDropped()
	{
		const QDomElement element = savedElement(
					QStringLiteral("<fluidOverride bore_mm=\"0\"/>"), true, uuid_own);
		QVERIFY(!element.isNull());
		QVERIFY(element.firstChildElement(QStringLiteral("fluidOverride")).isNull());
	}

	// The project database holds them, built or not
	void databaseHoldsTheValues()
	{
		const QStringList built = databaseRows(own_values, true);
		QVERIFY(!built.isEmpty());
		const QString joined = built.join(QLatin1Char(';'));
		QVERIFY2(joined.contains(uuid_own + QStringLiteral("|bore_mm|50")),
				 qPrintable(built.join(QLatin1Char('\n'))));
		QVERIFY2(joined.contains(uuid_own + QStringLiteral("|stroke_mm|250.5")),
				 qPrintable(built.join(QLatin1Char('\n'))));
		for (const QString &row : built)
			QVERIFY2(!row.startsWith(uuid_plain), qPrintable(row));
		QCOMPARE(databaseRows(own_values, false), built);
	}
};

QTEST_GUILESS_MAIN(tst_fluidoverride)
#include "tst_fluidoverride.moc"
