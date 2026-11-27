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
#include "pairrepresentation.h"

#include "addgraphicsobjectcommand.h"
#include "linkelementcommand.h"
#include "../diagram.h"
#include "../factory/elementfactory.h"
#include "../qetgraphicsitem/element.h"
#include "../qetproject.h"

#include <QCoreApplication>
#include <QUndoCommand>

Element *PairRepresentation::masterOf(const Element *contact)
{
	if (!contact) return nullptr;
	for (Element *elmt : contact->linkedElements())
		if (elmt->linkType() == Element::Master) return elmt;
	return nullptr;
}

Element *PairRepresentation::realOf(const Element *representation)
{
	if (!representation || !representation->isPairRepresentation()) return nullptr;
	Element *master = masterOf(representation);
	if (!master) return nullptr;
	for (Element *elmt : master->allLinkedElements())
		if (elmt && elmt->uuid() == representation->pairOf() && elmt->standsInProject())
			return elmt;
	return nullptr;
}

QList<Element *> PairRepresentation::representationsOf(const Element *real)
{
	QList<Element *> out;
	if (!real || real->isPairRepresentation()) return out;
	Element *master = masterOf(real);
	if (!master) return out;
	for (Element *elmt : master->allLinkedElements())
		if (elmt && elmt->pairOf() == real->uuid() && elmt->standsInProject())
			out << elmt;
	return out;
}

Element *PairRepresentation::create(Element *real, Diagram *folio, const QPointF &pos,
									QString *problem)
{
	const auto fail = [problem](const QString &text) -> Element * {
		if (problem) *problem = text;
		return nullptr;
	};
	if (!real || !folio || !folio->project())
		return fail(QCoreApplication::translate("PairRepresentation", "No contact or no folio"));
	if (real->linkType() != Element::Slave || real->isPairRepresentation())
		return fail(QCoreApplication::translate("PairRepresentation",
				"Only a contact can be shown as a pair cross-reference"));
	Element *master = masterOf(real);
	if (!master)
		return fail(QCoreApplication::translate("PairRepresentation",
				"Link the contact to its coil or main function first"));
	if (folio->project()->isReadOnly())
		return fail(QCoreApplication::translate("PairRepresentation", "The project is read-only"));

	int state = 0;
	Element *rep = ElementFactory::Instance()->createElement(real->location(), nullptr, &state);
	if (state || !rep) {
		delete rep;
		return fail(QCoreApplication::translate("PairRepresentation",
				"The symbol of the contact cannot be built"));
	}
	rep->setPos(pos);
	rep->setRotation(real->rotation());
	rep->setPairOf(real->uuid());
	folio->addItem(rep);

	auto *macro = new QUndoCommand(QCoreApplication::translate(
			"PairRepresentation", "Pair cross-reference of %1").arg(real->actualLabel()));
	new AddGraphicsObjectCommand(rep, folio, pos, macro);
	auto *link = new LinkElementCommand(rep, macro);
	link->setLink(master);
	const int group = master->groupIndexForElement(real);
	if (group >= 0) link->setGroupIndex(group);
	folio->project()->undoStack()->push(macro);
		//The real contact names its new partner in its cross-reference
	emit real->linkedElementChanged();
	return rep;
}
