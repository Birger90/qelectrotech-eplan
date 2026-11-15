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
#ifndef SIMULATIONCATALOG_H
#define SIMULATIONCATALOG_H

#include <QByteArray>
#include <QList>
#include <QString>

/**
	@brief The SimulationCatalog class
	The versioned model catalog (roadmap R00): for every fluid function the
	simulation knows, its role and model. A part is simulated only through
	its catalog entry:
	- Physical: a model of the core computes it (ports, parameters, limits
	  and the checks that cover it are listed);
	- Ideal: an ideal connection, all its fluid ports are one net;
	- Display: a measuring point without effect on the net (in-line
	  instruments join their ports); its output signal is not computed;
	- Graphic: a drawing aid, ignored;
	- Unsupported: no model; a project with it does not start.
	A function that is not in the catalog does not start either: the
	simulation never guesses a model from a symbol or a kind (an unknown
	source is not a pressure source, a gripper is not a linear cylinder).
*/
class SimulationCatalog
{
	public:
		enum class Role { Physical, Ideal, Display, Graphic, Unsupported };

		struct Entry {
			QString function;    ///< fluid function, e.g. "actuator.cyl_conf"
			Role role;
			QString model;       ///< model id in the core, e.g. "linear_cylinder"
			QString domain;      ///< pneumatic, hydraulic, both
			QString ports;       ///< port names the model uses
			QString parameters;  ///< parameters with unit
			QString limits;      ///< what the model does not compute
			QString tests;       ///< checks of --simulate-reference that cover it
		};

		static QString version();
			/// the entry of @a function, nullptr when it is not in the catalog
		static const Entry *find(const QString &function);
		static const QList<Entry> &entries();
		static QString roleName(Role role);
			/// the catalog as JSON (for --simulate-catalog)
		static QByteArray json();
};

#endif // SIMULATIONCATALOG_H
