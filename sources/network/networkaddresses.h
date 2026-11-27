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
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef NETWORKADDRESSES_H
#define NETWORKADDRESSES_H

#include <QList>
#include <QPointer>
#include <QString>
#include <QStringList>

class Element;
class QETProject;
class QUndoCommand;

/**
	@brief The NetworkAddresses namespace
	The address table of the project's network devices: every symbol with
	an IP address or PROFINET name field (element information ip_address,
	subnet_mask, network_name, profinet_name) and the PLC modules of a
	station, with the problems of each row (an IP given twice, outside its
	network, network or broadcast address; a PROFINET name not valid by
	IEC 61158-6-10 or given twice).

	The IP networks of the project (project property ip_networks: name,
	address/prefix, gateway, first host) give the free addresses for the
	devices with an IP field and no address; a PROFINET name comes from
	location and label (+Q1 -2A1: q1-2a1, "pn-" in front of a digit).
	Names and addresses already written stay.
*/
namespace NetworkAddresses
{
	struct Network {
		QString name;
		quint32 address = 0;
		int prefix = 24;
		quint32 gateway = 0;
		int firstHost = 1;   ///< the first host number given, 10 gives .10
	};
	struct Row {
		QPointer<Element> element;
		QString label, location, ip, mask, network, profinet, station;
		int folio = -1;
		QStringList problems;
		bool ipField = false;        ///< it takes an IP address
		bool profinetField = false;  ///< it takes a PROFINET name
	};

	bool ipv4(const QString &text, quint32 *value);
	QString ipText(quint32 value);
		/// why @p name is no valid PROFINET device name, empty when it is one
	QString profinetNameProblem(const QString &name);
		/// the PROFINET name of a device: +Q1 -2A1 gives q1-2a1
	QString profinetNameFor(const QString &label, const QString &location);

	QList<Network> networks(QETProject *project);
	void setNetworks(QETProject *project, const QList<Network> &networks);
		/// "Machine = 10.0.0.0/24, gw 10.0.0.1, from 10"
	bool parseNetwork(const QString &text, Network *network);
	QString networkText(const Network &network);

	QList<Row> rows(QETProject *project);
		/// one undo step each, nullptr when nothing changes
	QUndoCommand *assignIpCommand(QETProject *project, int *count = nullptr);
	QUndoCommand *profinetNamesCommand(QETProject *project, int *count = nullptr);
	QUndoCommand *setValueCommand(Element *element, const QString &key, const QString &value);
	bool exportCsv(QETProject *project, const QString &path, QString *error = nullptr);
}

#endif // NETWORKADDRESSES_H
