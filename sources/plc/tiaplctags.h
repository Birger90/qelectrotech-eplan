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
#ifndef TIAPLCTAGS_H
#define TIAPLCTAGS_H

#include <QPointer>
#include <QString>
#include <QStringList>
#include <QVector>

class Element;
class QETProject;
class QUndoCommand;

/**
	@brief PLC tags of a project, exchanged with Siemens TIA Portal.

	The channels are the rows of the IO tables of the project's PLC
	modules (plcMasterData/plcIOs): per row its kind (DI, DQ, AI, AQ),
	its address, its symbolic address (the row's comment, as the EPLAN
	import writes it) and its function text.

	Export: one PLC tag per channel, written as the tag table TIA Portal
	imports.
	- .xlsx: the workbook of "Format of the export file (*.xlsx)" in the
	  STEP 7 help (V20/V21): sheet "PLCTags" with the columns Name, Path,
	  Data Type, Logical Address, Comment, Hmi Visible, Hmi Accessible,
	  Hmi Writeable, Typeobject ID, Version ID; sheet "TagTableProperties"
	  with Path, BelongsToUnit, Accessibility; the custom document property
	  TIA_Version = 1.0 (without it TIA reports "Missing version ID").
	  Imported in the tag table editor (Import button).
	- .xml: a SimaticML tag table (SW.Tags.PlcTagTable), the format TIA
	  Portal Openness exports and imports (PlcTagTableComposition.Import).

	Address: an absolute address of the row (E 50.0, %I50.0, A1.3, PEW256,
	%QW64 ...) is taken as it is, in international mnemonics. A row that
	only names its channel (DI0, DQ3, AI1, as the modules of the
	collection carry them) gets one counted from the module's start
	address: element information plc_start_address (inputs) and
	plc_start_address_2 (outputs) as EPLAN's "Startadresse" and
	"Startadresse 2"; without them the next free bytes after every
	absolute address, module after module in folio order, as TIA Portal
	assigns them when the modules are plugged in that order.

	Name: the symbolic address of the row; without one the module label
	and the channel (A3_DI0). Names are unique in a CPU without regard to
	case (help "Valid names of PLC tags"), so a repeated one gets _2, _3.

	Compare: a tag table read back (.xlsx exported by TIA Portal or by
	this export, or a SimaticML .xml) against the channels, by address
	first (lists carry no device tags), then by name.
*/
namespace TiaPlcTags
{
	/// one row of a PLC module's IO table
	struct Channel {
		QPointer<Element> master;	///< the PLC module
		QPointer<Element> slave;	///< the channel symbol linked to the row, if any
		int io = -1;			///< row in the module's IO table
		QString module;			///< label of the module
		QString kind;			///< DI, DQ, AI, AQ
		QString rawAddress;		///< address as the row has it
		QString address;		///< TIA address, e.g. %I50.0; empty if none
		bool autoAddress = false;	///< counted from the module's start address
		QString symbol;			///< the row's symbolic address (comment)
		QString function;		///< the row's function text
		QString name;			///< tag name written (symbol or generated)
		QString dataType;		///< Bool, Int, Byte, DWord
		int folio = -1;			///< folio index of slave, else module
	};

	/// one PLC tag of a TIA tag table
	struct Tag {
		QString name;
		QString path;			///< tag table (group/name)
		QString dataType;
		QString address;		///< as in the file
		QString comment;
	};

	enum class Status {
		Same,		///< address, name and comment agree
		NameDiffers,	///< same address, other name
		CommentDiffers,	///< same address and name, other comment
		BothDiffer,	///< same address, other name and comment
		AddressDiffers,	///< same name, other address
		MissingInTia,	///< channel without tag
		OnlyInTia	///< I/Q tag without channel
	};

	struct Difference {
		Status status = Status::Same;
		int channel = -1;		///< index into the channels, -1 for OnlyInTia
		int tag = -1;			///< index into the tags, -1 for MissingInTia
	};

	struct Comparison {
		QVector<Channel> channels;
		QVector<Tag> tags;
		QVector<Difference> rows;
		int otherTags = 0;		///< tags outside I and Q (memory, DB, no address)
		int count(Status status) const;
	};

	/// the channels of @p project; @p notes gets what could not be resolved
	QVector<Channel> channels(QETProject *project, QStringList *notes = nullptr);

	/**
		@return @p raw as TIA address in international mnemonics
		("E 50.0" -> "%I50.0", "PEW256" -> "%IW256", "AB3" -> "%QB3"),
		empty if it is no absolute I/Q address. M, DB and the like give
		an empty string too: they are not channels.
	*/
	QString normalizedAddress(const QString &raw);

	/**
		write @p channels as tag table @p table, by the end of @p path:
		.xlsx (TIA tag table editor), .xml (TIA Openness, SimaticML),
		.gvl/.st/.exp (CODESYS global variable list: VAR_GLOBAL with
		AT %IX0.0 : BOOL), .plcopen.xml (PLCopen XML TC6 v2.01, global
		variables of a resource; CODESYS, TwinCAT, Beremiz). IEC
		addresses give a bit its size X (%I50.0 -> %IX50.0).
	*/
	bool write(const QString &path, const QVector<Channel> &channels,
			   const QString &table, const QString &culture, QString *error);

	/// read a tag table: .xlsx (sheet PLCTags/PLC Tags) or SimaticML .xml
	QVector<Tag> read(const QString &path, QString *error);

	Comparison compare(const QVector<Channel> &channels, const QVector<Tag> &tags);

	/// localized status ("Name differs" ...)
	QString statusText(Status status);
	/// CSV report of @p comparison (one row per difference, header)
	QString reportCsv(const Comparison &comparison);

	/**
		Takes names and comments of the tags into the IO tables: for every
		row with the same address the tag's name becomes the symbolic
		address and its comment, if it has one, the function text.
		@param undo_parent: the changes become ChangeElementDataCommand
		children of it; nullptr changes the elements directly
		@return number of IO rows changed
	*/
	int apply(const Comparison &comparison, QUndoCommand *undo_parent = nullptr);
}

#endif // TIAPLCTAGS_H
