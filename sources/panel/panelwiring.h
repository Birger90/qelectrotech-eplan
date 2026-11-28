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
#ifndef PANELWIRING_H
#define PANELWIRING_H

#include <QJsonObject>
#include <QLineF>
#include <QList>
#include <QString>
#include <QStringList>

class PanelLayout;
class QetShapeItem;

/**
	@brief How full one cable duct gets: the wires through its fullest
	stretch, each taking the square of its outer diameter, against the
	room inside the duct.
*/
struct PanelDuctFill
{
	QetShapeItem *duct = nullptr;
	QString name;
	double width_mm = 0, depth_mm = 0;
	double inner_mm2 = 0;      ///< room inside: (width - 4) x (depth - 4), walls of 2 mm
	double used_mm2 = 0;       ///< wires at the fullest stretch
	int wires = 0;             ///< wires through the fullest stretch
	double percent() const {return inner_mm2 > 0 ? 100 * used_mm2 / inner_mm2 : 0;}
};

/**
	@brief The wires of a mounting surface, routed the simple way through
	its cable ducts (rules and sources: FORMELN.md, W-rows):

	- the schematic says which devices of this surface one potential joins
	  (conductors, junctions and folio reports followed; a contact belongs
	  to its device, a terminal to its terminal strip);
	- a potential joining n devices needs n - 1 wires, the shortest ones
	  (minimum spanning tree over the lengths through the ducts);
	- a wire leaves its device into the nearest duct, runs along the duct
	  centre lines to where ducts meet, the shortest way, and leaves into
	  the duct nearest the other device;
	- its cross-section is the largest one written on the conductors of
	  the potential (1.5 mm² when none is), its outer diameter a guide
	  value of a single core PVC wire (H05V-K / H07V-K).

	Wires to devices off this surface (motors, field devices) are not
	counted; neither are cables and the wiring of a device to itself.
*/
/**
	@brief One straight run of a bundle: wires without a duct, laid on the
	mounting plate along a grid and tied together where they share a way.
*/
struct PanelBundleRun
{
	QLineF line_mm;             ///< on the surface
	int wires = 0;
	double diameter_mm = 0;     ///< sqrt(sum d² / 0.75): round wires packed in a bundle
};

/**
	@brief One wire of the wiring list of a mounting surface: from one
	device to another, in the order the wiring goes (each potential grows
	from its first device, the nearest device next: the shortest wires the
	route found), its length through the ducts or on the plate, and the
	ferrule for its cross-section.
*/
struct PanelWireRun
{
	int order = 0;              ///< 1, 2 ... over the surface
	QString wire;               ///< the wire number of the potential, empty when none
	QString from, to;           ///< designations of the two devices
	double section_mm2 = 0;
	bool assumed_section = false;
	double length_mm = 0;       ///< device to device, without the reserve
	bool free = false;          ///< laid on the plate, no duct
	bool unrouted = false;      ///< no way at all
		/// with @p reserve_mm at each end, rounded up to 10 mm
	double cutLength(double reserve_mm) const;
		/// Phoenix Contact AI type for the cross-section (DIN 46228-4)
	QString ferrule() const;
};

struct PanelWiring
{
	QList<PanelDuctFill> ducts;
	QList<PanelBundleRun> bundles;  ///< the free ways, wires without a duct
	QList<PanelWireRun> runs;   ///< every wire, in the order of the wiring
	int wires = 0;              ///< wires routed through ducts
	int free_wires = 0;         ///< wires laid freely on the plate (no duct, or no duct joins them)
	int unrouted = 0;           ///< wires with no way at all (enclosed by devices)
	int assumed_section = 0;    ///< wires without a cross-section in the schematic
	int cable_ties = 0;         ///< on the bundles: one each 150 mm where two wires or more share a way
	double length_mm = 0;       ///< the routed wires together, device to device, ducts and free
	double free_length_mm = 0;  ///< the free ones of them
	double max_bundle_mm = 0;   ///< thickest bundle

		/// one finding per duct filled to the limit or above, like the layout check
	QStringList warnings() const;
	QString summary() const;
	QJsonObject toJson() const;
		/// the wiring list, one row per wire, ; separated with a header
	QString wireListCsv(double reserve_mm = 0) const;

	static PanelWiring route(const PanelLayout &layout);
		/// guide value of the outer diameter of a single core wire, mm
	static double outerDiameter(double section_mm2);
	static constexpr double kWarnPercent = 70;      ///< a duct this full is a finding
	static constexpr double kHintPercent = 56;      ///< 80 % of the limit: shown orange
	static constexpr double kDefaultSection = 1.5;  ///< mm², for conductors without one
	static constexpr double kGridMm = 25;           ///< grid of the free ways
	static constexpr double kBundleCost = 0.35;     ///< a step along a bundle against a free one
	static constexpr double kTieSpacingMm = 150;    ///< cable ties along a bundle
};

#endif // PANELWIRING_H
