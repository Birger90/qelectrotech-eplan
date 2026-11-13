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
#include "drawnterminals.h"

#include "realterminal.h"
#include "terminalstrip.h"
#include "terminalstripautoimport.h"
#include "terminalarticlecatalog.h"
#include "physicalterminal.h"
#include "UndoCommand/groupterminalscommand.h"
#include <ElementsCollection/elementslocation.h>
#include <autobreakconductor.h>
#include <cable/cablemanager.h>
#include <diagram.h>
#include <elementprovider.h>
#include <factory/elementfactory.h>
#include <qetgraphicsitem/conductor.h>
#include <qetgraphicsitem/element.h>
#include <qetgraphicsitem/terminal.h>
#include <qetgraphicsitem/terminalelement.h>
#include <qetinformation.h>
#include <qetproject.h>
#include <undocommand/addgraphicsobjectcommand.h>
#include <QPropertyUndoCommand/qpropertyundocommand.h>
#include <conductorproperties.h>

#include <QCoreApplication>
#include <QSet>
#include <QUndoCommand>
#include <QUndoStack>
#include <algorithm>

namespace
{
		///Every terminal label of the project, free terminals and placed ones
	QStringList allTerminalLabels(QETProject *project)
	{
		QStringList labels_;
		if (!project) {
			return labels_;
		}
		const ElementProvider provider_(project);
		for (auto terminal_ : provider_.freeTerminal()) {
			if (terminal_ && terminal_->realTerminal()) {
				labels_ << terminal_->realTerminal()->label();
			}
		}
		for (auto strip_ : project->terminalStrip()) {
			if (!strip_) {
				continue;
			}
			for (const auto &real_ : strip_->realTerminals()) {
				if (real_) {
					labels_ << real_->label();
				}
			}
		}
		return labels_;
	}

		///"WAGO 2002-1201" -> {"WAGO", "2002-1201"}; a single word is the reference
	QPair<QString, QString> splitArticle(const QString &article)
	{
		const auto text_ = article.simplified();
		const auto space_ = text_.indexOf(QLatin1Char(' '));
		if (space_ < 0) {
			return {QString(), text_};
		}
		return {text_.left(space_), text_.mid(space_ + 1)};
	}
}

QList<DrawnTerminals::Row> DrawnTerminals::rowsFor(Diagram *diagram, const QLineF &line)
{
	QList<Row> rows_;
	if (!diagram) {
		return rows_;
	}

	QSet<Conductor *> seen_;
	for (const auto &crossing_ : CableManager::crossedConductors(diagram, line))
	{
		if (!crossing_.conductor || seen_.contains(crossing_.conductor)) {
			continue;	//One terminal per wire, even when the line meets it twice
		}
		seen_.insert(crossing_.conductor);
		Row row_;
		row_.conductor = crossing_.conductor;
		row_.position = crossing_.position;
		row_.pe = looksLikePe(crossing_.conductor);
		rows_ << row_;
	}

		//In the direction the user drew the line: from left to right
		//gives 1, 2, 3 from the left, from right to left from the right.
	const QPointF dir_ = line.p2() - line.p1();
	std::sort(rows_.begin(), rows_.end(), [&](const Row &a, const Row &b) {
		const QPointF da = a.position - line.p1();
		const QPointF db = b.position - line.p1();
		return da.x() * dir_.x() + da.y() * dir_.y()
			 < db.x() * dir_.x() + db.y() * dir_.y();
	});
	return rows_;
}

QStringList DrawnTerminals::stripNames(QETProject *project)
{
	QSet<QString> names_;
	if (project)
	{
		for (auto strip_ : project->terminalStrip()) {
			if (strip_ && !strip_->name().isEmpty()) {
				names_.insert(strip_->name());
			}
		}
		for (const auto &label_ : allTerminalLabels(project)) {
			const auto name_ = TerminalStripAutoImport::stripNameForLabel(label_);
			if (!name_.isEmpty()) {
				names_.insert(name_);
			}
		}
	}
	QStringList list_(names_.begin(), names_.end());
	std::sort(list_.begin(), list_.end(), [](const QString &a, const QString &b) {
		return QString::localeAwareCompare(a, b) < 0;
	});
	return list_;
}

QString DrawnTerminals::nextFreeStrip(QETProject *project)
{
	const auto used_ = stripNames(project);
		//Follow the project: "-X1" where the strips are written with the
		//prefix of IEC 81346, "X1" everywhere else
	const bool dash_ = std::any_of(used_.begin(), used_.end(), [](const QString &name) {
		return name.startsWith(QLatin1Char('-'));
	});
	const QString prefix_ = dash_ ? QStringLiteral("-X") : QStringLiteral("X");
	for (int i = 1; i < 10000; ++i)
	{
		const QString name_ = prefix_ + QString::number(i);
		if (!used_.contains(name_)) {
			return name_;
		}
	}
	return prefix_;
}

int DrawnTerminals::nextNumber(QETProject *project, const QString &strip)
{
	int highest_ = 0;
	const auto wanted_ = strip.trimmed();
	for (const auto &label_ : allTerminalLabels(project))
	{
		if (TerminalStripAutoImport::stripNameForLabel(label_) != wanted_) {
			continue;
		}
		const auto colon_ = label_.indexOf(QLatin1Char(':'));
		if (colon_ < 0) {
			continue;
		}
		bool ok_ = false;
		const int number_ = label_.mid(colon_ + 1).trimmed().toInt(&ok_);
		if (ok_) {
			highest_ = qMax(highest_, number_);
		}
	}
	return highest_ + 1;
}

bool DrawnTerminals::looksLikePe(const Conductor *conductor)
{
	if (!conductor) {
		return false;
	}
	const auto p_ = conductor->properties();
	const auto is_pe_ = [](const QString &text) {
		const auto t_ = text.trimmed().toUpper();
		return t_ == QLatin1String("PE") || t_.startsWith(QLatin1String("PE "));
	};
	if (is_pe_(p_.m_potential_type) || is_pe_(p_.text) || is_pe_(p_.m_function)) {
		return true;
	}
	const auto colour_ = p_.m_wire_color.toUpper().remove(QLatin1Char(' '));
	return colour_.contains(QLatin1String("GNYE"))
		|| colour_.contains(QLatin1String("GN/YE"))
		|| colour_.contains(QLatin1String("GNGE"))
		|| colour_.contains(QLatin1String("GN/GE"))
		|| colour_.contains(QString::fromUtf8("GRÜN-GELB"))
		|| colour_.contains(QLatin1String("GRUENGELB"));
}

QString DrawnTerminals::conductorText(const Conductor *conductor)
{
	if (!conductor) {
		return QString();
	}
	const auto p_ = conductor->properties();
	QStringList parts_;
	if (!p_.text.trimmed().isEmpty() && p_.text != QLatin1String("_")) {
		parts_ << p_.text.trimmed();
	}
	if (!p_.m_potential_type.isEmpty()) {
		parts_ << p_.m_potential_type;
	}
	if (!p_.m_wire_color.isEmpty()) {
		parts_ << p_.m_wire_color;
	}
	if (!p_.m_wire_section.isEmpty()) {
		parts_ << p_.m_wire_section;
	}
	return parts_.join(QStringLiteral(" · "));
}

QString DrawnTerminals::defaultElementPath()
{
	return QStringLiteral("common://10_electric/10_allpole/130_terminals_terminal_strips/borne_continuite2.elmt");
}

int DrawnTerminals::levelsOf(const QString &article)
{
	const auto a_ = TerminalArticleCatalog::find(article);
	return a_.isValid() ? qMax(1, a_.levels) : 1;
}

void DrawnTerminals::numberRows(QList<Row> &rows, int first, int levels)
{
	const int per_ = qMax(1, levels);
	for (int i = 0; i < rows.size(); ++i) {
		rows[i].number = QString::number(first + i / per_);
	}
}

DrawnTerminals::Result DrawnTerminals::place(Diagram *diagram, const QLineF &line,
											 const QList<Row> &rows, const Settings &settings)
{
	Result result_;
	QETProject *project_ = diagram ? diagram->project() : nullptr;
	if (!project_ || rows.isEmpty()) {
		return result_;
	}

	ElementsLocation location_(settings.element_path.isEmpty()
							   ? defaultElementPath() : settings.element_path);
	if (!location_.isElement() || !location_.exist()) {
		result_.problems << QCoreApplication::translate("DrawnTerminals", "The terminal symbol %1 does not exist.")
							.arg(settings.element_path);
		return result_;
	}
	ElementsLocation import_ = location_;
	if (!(location_.isProject() && location_.project() == project_)) {
		import_ = project_->importElement(location_);
	}
	if (!import_.exist()) {
		result_.problems << QCoreApplication::translate("DrawnTerminals", "The terminal symbol could not be taken into the project.");
		return result_;
	}

		//The wires run across the line: a horizontal line crosses
		//vertical wires and the terminal has to stand upright.
	const bool wires_vertical_ = qAbs(line.dy()) < qAbs(line.dx());

	const auto article_ = splitArticle(settings.article);
	const auto pe_article_ = splitArticle(settings.pe_article.isEmpty()
										  ? settings.article : settings.pe_article);

	auto *undo_ = new QUndoCommand(QCoreApplication::translate("DrawnTerminals", "Draw terminal strip %1").arg(settings.strip));
	QList<Conductor *> handled_;
	QSet<Terminal *> used_;
	QList<Element *> placed_;
		//The wire keeps what it was on both sides of its terminal: its
		//number, colour, cross-section and function
	QList<QPair<Element *, ConductorProperties>> wire_properties_;

	const int levels_ = qMax(1, settings.levels);
	int row_index_ = -1;
	for (const auto &row_ : rows)
	{
		++row_index_;
		if (!row_.conductor) {
			continue;
		}
			//The first level of a terminal carries the article: one
			//terminal of several levels is one article to order
		const bool first_level_ = (row_index_ % levels_) == 0;
		int state_ = 0;
		Element *element_ = ElementFactory::Instance()->createElement(import_, nullptr, &state_);
		if (state_ || !element_) {
			delete element_;
			result_.problems << QCoreApplication::translate("DrawnTerminals", "The terminal symbol could not be built.");
			break;
		}
		const auto terminals_ = element_->terminals();
		if (terminals_.size() < 2) {
			delete element_;
			result_.problems << QCoreApplication::translate("DrawnTerminals", "The terminal symbol needs two connection points.");
			break;
		}

			//Turn the symbol so its two connection points lie along the
			//wire, then move it so they sit either side of the crossing.
		element_->setPos(QPointF());
		element_->setRotation(0);
		QPointF a_ = terminals_.at(0)->dockConductor();
		QPointF b_ = terminals_.at(1)->dockConductor();
		const bool symbol_vertical_ = qAbs(b_.y() - a_.y()) >= qAbs(b_.x() - a_.x());
		if (symbol_vertical_ != wires_vertical_)
		{
			element_->setRotation(90);
			a_ = terminals_.at(0)->dockConductor();
			b_ = terminals_.at(1)->dockConductor();
		}
		element_->setPos(row_.position - (a_ + b_) / 2.0);

		DiagramContext info_ = element_->elementData().m_informations;
		const QString number_ = row_.number.trimmed();
		info_.addValue(QETInformation::ELMT_LABEL, settings.strip + QLatin1Char(':') + number_);
		info_.addValue(QETInformation::ELMT_FORMULA, QString());
			//A terminal of several levels takes PE on one of them: the
			//PE article is only for terminals of one level
		const bool own_pe_ = row_.pe && levels_ == 1;
		const auto &art_ = own_pe_ ? pe_article_ : article_;
		if (!art_.second.isEmpty() && first_level_)
		{
			info_.addValue(QETInformation::ELMT_MANUFACTURER, art_.first);
			info_.addValue(QETInformation::ELMT_MANUFACTURER_REF, art_.second);
		}
			//An article of the catalogue brings what the terminal strip
			//needs to be drawn at scale and listed: width, colour,
			//cross-section, designation
		const auto catalog_ = TerminalArticleCatalog::find(own_pe_ ? settings.pe_article.isEmpty()
																	 ? settings.article : settings.pe_article
																   : settings.article);
		if (catalog_.isValid())
		{
			if (first_level_)
			{
				info_.addValue(QETInformation::ELMT_MANUFACTURER, catalog_.manufacturer);
				info_.addValue(QETInformation::ELMT_MANUFACTURER_REF, catalog_.order_number);
				if (!catalog_.designation.isEmpty()) {
					info_.addValue(QETInformation::ELMT_DESIGNATION, catalog_.designation);
				}
			}
			if (catalog_.width > 0) {
				info_.addValue(RealTerminal::widthInfoKey(), QString::number(catalog_.width));
			}
			if (catalog_.section > 0) {
				info_.addValue(RealTerminal::sectionInfoKey(), QString::number(catalog_.section));
			}
			const auto color_ = TerminalArticleCatalog::colorSpec(catalog_);
			if (!color_.isEmpty()) {
				info_.addValue(RealTerminal::colorInfoKey(), color_);
			}
			const auto entry_ = TerminalArticleCatalog::drawingCatalogEntry(catalog_);
			if (!entry_.isEmpty()) {
				info_.addValue(RealTerminal::catalogEntryInfoKey(), entry_);
			}
		}
		element_->setElementInformations(info_);

		diagram->addItem(element_);
		new AddGraphicsObjectCommand(element_, diagram, element_->pos(), undo_);
		autoBreakConductors(diagram, element_, undo_, handled_, used_, true);
		placed_ << element_;
		wire_properties_ << qMakePair(element_, row_.conductor->properties());
	}

	if (placed_.isEmpty()) {
		delete undo_;
		return result_;
	}

		//One Ctrl+Z takes the terminals and their strip away together
	QUndoStack *stack_ = project_->undoStack();
	stack_->beginMacro(undo_->text());
	stack_->push(undo_);
	for (const auto &pair_ : wire_properties_)
	{
		for (Terminal *terminal_ : pair_.first->terminals())
		{
			for (Conductor *conductor_ : terminal_->conductors())
			{
				const ConductorProperties before_ = conductor_->properties();
				ConductorProperties after_ = pair_.second;
					//Cable membership stays with the piece itself
				after_.m_cable = before_.m_cable;
				after_.m_cable_uuid = before_.m_cable_uuid;
				if (after_ == before_) {
					continue;
				}
				QVariant old_value_, new_value_;
				old_value_.setValue(before_);
				new_value_.setValue(after_);
				stack_->push(new QPropertyUndoCommand(conductor_, "properties", old_value_, new_value_));
			}
		}
	}
	TerminalStripAutoImport::importFreeTerminals(project_);

		//Several levels: the wires of one terminal become the levels of
		//one physical terminal of the strip, in the order they were drawn
	if (levels_ > 1)
	{
		TerminalStrip *strip_ = nullptr;
		for (auto *s_ : project_->terminalStrip()) {
			if (s_ && s_->name() == settings.strip) {
				strip_ = s_;
				break;
			}
		}
		for (int first_ = 0; strip_ && first_ < placed_.size(); first_ += levels_)
		{
			auto *receiver_element_ = qobject_cast<TerminalElement *>(placed_.at(first_));
			if (!receiver_element_ || !receiver_element_->realTerminal()) {
				continue;
			}
			const auto receiver_ = receiver_element_->realTerminal()->physicalTerminal();
			QVector<QSharedPointer<RealTerminal>> others_;
			for (int i = first_ + 1; i < qMin(first_ + levels_, int(placed_.size())); ++i) {
				if (auto *t_ = qobject_cast<TerminalElement *>(placed_.at(i))) {
					if (t_->realTerminal()) {
						others_ << t_->realTerminal();
					}
				}
			}
			if (receiver_ && !others_.isEmpty()) {
				stack_->push(new GroupTerminalsCommand(strip_, receiver_, others_));
			}
		}
	}
	stack_->endMacro();

	result_.placed = placed_.size();
	for (auto *element_ : placed_)
	{
		const auto terminals_ = element_->terminals();
		const bool wired_ = std::all_of(terminals_.begin(), terminals_.end(), [](Terminal *t) {
			return !t->conductors().isEmpty();
		});
		if (!wired_) {
			++result_.unconnected;
		}
	}
	if (result_.unconnected) {
		result_.problems << QCoreApplication::translate("DrawnTerminals", "%n terminal(s) did not land on their wire (the wire is too short "
							   "there or bends at the crossing); they stand on the folio unconnected.",
							   nullptr, result_.unconnected);
	}
	return result_;
}
