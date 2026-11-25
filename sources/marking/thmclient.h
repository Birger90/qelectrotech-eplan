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
#ifndef THMCLIENT_H
#define THMCLIENT_H

#include <QByteArray>
#include <QList>
#include <QElapsedTimer>
#include <QObject>
#include <QPointer>
#include <QString>

class QTcpSocket;
class QTimer;

/**
	@brief The connection to one THM printer: raw TCP (port 9100 unless
	the printer list says otherwise), one request at a time, each on a
	connection of its own that is closed when the request is done.

	Asynchronous: start() returns at once and finished() tells the
	outcome; run() waits for it in an event loop of its own, for the
	command line and scripts. Every step has a time limit, so an
	unreachable or silent printer ends in an error, never in a wait.

	Queries (Version, Status, Material) read a reply; the others only
	send. The commands are those of the cab JScript manual (07/2026):
	v (p. 68), ESC s (p. 29), ESC l (p. 24), ESC p1 / ESC p0 (p. 26-27),
	ESC t (p. 31). The manual asks for at least 0.5 s between two status
	queries and 1 s after ESC t before more data: the client waits so long
	before it starts the next request.

	A print job can go by LPD instead (RFC 1179, port 515, queue "lp" on
	cab printers): some printers keep their RawIP port taken by another
	connection, accept a connection there and read nothing, while LPD
	prints. LPD confirms every step with a zero byte, so a job sent that
	way is known to be taken over. Queries and control commands always
	use RawIP.
*/
class ThmClient : public QObject
{
	Q_OBJECT
	public:
		enum class Request {
			Version,   ///< v CR: firmware and model
			Status,    ///< ESC s: 9 bytes
			Material,  ///< ESC l: 5 bytes
			Pause,     ///< ESC p1
			Resume,    ///< ESC p0
			Cancel,    ///< ESC t: cancels the job and empties the buffer
			Job        ///< the bytes of a print job
		};

		enum class Transport {
			RawIp,  ///< the job on the RawIP port
			Lpd     ///< the job as an LPD print file, raw ("l")
		};

		struct Reply
		{
			Request request = Request::Status;
			bool ok = false;
			QByteArray data;   ///< what the printer answered
			QString error;     ///< why not ok, for a person
		};

		explicit ThmClient(QObject *parent = nullptr);
		~ThmClient() override;

		void setTarget(const QString &host, quint16 port);
			/// How a Job is sent; LPD on @p lpd_port into @p queue
		void setJobTransport(Transport transport, const QString &queue = QStringLiteral("lp"),
							 quint16 lpd_port = 515);
		Transport jobTransport() const { return m_transport; }
		QString host() const { return m_host; }
		quint16 port() const { return m_port; }
			/// Time limits in milliseconds: to connect, for the reply
			/// (queries) or for sending (job and control commands)
		void setTimeouts(int connect_ms, int reply_ms);
			/// The gaps the manual asks for; the tests make them shorter
		void setPauses(int status_ms, int cancel_ms);

		bool busy() const { return m_busy; }
			/// Starts @p request; false (and nothing done) while busy
		bool start(Request request, const QByteArray &job = QByteArray());
			/// start() and wait for finished(); for the command line
		Reply run(Request request, const QByteArray &job = QByteArray());

			/// The bytes sent for a request (the job's own for Job)
		static QByteArray command(Request request);
			/// Bytes of the reply that make it complete; 0 = up to a line end
		static int replyLength(Request request);
		static bool expectsReply(Request request);
		static QString requestTitle(Request request);
			/// The LPD conversation for one job (RFC 1179): open the queue,
			/// data file, control file; each part waits for a zero byte
		static QList<QByteArray> lpdSteps(const QString &queue, const QByteArray &job,
										  int number, const QString &host = QStringLiteral("qet"));
		static QString transportKey(Transport transport);
		static Transport transportFromKey(const QString &key);

	signals:
		void finished(const ThmClient::Reply &reply);

	private:
		void begin();
		void onConnected();
		void onReadyRead();
		void onBytesWritten(qint64 bytes);
		void onTimeout();
		void onSocketError();
		void finish(bool ok, const QString &error = QString());

		QString m_host;
		quint16 m_port = 9100;
		int m_connect_ms = 3000;
		int m_reply_ms = 3000;
		int m_status_pause_ms = 500;
		int m_cancel_pause_ms = 1000;

		bool m_busy = false;
		bool m_connected = false;
		Request m_request = Request::Status;
		QByteArray m_out;
		qint64 m_to_write = 0;
		QByteArray m_in;
		QPointer<QTcpSocket> m_socket;
		QTimer *m_timer = nullptr;
		QElapsedTimer m_last_status;
		QElapsedTimer m_last_cancel;
		Transport m_transport = Transport::RawIp;
		QString m_lpd_queue = QStringLiteral("lp");
		quint16 m_lpd_port = 515;
		bool m_lpd = false;           ///< this request goes by LPD
		QList<QByteArray> m_steps;    ///< LPD parts still to send
		int m_lpd_number = 0;
};

#endif // THMCLIENT_H
