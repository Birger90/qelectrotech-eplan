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
// The client of the EPLAN Data Portal against a local HTTP stub: the
// search reply is read, the token goes as "Bearer PAT:<token>", the DXF
// package arrives with its file name, and a wrong token, a reached limit,
// a part without DXF and an unreachable server give their own message.
// The data is made up; it only has the shape of the Data Portal's replies.

#include "import/eplandxf/dataportalclient.h"

#include <QSignalSpy>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTest>
#include <QUrlQuery>

namespace {
	const QByteArray PARTS = R"({
		"meta": {"page": {"offset": 0, "limit": 50, "total": 120}},
		"data": [
			{"type": "parts", "id": "1001",
			 "attributes": {"part_number": "TST.K-100", "order_number": "K-100",
					"type_number": "K100",
					"designation": {"en_US": "Test contactor", "de_DE": "Testschuetz"}},
			 "relationships": {
				"manufacturer": {"data": {"type": "manufacturers", "id": "7"}},
				"product_group": {"data": {"type": "eplan_catalog_entries", "id": "200030000"}},
				"data_types": {"data": [{"type": "data_types", "id": "commercial_data"},
							{"type": "data_types", "id": "dxf_data"}]}}},
			{"type": "parts", "id": "1002",
			 "attributes": {"part_number": "TST.K-200", "order_number": "K-200",
					"designation": {"en_US": "Test relay"}},
			 "relationships": {
				"manufacturer": {"data": {"type": "manufacturers", "id": "7"}},
				"data_types": {"data": [{"type": "data_types", "id": "commercial_data"}]}}}
		],
		"included": [
			{"type": "manufacturers", "id": "7", "attributes": {"short_name": "TST", "long_name": "Test Werke"}},
			{"type": "eplan_catalog_entries", "id": "200030000", "attributes": {"name": "Relais, Schuetze"}}
		]})";

	/// one canned reply of the stub
	struct Reply { int status = 200; QByteArray headers; QByteArray body; };

	/// HTTP/1.1 server on 127.0.0.1 that answers every request with m_reply
	class Stub : public QTcpServer
	{
		public:
			Stub()
			{
				connect(this, &QTcpServer::newConnection, this, [this]() {
					while (QTcpSocket *s = nextPendingConnection()) {
						connect(s, &QTcpSocket::readyRead, s, [this, s]() { read(s); });
						connect(s, &QTcpSocket::disconnected, s, &QObject::deleteLater);
					}
				});
				listen(QHostAddress::LocalHost);
			}
			QString url() const { return QStringLiteral("http://127.0.0.1:%1/api").arg(serverPort()); }

			Reply m_reply;
			QByteArray m_target;   ///< path and query of the last request
			QByteArray m_auth;     ///< its Authorization header

		private:
			void read(QTcpSocket *s)
			{
				QByteArray &buf = m_buffers[s];
				buf += s->readAll();
				if (!buf.contains("\r\n\r\n")) return;
				const QList<QByteArray> lines = buf.left(buf.indexOf("\r\n\r\n")).split('\n');
				m_target = lines.first().split(' ').value(1);
				m_auth.clear();
				for (const QByteArray &l : lines)
					if (l.toLower().startsWith("authorization:")) m_auth = l.mid(14).trimmed();
				m_buffers.remove(s);
				const QByteArray head = "HTTP/1.1 " + QByteArray::number(m_reply.status) + " X\r\n"
					+ m_reply.headers + "Content-Length: " + QByteArray::number(m_reply.body.size())
					+ "\r\nConnection: close\r\n\r\n";
				s->write(head + m_reply.body);
				s->disconnectFromHost();
			}
			QHash<QTcpSocket *, QByteArray> m_buffers;
	};
}

class DataPortalClientTest : public QObject
{
	Q_OBJECT

	private:
		/// the message of a failing request
		QString failure(DataPortalClient &client, bool download)
		{
			QSignalSpy failed(&client, &DataPortalClient::failed);
			if (download) client.downloadDxf(QStringLiteral("1001"));
			else client.search(QStringLiteral("K-100"), QString());
			if (!failed.wait(10000)) return QString();
			return failed.first().first().toString();
		}

	private slots:
		void authorizationHeader()
		{
			QCOMPARE(DataPortalClient::authorization(QStringLiteral("abc123")), QByteArray("Bearer PAT:abc123"));
			QCOMPARE(DataPortalClient::authorization(QStringLiteral(" PAT:abc123 ")), QByteArray("Bearer PAT:abc123"));
			QCOMPARE(DataPortalClient::authorization(QStringLiteral("Bearer PAT:abc123")), QByteArray("Bearer PAT:abc123"));
			QCOMPARE(DataPortalClient::authorization(QStringLiteral("eyJhbGc.eyJzdWI.c2ln")),
				 QByteArray("Bearer eyJhbGc.eyJzdWI.c2ln"));
		}

		void parseParts()
		{
			int total = 0;
			QString error;
			const QVector<DataPortalPart> parts = DataPortalClient::parseParts(
				PARTS, QStringLiteral("de_DE"), &total, &error);
			QVERIFY2(error.isEmpty(), qPrintable(error));
			QCOMPARE(total, 120);
			QCOMPARE(parts.size(), 2);
			QCOMPARE(parts.at(0).id, QStringLiteral("1001"));
			QCOMPARE(parts.at(0).order_number, QStringLiteral("K-100"));
			QCOMPARE(parts.at(0).manufacturer, QStringLiteral("Test Werke"));
			QCOMPARE(parts.at(0).manufacturer_short, QStringLiteral("TST"));
			QCOMPARE(parts.at(0).designation, QStringLiteral("Testschuetz"));
			QCOMPARE(parts.at(0).product_group, QStringLiteral("Relais, Schuetze"));
			QVERIFY(parts.at(0).has_dxf);
			// no German text: English
			QCOMPARE(parts.at(1).designation, QStringLiteral("Test relay"));
			QVERIFY(!parts.at(1).has_dxf);

			DataPortalClient::parseParts("<html>maintenance</html>", QStringLiteral("en_US"), nullptr, &error);
			QVERIFY(!error.isEmpty());
		}

		void searchWithoutToken()
		{
			Stub stub;
			stub.m_reply = {200, "Content-Type: application/vnd.api+json\r\n", PARTS};
			DataPortalClient client;
			client.setBaseUrl(stub.url());
			QSignalSpy found(&client, &DataPortalClient::searchFinished);
			client.search(QStringLiteral("K-100"), QStringLiteral("TST"));
			QVERIFY(found.wait(10000));
			QCOMPARE(found.first().first().value<QVector<DataPortalPart>>().size(), 2);
			QCOMPARE(found.first().at(1).toInt(), 120);
			QVERIFY(stub.m_auth.isEmpty());
			const QUrlQuery query(QUrl(QString::fromLatin1(stub.m_target)).query());
			QVERIFY(stub.m_target.startsWith("/api/parts?"));
			QCOMPARE(query.queryItemValue(QStringLiteral("search")), QStringLiteral("K-100"));
			QCOMPARE(query.queryItemValue(QStringLiteral("filter[manufacturer]")), QStringLiteral("TST"));
			QVERIFY(query.queryItemValue(QStringLiteral("include")).contains(QStringLiteral("manufacturer")));
		}

		void downloadWithToken()
		{
			Stub stub;
			stub.m_reply = {200, "Content-Type: application/zip\r\n"
					     "Content-Disposition: attachment; filename=\"MAC_TST.K-100.zip\"\r\n",
					QByteArray("PK\x03\x04", 4) + QByteArray(64, 'x')};
			DataPortalClient client;
			client.setBaseUrl(stub.url());
			client.setToken(QStringLiteral("my-test-token"));
			QSignalSpy done(&client, &DataPortalClient::downloadFinished);
			client.downloadDxf(QStringLiteral("1001"));
			QVERIFY(done.wait(10000));
			QCOMPARE(stub.m_target, QByteArray("/api/download/dxf_data/part/1001"));
			QCOMPARE(stub.m_auth, QByteArray("Bearer PAT:my-test-token"));
			QCOMPARE(done.first().at(1).toString(), QStringLiteral("MAC_TST.K-100.zip"));
			QVERIFY(done.first().at(0).toByteArray().startsWith("PK"));
		}

		void errors()
		{
			Stub stub;
			DataPortalClient client;
			client.setBaseUrl(stub.url());

			stub.m_reply = {401, {}, R"({"errors":[{"status":"401","title":"Missing JWT token"}]})"};
			QVERIFY(failure(client, true).contains(QStringLiteral("Personal Access Token")));
			client.setToken(QStringLiteral("wrong"));
			QVERIFY(failure(client, true).contains(QStringLiteral("did not accept the token")));

			stub.m_reply = {403, {}, {}};
			QVERIFY(failure(client, true).contains(QStringLiteral("organisation")));

			stub.m_reply = {404, {}, {}};
			QVERIFY(failure(client, true).contains(QStringLiteral("no DXF package")));

			stub.m_reply = {429, "Retry-After: 30\r\n", {}};
			QVERIFY(failure(client, false).contains(QStringLiteral("30 second")));
			stub.m_reply = {429, {}, {}};
			QVERIFY(failure(client, false).contains(QStringLiteral("limit is reached")));

			stub.m_reply = {400, {}, R"({"errors":[{"status":"400","detail":"The type 'x' is invalid."}]})"};
			QVERIFY(failure(client, false).contains(QStringLiteral("The type 'x' is invalid.")));

			stub.m_reply = {503, {}, {}};
			QVERIFY(failure(client, false).contains(QStringLiteral("HTTP 503")));

			// a web page instead of the package
			stub.m_reply = {200, "Content-Type: text/html\r\n", "<html></html>"};
			QVERIFY(failure(client, true).contains(QStringLiteral("no ZIP")));

			// nobody listens there any more
			const QString url = stub.url();
			stub.close();
			client.setBaseUrl(url);
			QVERIFY(failure(client, false).contains(QStringLiteral("cannot be reached")));
		}
};

QTEST_GUILESS_MAIN(DataPortalClientTest)
#include "tst_dataportalclient.moc"
