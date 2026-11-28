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
#ifndef PAIRREPRESENTATION_H
#define PAIRREPRESENTATION_H

#include <QList>
#include <QPointF>
#include <QString>

class Diagram;
class Element;

/**
	@brief The PairRepresentation namespace
	Pair cross-references (EPLAN: "Paarquerverweis"): a contact wired on
	one folio shown on another as a drawing only, an auxiliary contact of a
	motor protection switch beside its main contacts say. The
	representation is a copy of the contact's symbol, linked to the same
	master with the same contact group, marked with the contact it shows
	(Element::pairOf()). Its label and terminal names are the contact's;
	the master's contact mirror, its counts and the lists leave it out
	(Element::linkedElements()); the representation points at the real
	contact and the real contact at its representations.
*/
namespace PairRepresentation
{
		/// the master a contact is linked to, nullptr if none
	Element *masterOf(const Element *contact);
		/// the contact @p representation shows, nullptr when gone
	Element *realOf(const Element *representation);
		/// the representations of the contact @p real
	QList<Element *> representationsOf(const Element *real);
		/// Put a representation of @p real on @p folio at @p pos, one undo
		/// step. @return it, nullptr with @p problem said when it cannot be
	Element *create(Element *real, Diagram *folio, const QPointF &pos,
					QString *problem = nullptr);
}

#endif // PAIRREPRESENTATION_H
