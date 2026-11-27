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
#ifndef PROJECTCHECK_H
#define PROJECTCHECK_H

#include <QCoreApplication>
#include <QGraphicsObject>
#include <QJsonObject>
#include <QList>
#include <QPointer>
#include <QString>
#include <QStringList>
#include <QUuid>
#include <QVariantList>
#include <QVector>

class Diagram;
class QETProject;

/**
	@brief The ProjectCheck class
	The checks of a project, run one after the other for the Check panel
	(#1878 B1). Each check is the one QElectroTech already has; this only
	gives their findings one shape: a finding is a line of text and the
	places it is about, each of which can be shown on its folio.

	Duplicate labels and device types read the built folios: the caller
	builds the lazy ones first. The others read the project database only.
	A check reports; it changes nothing in the project.
*/
class ProjectCheck
{
	Q_GADGET
	Q_DECLARE_TR_FUNCTIONS(ProjectCheck)

	public:
			/// The name of each kind is its id in a project file
			/// (CheckSettings, #1878 B8): never rename one
		enum class Kind {
			DuplicateLabels,   ///< DuplicateLabels::find()
			PotentialTypes,    ///< projectDataBase::potentialTypeMismatches()
			OpenWireEnds,      ///< projectDataBase::openWireEnds()
			UnwiredSymbols,    ///< symbols none of whose terminals has a wire
			TerminalBranches,  ///< projectDataBase::terminalBranches()
			DeviceTypes,       ///< DeviceTypeCheck::check()
			WiresPerTerminal,  ///< terminals with more wires than WiringRules::limit()
			TerminalNames,     ///< symbols with two terminals of one name (TerminalNameCheck)
			WireInTwoCables,   ///< a wire that is a core of two cables
			NoPartNumber,      ///< symbols the parts list holds, with no part number
			UnansweredPrompts, ///< labels and texts still holding a ? or a ?name
				// Fork: the rules of "Check plan" (plancheck/plancheck.cpp)
			Links,             ///< contacts, coils, valve coils and folio reports without their partner
			PlcChannels,       ///< PLC channels without a card or an address
			Cables,            ///< cables without a label or a target
			TerminalStrips,    ///< terminals in no terminal strip
			LabelsAndParts,    ///< devices without a label or a part number
			OpenTerminals,     ///< terminals and fluid ports left open on a wired symbol
			WireProtection,    ///< wires too thin for the protective device in front of them
			Network,           ///< IP addresses and PROFINET device names: valid, unique, in the subnet
			CabinetLayout      ///< the warnings of the cabinet layouts
		};

			/// Fork: how bad a finding is, for the filter of the panel and
			/// the exit code of --check-project
		enum class Severity {
			Error,    ///< the plan is wrong: reports and lists will be wrong
			Warning,  ///< most likely a mistake, worth a look
			Info      ///< often on purpose, a hint only
		};
		Q_ENUM(Kind)

			/// Where a finding is. One of item, element or wires says what
			/// to show; a place with none of them is a line of detail only.
		struct Place {
			QString text;                    ///< what is there: a label, a part
			QString folio;                   ///< "3 — Power", empty if not known
			QString cell;                    ///< "B4", empty if not known
			QPointer<QGraphicsObject> item;  ///< a built item
			QUuid element;                   ///< a symbol, when item is not given
			QUuid drawing;                   ///< a free text of the folio, found
											 ///< on the folio diagram
			QUuid diagram;                   ///< the symbol's folio
			QList<QUuid> wires;              ///< wires, found when shown, on
											 ///< the folio diagram if given
			bool canShow() const {return item || !element.isNull() || !drawing.isNull() || !wires.isEmpty();}
		};

		struct Finding {
			QString text;
			QList<Place> places;
				// Fork: set by runAll()
			Severity severity = Severity::Warning;
			QString rule;   ///< key of the rule, e.g. "open_terminal"
			QString eplan;  ///< number of EPLAN's message, empty when none
				/// Further lasting ids of what the finding is about (a
				/// terminal's uuid), when its places alone do not tell
				/// it from another finding: CheckSettings::findingId()
			QStringList ids;
		};

		struct Result {
			Kind kind = Kind::DuplicateLabels;
				/// Why the check found nothing to check (switched off for
				/// the project, no limit set); empty when it ran
			QString note;
			QVector<Finding> findings;
		};

			/// Every check, in the order the panel lists them
		static QList<Kind> kinds();
		static QString title(Kind kind);
			/// The id of @p kind in a project file: its name, "OpenWireEnds"
		static QString id(Kind kind);
		static bool kindFromId(const QString &id, Kind *kind);
			/// Where the check's own window is, with its options
		static QString toolTip(Kind kind);
		static Result run(QETProject *project, Kind kind);

			// Fork (plancheck/plancheck.cpp): every check in one pass, each
			// finding with its severity, for the panel, the script API and
			// --check-project alike
		static QVector<Result> runAll(QETProject *project);
		static QString key(Kind kind);              ///< "duplicate_labels", ...
		static QString severityKey(Severity severity);   ///< "error", "warning", "info"
		static QString severityText(Severity severity);  ///< translated
			/// the folio of a place, nullptr if none; builds no folio
		static Diagram *diagramOf(QETProject *project, const Place &place);
			/// one finding for scripts: {severity, check, rule, eplan,
			/// folio, folioTitle, label, text, items}; folio counted from 0
		static QVariantMap toVariant(const Result &result, const Finding &finding,
									 QETProject *project);
			/// the whole report: {project, ok, summary{errors, warnings,
			/// infos}, findings[]}; @p folio >= 0 keeps that folio only
		static QVariantList toVariantList(const QVector<Result> &results,
										  QETProject *project, int folio = -1);
		static QJsonObject toJson(const QVector<Result> &results, QETProject *project);

	private:
		static Result duplicateLabels(QETProject *project);
		static Result potentialTypes(QETProject *project);
		static Result openWireEnds(QETProject *project);
		static Result unwiredSymbols(QETProject *project);
		static Result terminalBranches(QETProject *project);
		static Result deviceTypes(QETProject *project);
		static Result wiresPerTerminal(QETProject *project);
		static Result terminalNames(QETProject *project);
			// Fork
		static bool isForkKind(Kind kind);
		static QString forkTitle(Kind kind);
		static QString forkToolTip(Kind kind);
		static Result wireInTwoCables(QETProject *project);
		static Result noPartNumber(QETProject *project);
		static Result unansweredPrompts(QETProject *project);
};

#endif // PROJECTCHECK_H
