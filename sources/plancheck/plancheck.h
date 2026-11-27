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
#ifndef PLANCHECK_H
#define PLANCHECK_H

#include "../projectcheck.h"

#include <QList>
#include <QString>
#include <QUuid>
#include <QVector>

class Diagram;
class Element;
class QETProject;
class Terminal;

/**
	@brief The PlanCheck namespace
	The fork's rules of the Check panel, the ones QElectroTech had no check
	for, as EPLAN's check run (Prüflauf) has them: open terminals, devices
	without a part or label, terminals without a strip, contacts without
	their coil, folio reports without their partner, cables without a
	target, PLC channels without an address, fluid ports and solenoids,
	and the warnings of the cabinet layouts.

	Duplicate labels, potential types and device types are the panel's own
	checks (ProjectCheck): they are not repeated here. ProjectCheck::runAll()
	runs both and gives every finding its severity.

	Where EPLAN has a message for the same thing its number is given
	(e.g. 005026), so a reader who knows EPLAN finds the rule again.
	Read only: nothing is changed, apart from loading folios not read yet.
*/
namespace PlanCheck
{
	struct Finding
	{
		ProjectCheck::Severity severity = ProjectCheck::Severity::Info;
		QString rule;         ///< key of the rule, e.g. "open_terminal"
		QString eplan;        ///< number of EPLAN's message, empty when none
		Diagram *folio = nullptr;
		QString label;        ///< label (BMK) of the device, cable or strip
		QString text;         ///< what is wrong, in the language of the interface
		QList<QUuid> items;   ///< elements or conductors to show, may be empty
		bool incomplete = false;   ///< a datum was missing: not checked ("Angabe fehlt")
	};

		/// the panel's category of a rule
	ProjectCheck::Kind kindOf(const QString &rule);

		/// every rule, the folios loaded first
	QVector<Finding> run(QETProject *project);

		// shared with the checks of logiccheck.cpp
	double ratedCurrentOf(Element *e);         ///< In in A, 0 when unknown
	double sectionOf(QString text);            ///< mm² of "1,5", "3x2.5"; 0 when none
	bool isOutput(Terminal *t, const QList<Terminal *> &all);   ///< even numbered, else at the bottom
}

#endif // PLANCHECK_H
