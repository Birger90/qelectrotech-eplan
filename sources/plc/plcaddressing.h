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
#ifndef PLCADDRESSING_H
#define PLCADDRESSING_H

#include <QList>
#include <QString>
#include <QStringList>
#include <QVariantMap>

class Element;
class QETProject;
class QUndoCommand;

/**
	@brief The PlcAddressing namespace
	The addresses of the PLC modules (the address of each row of their IO
	table), given per station in the station's own format:

	| format      | digital in | digital out | analog in | analog out |
	|-------------|------------|-------------|-----------|------------|
	| siemens     | I0.0       | Q0.0        | IW2       | QW2        |
	| siemens_de  | E0.0       | A0.0        | EW2       | AW2        |
	| iec         | %IX0.0     | %QX0.0      | %IW2      | %QW2       |
	| omron       | 0.00       | 100.00      | 2000      | 2100       |
	| twincat     | %I*        | %Q*         | %I*       | %Q*        |

	A station is the element information "plc_station" of the module
	(empty: the project's one station). Its modules are numbered in folio
	order, then left to right: each module takes whole bytes (Omron:
	16-bit words) for its digital channels, its analog channels a word
	each, after the digital bytes or from a start of their own. "Analog
	first" puts the modules with analog channels before the others, as
	WAGO and other CODESYS couplers lay out their process image. A row
	whose address is "IOL:2/2" is an IO-Link port with 2 bytes of process
	data in and 2 out: it gets "IB4…IB5 / QB2…QB3" (EB/AB, %IB/%QB).

	Stable: a module whose rows all have an address of its format keeps
	them and the others take the next free bytes; "all" numbers every
	module anew. The settings of each station are kept with the project
	(plc_station_format__<station> ...; plc_address_format for all).
*/
namespace PlcAddressing
{
	enum class Format { Siemens, SiemensGerman, Iec, Omron, TwinCat };
	enum class Order { Cards, AnalogFirst };

	struct Station {
		QString name;
		Format format = Format::Siemens;
		Order order = Order::Cards;
		int inputStart = -1;        ///< first input byte (word), -1: the format's
		int outputStart = -1;       ///< first output byte (word), -1: the format's
		int analogInputStart = -1;  ///< own start of the analog inputs, -1: after the digital
		int analogOutputStart = -1; ///< own start of the analog outputs, -1: after the digital
	};

	QString formatKey(Format format);
	Format formatFromKey(const QString &key, Format fallback = Format::Siemens);
	QString formatTitle(Format format);

		/// the station of a PLC module, empty for the project's one
	QString stationOf(const Element *module);
	QStringList stations(QETProject *project);
	Station station(QETProject *project, const QString &name);
	void setStation(QETProject *project, const Station &station);
	QVariantMap toMap(const Station &station);
	Station fromMap(const QVariantMap &map, const Station &base);

	struct Change {
		Element *module = nullptr;
		int row = -1;
		QString oldAddress;
		QString newAddress;
	};
		/// What giving the addresses would change. @p all: every module
		/// anew; @p only: one station (a null string for all)
	QList<Change> plan(QETProject *project, bool all = false, const QString &only = QString());
	QUndoCommand *command(const QList<Change> &changes, QUndoCommand *parent = nullptr);
}

#endif // PLCADDRESSING_H
