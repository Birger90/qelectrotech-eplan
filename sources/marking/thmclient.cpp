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
#include "thmclient.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QTcpSocket>
#include <QTimer>

ThmClient::ThmClient(QObject *parent) :
	QObject(parent),
	m_timer(new QTimer(this))
{
	m_timer->setSingleShot(true);
	connect(m_timer, &QTimer::timeout, this, &ThmClient::onTimeout);
}

ThmClient::~ThmClient()
{
	if (m_socket) {
		m_socket->abort();
		delete m_socket;
	}
}

void ThmClient::setTarget(const QString &host, quint16 port)
{
	m_host = host.trimmed();
	m_port = port;
}

void ThmClient::setJobTransport(Transport transport, const QString &queue, quint16 lpd_port)
{
	m_transport = transport;
	m_lpd_queue = queue.trimmed().isEmpty() ? QStringLiteral("lp") : queue.trimmed();
	m_lpd_port = lpd_port ? lpd_port : 515;
}

QString ThmClient::transportKey(Transport transport)
{
	return transport == Transport::Lpd ? QStringLiteral("lpd") : QStringLiteral("rawip");
}

ThmClient::Transport ThmClient::transportFromKey(const QString &key)
{
	return key.trimmed().compare(QLatin1String("lpd"), Qt::CaseInsensitive) == 0
			? Transport::Lpd : Transport::RawIp;
}

QList<QByteArray> ThmClient::lpdSteps(const QString &queue, const QByteArray &job,
									  int number, const QString &host)
{
	const QByteArray h = host.toLatin1();
	const QByteArray n = QByteArray::number(qAbs(number) % 1000).rightJustified(3, '0');
	const QByteArray data_name = "dfA" + n + h;
	const QByteArray control = "H" + h + "\nPqet\nl" + data_name + "\nN" + data_name + "\n";
	QList<QByteArray> steps;
	steps << ("\x02" + queue.toLatin1() + "\n");
	steps << ("\x03" + QByteArray::number(job.size()) + " " + data_name + "\n");
	steps << (job + QByteArray(1, '\0'));
	steps << ("\x02" + QByteArray::number(control.size()) + " cfA" + n + h + "\n");
	steps << (control + QByteArray(1, '\0'));
	return steps;
}

void ThmClient::setTimeouts(int connect_ms, int reply_ms)
{
	m_connect_ms = qMax(1, connect_ms);
	m_reply_ms = qMax(1, reply_ms);
}

void ThmClient::setPauses(int status_ms, int cancel_ms)
{
	m_status_pause_ms = qMax(0, status_ms);
	m_cancel_pause_ms = qMax(0, cancel_ms);
}

QByteArray ThmClient::command(Request request)
{
	switch (request) {
		case Request::Version: return QByteArray("v\r");
		case Request::Status: return QByteArray("\x1bs");
		case Request::Material: return QByteArray("\x1bl");
		case Request::Pause: return QByteArray("\x1bp1");
		case Request::Resume: return QByteArray("\x1bp0");
		case Request::Cancel: return QByteArray("\x1bt");
		case Request::Job: break;
	}
	return QByteArray();
}

int ThmClient::replyLength(Request request)
{
	switch (request) {
		case Request::Status: return 9;
		case Request::Material: return 5;
		default: return 0;
	}
}

bool ThmClient::expectsReply(Request request)
{
	return request == Request::Version || request == Request::Status
			|| request == Request::Material;
}

QString ThmClient::requestTitle(Request request)
{
	switch (request) {
		case Request::Version: return QCoreApplication::translate("ThmClient", "Firmware version");
		case Request::Status: return QCoreApplication::translate("ThmClient", "Status");
		case Request::Material: return QCoreApplication::translate("ThmClient", "Material");
		case Request::Pause: return QCoreApplication::translate("ThmClient", "Pause");
		case Request::Resume: return QCoreApplication::translate("ThmClient", "Resume");
		case Request::Cancel: return QCoreApplication::translate("ThmClient", "Cancel");
		case Request::Job: return QCoreApplication::translate("ThmClient", "Print job");
	}
	return QString();
}

bool ThmClient::start(Request request, const QByteArray &job)
{
	if (m_busy) return false;
	m_busy = true;
	m_request = request;
	m_out = request == Request::Job ? job : command(request);
	m_in.clear();
	m_connected = false;
	m_lpd = request == Request::Job && m_transport == Transport::Lpd;
	m_steps.clear();
	if (m_lpd && !m_out.isEmpty()) m_steps = lpdSteps(m_lpd_queue, m_out, ++m_lpd_number);
	if (m_host.isEmpty() || m_port == 0) {
		QTimer::singleShot(0, this, [this]() {
			finish(false, QCoreApplication::translate("ThmClient", "No printer address is set."));
		});
		return true;
	}
	if (m_out.isEmpty()) {
		QTimer::singleShot(0, this, [this]() {
			finish(false, QCoreApplication::translate("ThmClient", "There is nothing to send."));
		});
		return true;
	}
		//The gaps the manual asks for: 0.5 s between status queries
		//(p. 30), 1 s after a cancel before more data (p. 31)
	qint64 wait = 0;
	if (request == Request::Status && m_last_status.isValid()) {
		wait = qMax<qint64>(wait, m_status_pause_ms - m_last_status.elapsed());
	}
	if (m_last_cancel.isValid()) {
		wait = qMax<qint64>(wait, m_cancel_pause_ms - m_last_cancel.elapsed());
	}
	QTimer::singleShot(int(qMax<qint64>(0, wait)), this, &ThmClient::begin);
	return true;
}

ThmClient::Reply ThmClient::run(Request request, const QByteArray &job)
{
	Reply result;
	result.request = request;
	if (m_busy) {
		result.error = QCoreApplication::translate("ThmClient", "A request is still running.");
		return result;
	}
	QEventLoop loop;
	const auto c = connect(this, &ThmClient::finished, &loop,
						   [&](const ThmClient::Reply &r) { result = r; loop.quit(); });
	start(request, job);
	loop.exec();
	disconnect(c);
	return result;
}

void ThmClient::begin()
{
	if (!m_busy) return;
	if (m_request == Request::Status) m_last_status.start();
	m_socket = new QTcpSocket(this);
	connect(m_socket, &QTcpSocket::connected, this, &ThmClient::onConnected);
	connect(m_socket, &QTcpSocket::readyRead, this, &ThmClient::onReadyRead);
	connect(m_socket, &QTcpSocket::bytesWritten, this, &ThmClient::onBytesWritten);
	connect(m_socket, &QTcpSocket::errorOccurred, this, &ThmClient::onSocketError);
	connect(m_socket, &QTcpSocket::disconnected, this, [this]() {
		if (!m_busy) return;
		if (m_lpd) {
			finish(false, QCoreApplication::translate("ThmClient", "The printer closed the LPD connection before the job was taken over."));
		} else if (!expectsReply(m_request) && m_connected && m_to_write <= 0) {
			finish(true);
		} else if (expectsReply(m_request) && m_request == Request::Version && !m_in.trimmed().isEmpty()) {
			finish(true);
		} else {
			finish(false, QCoreApplication::translate("ThmClient", "The printer closed the connection."));
		}
	});
	m_timer->start(m_connect_ms);
	m_socket->connectToHost(m_host, m_lpd ? m_lpd_port : m_port);
}

void ThmClient::onConnected()
{
	m_connected = true;
	m_timer->start(m_reply_ms);
	if (m_lpd) {
		m_to_write = m_steps.first().size();
		m_socket->write(m_steps.first());
		return;
	}
	m_to_write = m_out.size();
	m_socket->write(m_out);
}

void ThmClient::onBytesWritten(qint64 bytes)
{
	m_to_write -= bytes;
	if (m_lpd) return; //each part waits for its zero byte
	if (m_to_write > 0 || expectsReply(m_request)) return;
	if (m_request == Request::Cancel) m_last_cancel.start();
		//All handed over: close, the job goes on in the printer
	m_socket->disconnectFromHost();
	if (m_socket && m_socket->state() == QAbstractSocket::UnconnectedState) finish(true);
}

void ThmClient::onReadyRead()
{
	m_in += m_socket->readAll();
	if (m_lpd) {
		while (!m_in.isEmpty() && !m_steps.isEmpty()) {
			const char ack = m_in.at(0);
			m_in.remove(0, 1);
			if (ack != '\0') {
				finish(false, QCoreApplication::translate("ThmClient", "The printer refused the LPD job (answer %1 in step %2).")
					   .arg(int(static_cast<unsigned char>(ack))).arg(6 - m_steps.size()));
				return;
			}
			m_steps.removeFirst();
			if (m_steps.isEmpty()) {
				finish(true);
				return;
			}
			m_timer->start(m_reply_ms);
			m_to_write = m_steps.first().size();
			m_socket->write(m_steps.first());
		}
		return;
	}
	const int length = replyLength(m_request);
	if (length > 0) {
			//Line ends the printer may add are not part of the reply
		const QByteArray trimmed = m_in.trimmed();
		if (trimmed.size() >= length) {
			m_in = trimmed.left(length);
			finish(true);
		}
		return;
	}
	if (m_request == Request::Version
			&& (m_in.contains('\r') || m_in.contains('\n')) && !m_in.trimmed().isEmpty()) {
		finish(true);
	}
}

void ThmClient::onTimeout()
{
	if (!m_busy) return;
	if (!m_connected) {
		finish(false, QCoreApplication::translate("ThmClient", "The printer %1:%2 cannot be reached (no connection within %3 s).")
			   .arg(m_host).arg(m_lpd ? m_lpd_port : m_port).arg(m_connect_ms / 1000.0));
		return;
	}
		//A version without line end is complete when nothing more comes
	if (m_request == Request::Version && !m_in.trimmed().isEmpty()) {
		finish(true);
		return;
	}
	if (m_lpd) {
		finish(false, QCoreApplication::translate("ThmClient", "The printer did not confirm the LPD job within %1 s.")
			   .arg(m_reply_ms / 1000.0));
		return;
	}
	if (!expectsReply(m_request) && m_to_write <= 0) {
		finish(true);
		return;
	}
	if (expectsReply(m_request)) {
		finish(false, m_in.isEmpty()
			   ? QCoreApplication::translate("ThmClient", "The printer did not answer within %1 s.").arg(m_reply_ms / 1000.0)
			   : QCoreApplication::translate("ThmClient", "The printer's answer is incomplete: \"%1\".")
				 .arg(QString::fromLatin1(m_in)));
	} else {
		finish(false, QCoreApplication::translate("ThmClient", "Sending to the printer did not finish within %1 s.")
			   .arg(m_reply_ms / 1000.0));
	}
}

void ThmClient::onSocketError()
{
	if (!m_busy || !m_socket) return;
	if (m_socket->error() == QAbstractSocket::RemoteHostClosedError) return; //disconnected() decides
	finish(false, m_connected
		   ? QCoreApplication::translate("ThmClient", "Connection to the printer failed: %1").arg(m_socket->errorString())
		   : QCoreApplication::translate("ThmClient", "The printer %1:%2 cannot be reached: %3")
			 .arg(m_host).arg(m_port).arg(m_socket->errorString()));
}

void ThmClient::finish(bool ok, const QString &error)
{
	if (!m_busy) return;
	m_busy = false;
	m_timer->stop();
	if (m_socket) {
		QTcpSocket *socket = m_socket;
		m_socket = nullptr;
		socket->disconnect(this);
			//A reply read in full: closing may drop nothing; a job is
			//closed gracefully so the bytes already sent still arrive
		if (socket->state() != QAbstractSocket::UnconnectedState) {
			if (expectsReply(m_request) || !ok) socket->abort();
			else socket->disconnectFromHost();
		}
		socket->deleteLater();
	}
	Reply r;
	r.request = m_request;
	r.ok = ok;
	r.data = m_in;
	r.error = error;
	m_out.clear();
	m_steps.clear();
	m_lpd = false;
	emit finished(r);
}
