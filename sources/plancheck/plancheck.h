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

#include <QJsonObject>
#include <QList>
#include <QString>
#include <QUuid>
#include <QVariantMap>
#include <QVector>

class QETProject;

/**
	@brief The PlanCheck namespace
	"Check plan": every check of the project in one run, as EPLAN's check
	run (Pruflauf) collects its messages in one list. It reuses the checks
	that already exist one by one -- duplicate labels, fluid ports and
	solenoids, potential types, device types, the cabinet layout check --
	and adds the ones nothing did yet: open terminals, devices without a
	part, terminals without a strip, contacts without their coil, folio
	reports without their partner, cables without a target and PLC
	channels without an address.

	Each finding names its severity, its folio, the label of the device and
	the items to show, so a click can jump to them. Where EPLAN has a
	message for the same thing its number is given (e.g. 005026), so a
	reader who knows EPLAN finds the rule again.

	Read only: nothing in the project is changed, apart from loading folios
	not read yet.
*/
namespace PlanCheck
{
	enum class Severity {
		Error,    ///< the plan is wrong: reports and lists will be wrong
		Warning,  ///< most likely a mistake, worth a look
		Info      ///< often on purpose, a hint only
	};

	struct Finding
	{
		Severity severity = Severity::Info;
		QString check;        ///< key of the check, e.g. "open_terminal"
		QString eplan;        ///< number of EPLAN's message, empty when none
		int folio = -1;       ///< index of the folio, -1 for the project
		QString label;        ///< label (BMK) of the device, cable or strip
		QString text;         ///< what is wrong, in the language of the interface
		QList<QUuid> items;   ///< elements or conductors to show, may be empty
	};

	struct CheckInfo
	{
		QString key;
		QString eplan;
		QString title;        ///< the check in a few words, translated
	};

		/// every check this run makes, in the order of the report
	QList<CheckInfo> checks();

		/// run every check; @p folio >= 0 keeps only the findings of that
		/// folio (and those of the whole project, folio -1)
	QVector<Finding> run(QETProject *project, int folio = -1);

	QString severityKey(Severity severity);    ///< "error", "warning", "info"
	QString severityText(Severity severity);   ///< translated
	int count(const QVector<Finding> &findings, Severity severity);

		/// one finding for scripts: {severity, check, eplan, folio,
		/// folioTitle, label, text, items}; folio counted from 0
	QVariantMap toVariant(const Finding &finding, QETProject *project);
		/// the whole report: {project, ok, summary{errors, warnings, infos},
		/// findings[]}
	QJsonObject toJson(const QVector<Finding> &findings, QETProject *project);
}

#endif // PLANCHECK_H
