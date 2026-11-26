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
#ifndef PANELCLIMATE_H
#define PANELCLIMATE_H

#include "paneldata.h"

#include <QDialog>
#include <QJsonObject>
#include <QList>
#include <QPointer>
#include <QString>

class Element;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QTableWidget;
class QetShapeItem;

/**
	@brief The climate of an enclosure, worked out the simple way the
	enclosure makers do it (formulas and sources: FORMELN.md, K-rows):
	the power loss of the devices on its mounting plate, the effective
	surface A_e of the enclosure from its size and how it stands (surface
	factors of IEC/TR 60890), the heat its walls give off,
	k * A_e * (inside - outside), and from that the rise of the inside
	temperature and what a filter fan, a heat exchanger or a cooling unit
	has to take away to keep the inside at the temperature allowed.
	Even power loss and mixed air inside; a guide value, no verification
	of the temperature rise by itself.

	The power loss of a device is its element information power_loss_w
	(written by the EDZ import from EPLAN's "max. power dissipation"), on
	the layout device, else on its definition, else on the device of the
	schematic it stands for.
*/
struct PanelClimate
{
	struct Device {
		Element *element = nullptr;    ///< the device on the layout
		QString designation, article;
		double watts = 0;
		QString source;                ///< where the value comes from, empty when none
	};

	PanelClimateSettings settings;
	QString name;                      ///< the enclosure, else the mounting plate
	double width_mm = 0, height_mm = 0, depth_mm = 0;
	bool size_estimated = false;       ///< no enclosure and no size given: from the plate
	QList<Device> devices;
	int without_loss = 0;              ///< devices with no power loss data
	double device_w = 0;               ///< sum of the devices' power loss
	double power_w = 0;                ///< power loss counted: simultaneity * devices + extra
	double area_m2 = 0;                ///< effective surface A_e
	double rise_k = 0;                 ///< inside temperature rise without cooling
	double inside_c = 0;               ///< inside temperature without cooling
	double cooling_w = 0;              ///< heat to take away to keep the allowed inside temperature
	double fan_m3h = 0;                ///< filter fan air flow (0: outside not cooler than inside)
	double exchanger_w_k = 0;          ///< specific output of an air/air heat exchanger

		/// work out the results again from the devices, sizes and settings
	void evaluate();
	bool needsCooling() const {return cooling_w > 0.05;}
		/// one line for the layout check and the report
	QString summary() const;
	QJsonObject toJson() const;

		/// the climate of the enclosure around @p plate with its own settings
	static PanelClimate compute(QetShapeItem *plate);
	static PanelClimate compute(QetShapeItem *plate, const PanelClimateSettings &settings);
		/// A_e in m² for installation 1..7 of IEC/TR 60890, sizes in m
	static double effectiveArea(int installation, double width_m, double height_m, double depth_m);
	static QString installationName(int installation);
		/// the power loss an element carries itself (information or definition), W
	static double ownPowerLoss(Element *element);
		/// m³ K / (W h): air flow per watt and kelvin, sea level (1/(rho c_p))
	static constexpr double kAirFactor = 3.1;
};

/**
	@brief Cabinet layout > Climate calculation...: the conditions of the
	enclosure, the power loss of each device (editable, written to the
	device on the layout) and the result, worked out again with every
	change. OK keeps the conditions with the mounting plate, as one undo
	step together with the changed power losses.
*/
class PanelClimateDialog : public QDialog
{
	Q_OBJECT

	public:
		explicit PanelClimateDialog(QetShapeItem *plate, QWidget *parent = nullptr);
		void accept() override;

	private:
		PanelClimateSettings settings() const;
		void recompute();

		QPointer<QetShapeItem> m_plate;
		PanelClimate m_base;               ///< devices and the enclosure's own size
		QComboBox *m_installation = nullptr, *m_material = nullptr;
		QDoubleSpinBox *m_ambient = nullptr, *m_inside = nullptr, *m_k = nullptr,
			*m_simultaneity = nullptr, *m_extra = nullptr,
			*m_width = nullptr, *m_height = nullptr, *m_depth = nullptr;
		QTableWidget *m_table = nullptr;
		QLabel *m_result = nullptr;
		bool m_filling = false;
};

#endif // PANELCLIMATE_H
