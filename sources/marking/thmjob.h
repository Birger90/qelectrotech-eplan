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
#ifndef THMJOB_H
#define THMJOB_H

#include "markingexport.h"

#include <QByteArray>
#include <QList>
#include <QString>
#include <QStringList>

/**
	@brief Print jobs and replies of the thermal transfer printers of the
	THM family (THM MMP, THM MultiMark), which are built on cab technology
	and understand cab's printer language JScript.

	Pure functions: the bytes of a job from a list of labels and a material
	profile, and what the printer's replies to the status queries mean. No
	network, no settings, no project: the client (ThmClient) sends the
	bytes, the printer list (ThmPrinters) holds the profiles.

	Every command written and every reply read is taken from the cab
	"JScript Programming Manual", edition 07/2026, firmware 5.48
	(https://www.cab.de/media/pushfile.cfm?file=3047); the page of each one
	is given where it is used and in docs/beschriftung-export.md. That
	manual says that cab cannot guarantee every function on OEM printers,
	so a profile is to be tried on the real printer first.
*/
namespace ThmJob
{
		/// The label sensor, parameter ptype of the S command (manual p. 220)
	enum class Sensor {
		Endless,        ///< "e": continuous material, sensor off
		Gap,            ///< "l1": die cut labels with a gap
		ReflectTop,     ///< "l0": reflective mark on the upper side
		ReflectBottom   ///< "l2": reflective mark on the lower side
	};

		/// The two THM printer families seen in use. They differ in what
		/// the setup menu of the device accepts (web interface cmd=1 and
		/// cmd=9 read on the devices themselves):
		/// THM MMP, firmware 4.29: speeds 30-125 mm/s, heat -10 ... +20,
		/// print head 105.7 mm;
		/// THM MultiMark, firmware 5.36: speeds 30-150 mm/s, heat -20 ...
		/// +20, print head 106 mm.
	enum class Family {
		Mmp,        ///< THM MMP; also the stricter limits when unknown
		MultiMark   ///< THM MultiMark
	};

		/// The material a printer is loaded with and how to print on it.
		/// Lengths in millimetres. No default is a value of any real
		/// material: width and height 0 make validate() fail until the
		/// profile is filled in.
	struct Profile
	{
		double width = 0;        ///< one label, across the transport
		double height = 0;       ///< one label, in the transport direction
		double gap = 0;          ///< between two labels in the transport direction
		Sensor sensor = Sensor::Gap;
		int columns = 1;         ///< labels side by side
		double column_pitch = 0; ///< start of one column to the next; 0 = width
		double margin_left = 0.5;  ///< kept free inside a label, left (as Kabelmark)
		double margin_right = 0.5; ///< kept free inside a label, right
		double offset_x = 0;     ///< xo of the S command
		double offset_y = 0;     ///< yo of the S command
		int font = 3;            ///< T command font number, 3 = Swiss 721 (manual p. 224)
		double text_size = 3;    ///< T command size, in millimetres
		double baseline = 0;     ///< the text's y; 0 = (height + text_size) / 2
		bool set_heat = false;   ///< write an H command
		int speed = 0;           ///< H: mm/s
		int heat = 0;            ///< H: heat correction, kHeatMin ... kHeatMax
		bool transfer = true;    ///< H: T thermal transfer, else D direct
		bool rotate = false;     ///< O R: contents turned by 180 degrees
			/// The device family whose limits validate() applies
		Family family = Family::Mmp;
			/// wd of the S command, the whole width of a row as the maker's
			/// layout gives it; 0: the columns' width (totalWidth())
		double page_width = 0;
			/// Where the first column starts, from the left of the row
		double first_x = 0;
			/// The text field inside a label in the transport direction:
			/// from field_top to field_bottom (0: the label's height); the
			/// text is centred in it
		double field_top = 0;
		double field_bottom = 0;
			/// r of the T command: 0, 90, 180 or 270 degrees
		int text_rotation = 0;
			/// Further options of the O command besides R, as the maker's
			/// software writes them (e.g. T, P)
		QStringList print_options;
			/// Written after "H speed,heat,T", e.g. "B30"
		QString heat_options;
			/// zO: a zero without slash, as the maker's software writes it
		bool plain_zero = false;
	};

		/// One text and how many labels of it
	struct Label
	{
		QString text;
		int count = 1;
	};

		/// Limits read from a THM MMP (firmware 4.29) through its web
		/// interface (cgi-bin get cmd=1 and cmd=9): heat correction -10 ... +20,
		/// speeds in fixed steps only, print head 105.7 mm at 11.8 dots/mm.
	constexpr int kHeatMin = -10;
	constexpr int kHeatMax = 20;
	constexpr double kPrintWidthMm = 105.7;
	constexpr double kDotsPerMm = 11.8;
		/// 30, 40, 50, 75, 100, 125 mm/s (THM MMP)
	QList<int> speedSteps();
		/// The speeds, heat range and print head width of a family
	QList<int> speedSteps(Family family);
	int heatMin(Family family);
	int heatMax(Family family);
	double printWidth(Family family);
		/// "mmp", "multimark"
	QString familyKey(Family family);
	bool familyFromKey(const QString &key, Family *family);
		/// From what a printer calls itself ("4.29 ... (THM MMP)",
		/// "THM MultiMark"); MMP when it says neither
	Family familyFromModel(const QString &model);
		/// all columns side by side: (columns - 1) * pitch + width
	double totalWidth(const Profile &profile);
		/// wd of the S command: page_width, else first_x + totalWidth()
	double rowWidth(const Profile &profile);

	bool validate(const Profile &profile, QString *error = nullptr);

		/// The job, CR after each line: per label (or per row of labels
		/// when the profile has several columns) m m, J, [H], S, [O R],
		/// T ..., A <count>; \x1boUTF-8; before a block with non-ASCII
		/// text. Empty when the profile is not valid (@p error says why).
	QByteArray build(const QList<Label> &labels, const Profile &profile,
					 QString *error = nullptr);

		/// The text as it can stand in a T command: line breaks and other
		/// control characters become spaces, '[' becomes [U:$5B] so that
		/// it is not read as a special content field (manual p. 241, 353)
	QString escapeText(const QString &text);

		/// The markers of the marking export as labels: @p copies of
		/// each; with @p merge the equal texts in one label, in the order
		/// they first come. Empty texts are left out.
	QList<Label> toLabels(const QList<MarkingExport::Item> &items,
						  int copies = 1, bool merge = true);
	int labelCount(const QList<Label> &labels);

		/// "e", "l1", "l0", "l2"
	QString sensorKey(Sensor sensor);
	bool sensorFromKey(const QString &key, Sensor *sensor);
	QString sensorTitle(Sensor sensor);

	/**
		@brief The reply to ESC s (manual p. 29-30): XYNNNNNNZ, 9 bytes.
		X Y online / N offline; Y '-' no error, else an error letter;
		NNNNNN the labels still to print; Z Y the interpreter is active
		(a job is in process) / N standby.
	*/
	struct Status
	{
		bool valid = false;
		bool online = false;
		char error = '-';
		int pending = 0;
		bool interpreter_active = false;
		QByteArray raw;
		bool hasError() const { return valid && error != '-'; }
	};
	Status parseStatus(const QByteArray &reply);
		/// What the error letter of ESC s means, from the manual's table
	QString errorText(char code);
		/// One line for a person: online, error, labels pending
	QString statusText(const Status &status);

	/**
		@brief The reply to ESC l (manual p. 24): XNNNN, 5 bytes.
		X Y synchronized / N not; NNNN the label distance in millimetres,
		bottom edge to bottom edge of the next label; 0000 unknown.
	*/
	struct Material
	{
		bool valid = false;
		bool synchronized = false;
		int pitch = 0;           ///< millimetres; 0 unknown
		QByteArray raw;
	};
	Material parseMaterial(const QByteArray &reply);
	QString materialText(const Material &material);

		/// Whether the measured distance fits the profile's height + gap
	enum class Fit { Unknown, Fits, Differs };
		/// The printer reports whole millimetres, so up to 1 mm apart fits
	Fit materialFit(const Material &material, const Profile &profile);

		/// The reply to v (manual p. 68), without line ends
	QString parseVersion(const QByteArray &reply);

		/// Numbers as JScript reads them: '.' as decimal point, at most
		/// two decimals, no trailing zeros (manual p. 8: ',' and ';'
		/// separate parameters)
	QString number(double value);
}

#endif // THMJOB_H
