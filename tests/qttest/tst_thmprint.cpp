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
// The THM label print (sources/marking/thm*): the bytes of a job, the
// replies to ESC s and ESC l, the choice of the printer by the article
// loaded, the printer list in the settings, the catalog file, and the
// client against a local QTcpServer standing in for the printer. Every
// value here is made up; nothing is sent to a real printer.

#include "marking/thmcatalog.h"
#include "marking/thmclient.h"
#include "marking/thmjob.h"
#include "marking/thmprinters.h"
#include "qettesttimeout.h"

#include <QElapsedTimer>
#include <QSettings>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTemporaryDir>
#include <QTest>

namespace {

	/// A printer on localhost: answers v, ESC s and ESC l as the manual
	/// describes, keeps whatever else it is sent
class FakePrinter : public QObject
{
	public:
		QByteArray version = "9.99 Jan 01, 2026 (TEST MODEL)\r";
		QByteArray status = "Y-000000Y";
		QByteArray material = "Y0037";
		bool silent = false;
		QByteArray received;   ///< everything sent, over all connections
		int connections = 0;
		int closed = 0;

		FakePrinter()
		{
			m_server.listen(QHostAddress::LocalHost, 0);
			connect(&m_server, &QTcpServer::newConnection, this, [this]() {
				while (QTcpSocket *s = m_server.nextPendingConnection()) {
					++connections;
					connect(s, &QTcpSocket::readyRead, this, [this, s]() {
						const QByteArray in = s->readAll();
						received += in;
						if (silent) return;
						if (in == "v\r") s->write(version);
						else if (in == "\x1bs") s->write(status);
						else if (in == "\x1bl") s->write(material);
					});
					connect(s, &QTcpSocket::disconnected, this, [this, s]() {
						++closed;
						s->deleteLater();
					});
				}
			});
		}
		quint16 port() const { return m_server.serverPort(); }

	private:
		QTcpServer m_server;
};

QString localHost()
{
	return QHostAddress(QHostAddress::LocalHost).toString();
}

ThmJob::Profile smallProfile()
{
	ThmJob::Profile p;
	p.width = 12;
	p.height = 6;
	p.gap = 2;
	p.sensor = ThmJob::Sensor::Gap;
	return p;
}

ThmPrinter printer(const QString &id, const QString &material)
{
	ThmPrinter p;
	p.id = id;
	p.name = QStringLiteral("Printer ") + id;
	p.host = localHost();
	p.material_no = material;
	p.profile = smallProfile();
	return p;
}

} // namespace

class TestThmPrint : public QObject
{
	Q_OBJECT

	private slots:
		void jobOneLabel()
		{
			QString error;
			const QByteArray job = ThmJob::build({{QStringLiteral("X1"), 3}}, smallProfile(), &error);
			QVERIFY2(error.isEmpty(), qPrintable(error));
			QCOMPARE(job, QByteArray("m m\rJ\rS l1;0,0,6,8,12\rT 0,4.5,0,3,3;[J:c12]X1\rA 3\r"));
		}

		void jobHeatRotateMargins()
		{
			ThmJob::Profile p = smallProfile();
			p.sensor = ThmJob::Sensor::Endless;
			p.gap = 0.5;
			p.margin_left = 1;
			p.margin_right = 1.5;
			p.offset_x = 0.25;
			p.text_size = 2.5;
			p.baseline = 5;
			p.font = 5;
			p.set_heat = true;
			p.speed = 50;
			p.heat = -2;
			p.transfer = true;
			p.rotate = true;
			const QByteArray job = ThmJob::build({{QStringLiteral("-K1"), 1},
												  {QStringLiteral("-K2"), 2}}, p);
			QCOMPARE(job, QByteArray(
						 "m m\rJ\rH 50,-2,T\rS e;0.25,0,6,6.5,12\rO R\rT 1,5,0,5,2.5;[J:c9.5]-K1\rA 1\r"
						 "m m\rJ\rH 50,-2,T\rS e;0.25,0,6,6.5,12\rO R\rT 1,5,0,5,2.5;[J:c9.5]-K2\rA 2\r"));
			p.transfer = false;
			QVERIFY(ThmJob::build({{QStringLiteral("A"), 1}}, p).contains("H 50,-2,D\r"));
		}

		void jobUtf8AndEscaping()
		{
			const QString text = QString::fromUtf8("Ü1[2]\nB");
			const QByteArray job = ThmJob::build({{text, 1}}, smallProfile());
			QVERIFY(job.startsWith("\x1boUTF-8;m m\r"));
			QVERIFY(job.contains(QString::fromUtf8("[J:c12]Ü1[U:$5B]2] B\r").toUtf8()));
				//Only the block with non-ASCII text gets the codepage
			const QByteArray two = ThmJob::build({{QStringLiteral("A"), 1}, {QString::fromUtf8("µ"), 1}},
												 smallProfile());
			QCOMPARE(two.count("\x1boUTF-8;"), 1);
			QVERIFY(two.startsWith("m m\r"));
		}

		void jobColumns()
		{
			ThmJob::Profile p = smallProfile();
			p.columns = 3;
			p.column_pitch = 15;
			const QByteArray job = ThmJob::build({{QStringLiteral("A"), 2},
												  {QStringLiteral("B"), 1},
												  {QStringLiteral("C"), 4}}, p);
			const QByteArray size = "S l1;0,0,6,8,42\r";
			QCOMPARE(job, QByteArray(
						 "m m\rJ\r" + size +
						 "T 0,4.5,0,3,3;[J:c12]A\rT 15,4.5,0,3,3;[J:c12]A\rT 30,4.5,0,3,3;[J:c12]B\rA 1\r"
						 "m m\rJ\r" + size +
						 "T 0,4.5,0,3,3;[J:c12]C\rT 15,4.5,0,3,3;[J:c12]C\rT 30,4.5,0,3,3;[J:c12]C\rA 1\r"
						 "m m\rJ\r" + size +
						 "T 0,4.5,0,3,3;[J:c12]C\rA 1\r"));
				//Equal rows in a row are one block printed twice
			const QByteArray same = ThmJob::build({{QStringLiteral("D"), 6}}, p);
			QCOMPARE(same.count("J\r"), 1);
			QVERIFY(same.endsWith("A 2\r"));
		}

		void jobRefused()
		{
			QString error;
			ThmJob::Profile p;
			QVERIFY(ThmJob::build({{QStringLiteral("X"), 1}}, p, &error).isEmpty());
			QVERIFY(!error.isEmpty());
			p = smallProfile();
			p.margin_left = 6;
			p.margin_right = 6;
			QVERIFY(!ThmJob::validate(p, &error));
			p = smallProfile();
			p.set_heat = true;
			p.speed = 50;
			p.heat = 21;
			QVERIFY(!ThmJob::validate(p, &error));
			QVERIFY(ThmJob::build({}, smallProfile(), &error).isEmpty());
			QVERIFY(!error.isEmpty());
		}

		void numbers()
		{
			QCOMPARE(ThmJob::number(6), QStringLiteral("6"));
			QCOMPARE(ThmJob::number(6.5), QStringLiteral("6.5"));
			QCOMPARE(ThmJob::number(0.126), QStringLiteral("0.13"));
			QCOMPARE(ThmJob::number(-0.001), QStringLiteral("0"));
		}

		void labelsMerged()
		{
			QList<MarkingExport::Item> items;
			MarkingExport::Item a;
			a.text = QStringLiteral("101");
			items << a << a;   //both ends of one wire
			MarkingExport::Item b;
			b.text = QStringLiteral("W1");
			b.count = 2;       //a cable: two ends in one item
			items << b;
			MarkingExport::Item empty;
			empty.text = QStringLiteral("  ");
			items << empty << a;
			const QList<ThmJob::Label> merged = ThmJob::toLabels(items, 2, true);
			QCOMPARE(merged.size(), 2);
			QCOMPARE(merged.at(0).text, QStringLiteral("101"));
			QCOMPARE(merged.at(0).count, 6);
			QCOMPARE(merged.at(1).count, 4);
			QCOMPARE(ThmJob::labelCount(merged), 10);
			const QList<ThmJob::Label> single = ThmJob::toLabels(items, 1, false);
			QCOMPARE(single.size(), 4);
			QCOMPARE(ThmJob::labelCount(single), 5);
		}

		void statusReplies()
		{
			ThmJob::Status s = ThmJob::parseStatus("Y-000000Y");
			QVERIFY(s.valid);
			QVERIFY(s.online);
			QVERIFY(!s.hasError());
			QCOMPARE(s.pending, 0);
			QVERIFY(s.interpreter_active);

			s = ThmJob::parseStatus("NE000012N\r\n");
			QVERIFY(s.valid);
			QVERIFY(!s.online);
			QVERIFY(s.hasError());
			QCOMPARE(s.error, 'E');
			QCOMPARE(s.pending, 12);
			QVERIFY(!s.interpreter_active);
			QVERIFY(ThmJob::statusText(s).contains(ThmJob::errorText('E')));

			QCOMPARE(ThmJob::parseStatus("YF000001Y").error, 'F');
			QVERIFY(ThmJob::errorText('?').contains(QLatin1Char('?')));
			QVERIFY(!ThmJob::parseStatus("Y-00a000Y").valid);
			QVERIFY(!ThmJob::parseStatus("Y-000000").valid);
			QVERIFY(!ThmJob::parseStatus("X-000000Y").valid);
			QVERIFY(!ThmJob::parseStatus("Y#000000Y").valid);
			QVERIFY(!ThmJob::parseStatus("").valid);
		}

		void materialReplies()
		{
			ThmJob::Material m = ThmJob::parseMaterial("Y0037");
			QVERIFY(m.valid);
			QVERIFY(m.synchronized);
			QCOMPARE(m.pitch, 37);
			m = ThmJob::parseMaterial("N0000\r");
			QVERIFY(m.valid);
			QVERIFY(!m.synchronized);
			QCOMPARE(m.pitch, 0);
			QVERIFY(!ThmJob::parseMaterial("Y00x7").valid);
			QVERIFY(!ThmJob::parseMaterial("Y037").valid);
			QVERIFY(!ThmJob::parseMaterial("A0037").valid);

			ThmJob::Profile p = smallProfile();
			p.height = 35;
			p.gap = 2;
			QCOMPARE(ThmJob::materialFit(ThmJob::parseMaterial("Y0037"), p), ThmJob::Fit::Fits);
			QCOMPARE(ThmJob::materialFit(ThmJob::parseMaterial("Y0038"), p), ThmJob::Fit::Fits);
			QCOMPARE(ThmJob::materialFit(ThmJob::parseMaterial("Y0039"), p), ThmJob::Fit::Differs);
			QCOMPARE(ThmJob::materialFit(ThmJob::parseMaterial("Y0000"), p), ThmJob::Fit::Unknown);
		}

		void choice()
		{
			QList<ThmPrinter> list;
			QCOMPARE(ThmPrinters::choose(list, QStringLiteral("BSP-1")).outcome,
					 ThmPrinters::Choice::NoPrinters);
			list << printer(QStringLiteral("a"), QStringLiteral("BSP-1"))
				 << printer(QStringLiteral("b"), QStringLiteral("bsp - 2"))
				 << printer(QStringLiteral("c"), QStringLiteral("BSP-2"));
			QCOMPARE(ThmPrinters::choose(list, QString()).outcome, ThmPrinters::Choice::NoMaterial);
			ThmPrinters::Choice c = ThmPrinters::choose(list, QStringLiteral("BSP-3"));
			QCOMPARE(c.outcome, ThmPrinters::Choice::NoMatch);
			QVERIFY(c.message.contains(QStringLiteral("BSP-3")));
			c = ThmPrinters::choose(list, QStringLiteral(" bsp-1 "));
			QCOMPARE(c.outcome, ThmPrinters::Choice::Unique);
			QCOMPARE(c.matches, QList<int>({0}));
			c = ThmPrinters::choose(list, QStringLiteral("BSP-2"));
			QCOMPARE(c.outcome, ThmPrinters::Choice::Several);
			QCOMPARE(c.matches, QList<int>({1, 2}));
		}

		void printerSettings()
		{
			QTemporaryDir dir;
			QVERIFY(dir.isValid());
			QList<ThmPrinter> list;
			ThmPrinter p = printer(QStringLiteral("thm1"), QStringLiteral("BSP-1001"));
			p.port = 9101;
			p.model = QStringLiteral("TEST MODEL");
			p.profile.columns = 4;
			p.profile.column_pitch = 12.5;
			p.profile.sensor = ThmJob::Sensor::ReflectTop;
			p.profile.set_heat = true;
			p.profile.speed = 40;
			p.profile.heat = 3;
			p.profile.rotate = true;
			list << p;
			ThmPrinter q = printer(ThmPrinters::newId(list), QStringLiteral("BSP-1002"));
			QCOMPARE(q.id, QStringLiteral("thm2"));
			list << q;
			{
				QSettings s(dir.filePath(QStringLiteral("thm.ini")), QSettings::IniFormat);
				ThmPrinters::save(s, list);
				ThmPrinters::setCatalogFile(s, QStringLiteral("/somewhere/my_cat_x.mca"));
			}
			QSettings s(dir.filePath(QStringLiteral("thm.ini")), QSettings::IniFormat);
			const QList<ThmPrinter> back = ThmPrinters::load(s);
			QCOMPARE(back.size(), 2);
			QCOMPARE(back.at(0).port, quint16(9101));
			QCOMPARE(back.at(0).model, QStringLiteral("TEST MODEL"));
			QCOMPARE(back.at(0).profile.columns, 4);
			QCOMPARE(back.at(0).profile.column_pitch, 12.5);
			QCOMPARE(back.at(0).profile.sensor, ThmJob::Sensor::ReflectTop);
			QVERIFY(back.at(0).profile.rotate);
			QCOMPARE(ThmJob::build({{QStringLiteral("Z"), 5}}, back.at(0).profile),
					 ThmJob::build({{QStringLiteral("Z"), 5}}, p.profile));
			QCOMPARE(back.at(1).material_no, QStringLiteral("BSP-1002"));
			QCOMPARE(ThmPrinters::indexOf(back, QStringLiteral("THM2")), 1);
			QCOMPARE(ThmPrinters::catalogFile(s), QStringLiteral("/somewhere/my_cat_x.mca"));
				//Saving fewer printers leaves none of the old behind
			ThmPrinters::save(s, {back.at(1)});
			QCOMPARE(ThmPrinters::load(s).size(), 1);

			QString error;
			QVERIFY(ThmPrinters::validate(back.at(0), &error));
			ThmPrinter no_host = back.at(0);
			no_host.host.clear();
			QVERIFY(!ThmPrinters::validate(no_host, &error));
			ThmPrinter no_material = back.at(0);
			no_material.material_no = QStringLiteral("  ");
			QVERIFY(!ThmPrinters::validate(no_material, &error));
		}

		void catalog()
		{
			QString error;
			const QList<ThmCatalog::Variant> v = ThmCatalog::read(
						QFINDTESTDATA("fixtures/thm_catalog_example.mca"), &error);
			QVERIFY2(error.isEmpty(), qPrintable(error));
			QCOMPARE(v.size(), 3);
			QCOMPARE(v.at(0).material_no, QStringLiteral("BSP-1001"));
			QCOMPARE(v.at(0).name, QStringLiteral("Example 12 x 6"));
			QCOMPARE(v.at(1).description, QStringLiteral("Example card, 17 x 9 mm"));
			QCOMPARE(v.at(2).material_no, QStringLiteral("BSP-1003"));
			QCOMPARE(v.at(2).name, QString::fromUtf8("Example sleeve ÄÖÜ"));
			QCOMPARE(ThmCatalog::title(v.at(0)), QString::fromUtf8("BSP-1001 – Example 12 x 6"));

			QVERIFY(ThmCatalog::parse("<other/>", &error).isEmpty());
			QVERIFY(!error.isEmpty());
			QVERIFY(ThmCatalog::parse("<multiprint_catalog><cat_item title=\"X\"/></multiprint_catalog>", &error).isEmpty());
			QVERIFY(error.contains(QStringLiteral("MultiMark")));
			QVERIFY(ThmCatalog::parse("<multiprint_catalog><cat_item", &error).isEmpty());
			QVERIFY(!error.isEmpty());
			QVERIFY(ThmCatalog::read(QStringLiteral("/no/such/file.mca"), &error).isEmpty());
		}

		void clientQueries()
		{
			FakePrinter fake;
			ThmClient client;
			client.setTarget(localHost(), fake.port());
			client.setTimeouts(QetTest::timeout(2000), QetTest::timeout(2000));

			ThmClient::Reply r = client.run(ThmClient::Request::Version);
			QVERIFY2(r.ok, qPrintable(r.error));
			QCOMPARE(ThmJob::parseVersion(r.data), QStringLiteral("9.99 Jan 01, 2026 (TEST MODEL)"));

			r = client.run(ThmClient::Request::Status);
			QVERIFY2(r.ok, qPrintable(r.error));
			QCOMPARE(r.data, QByteArray("Y-000000Y"));
			QVERIFY(ThmJob::parseStatus(r.data).online);

			r = client.run(ThmClient::Request::Material);
			QVERIFY2(r.ok, qPrintable(r.error));
			QCOMPARE(ThmJob::parseMaterial(r.data).pitch, 37);

			fake.status = "NE000002N\r\n";
			r = client.run(ThmClient::Request::Status);
			QVERIFY(r.ok);
			QCOMPARE(ThmJob::parseStatus(r.data).error, 'E');
			QCOMPARE(fake.received, QByteArray("v\r\x1bs\x1bl\x1bs"));
			QCOMPARE(fake.connections, 4);
		}

		void clientSends()
		{
			FakePrinter fake;
			ThmClient client;
			client.setTarget(localHost(), fake.port());
			client.setTimeouts(QetTest::timeout(2000), QetTest::timeout(2000));
			client.setPauses(0, 0);
			const QByteArray job = ThmJob::build({{QStringLiteral("101"), 2}}, smallProfile());
			ThmClient::Reply r = client.run(ThmClient::Request::Job, job);
			QVERIFY2(r.ok, qPrintable(r.error));
			QTRY_COMPARE_WITH_TIMEOUT(fake.received, job, QetTest::timeout(2000));
			QTRY_COMPARE_WITH_TIMEOUT(fake.closed, 1, QetTest::timeout(2000));
			fake.received.clear();
			QVERIFY(client.run(ThmClient::Request::Pause).ok);
			QVERIFY(client.run(ThmClient::Request::Resume).ok);
			QVERIFY(client.run(ThmClient::Request::Cancel).ok);
			QTRY_COMPARE_WITH_TIMEOUT(fake.received, QByteArray("\x1bp1\x1bp0\x1bt"),
									  QetTest::timeout(2000));
		}

		void clientWaitsBetweenStatusQueries()
		{
			FakePrinter fake;
			ThmClient client;
			client.setTarget(localHost(), fake.port());
			client.setPauses(300, 400);
			QVERIFY(client.run(ThmClient::Request::Status).ok);
			QElapsedTimer t;
			t.start();
			QVERIFY(client.run(ThmClient::Request::Status).ok);
			QVERIFY2(t.elapsed() >= 250, qPrintable(QString::number(t.elapsed())));
			QVERIFY(client.run(ThmClient::Request::Cancel).ok);
			t.start();
			QVERIFY(client.run(ThmClient::Request::Material).ok);
			QVERIFY2(t.elapsed() >= 350, qPrintable(QString::number(t.elapsed())));
		}

		void clientErrors()
		{
			ThmClient client;
			client.setTimeouts(QetTest::timeout(2000), 300);
			ThmClient::Reply r = client.run(ThmClient::Request::Status);
			QVERIFY(!r.ok);
			QVERIFY(!r.error.isEmpty());

				//A port nobody listens on: refused at once
			quint16 closed_port = 0;
			{
				QTcpServer probe;
				QVERIFY(probe.listen(QHostAddress::LocalHost, 0));
				closed_port = probe.serverPort();
			}
			client.setTarget(localHost(), closed_port);
			r = client.run(ThmClient::Request::Status);
			QVERIFY(!r.ok);
			QVERIFY2(r.error.contains(QString::number(closed_port)), qPrintable(r.error));

				//Connected, but no answer: the reply time limit
			FakePrinter fake;
			fake.silent = true;
			client.setTarget(localHost(), fake.port());
			QElapsedTimer t;
			t.start();
			r = client.run(ThmClient::Request::Status);
			QVERIFY(!r.ok);
			QVERIFY(t.elapsed() < QetTest::timeout(3000));
			QVERIFY(!r.error.isEmpty());

				//An incomplete answer is reported with what came
			fake.silent = false;
			fake.material = "Y00";
			r = client.run(ThmClient::Request::Material);
			QVERIFY(!r.ok);
			QVERIFY2(r.error.contains(QStringLiteral("Y00")), qPrintable(r.error));

				//One request at a time
			QVERIFY(client.start(ThmClient::Request::Status));
			QVERIFY(!client.start(ThmClient::Request::Status));
			QTRY_VERIFY_WITH_TIMEOUT(!client.busy(), QetTest::timeout(3000));
		}
};

QTEST_GUILESS_MAIN(TestThmPrint)
#include "tst_thmprint.moc"
