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
#include "reportelement.h"

#include <diagram.h>
#include <diagramposition.h>
#include <qetgraphicsitem/conductor.h>
#include <qetgraphicsitem/terminal.h>
#include <qetproject.h>
#include "dynamicelementtextitem.h"

ReportElement::ReportElement(const ElementsLocation &location, const QString& link_type,QGraphicsItem *qgi, int *state) :
	Element(location, qgi, state,
			link_type == "next_report"? Element::NextReport : Element::PreviousReport),
	m_inverse_report(link_type == "next_report"? Element::PreviousReport : Element::NextReport)
{}

/**
	@brief ReportElement::~ReportElement
	Destructor
*/
ReportElement::~ReportElement()
{
	unlinkAllElements();
	if (terminals().size())
		disconnect(terminals().first(), nullptr, nullptr, nullptr);
}

/**
	@brief ReportElement::linkToElement
	Link this element to the other element.
	A report links to one report of the other kind, and a new link replaces
	the one it had. A star (@sa Element::isReportStar()) keeps every report
	of the other kind it is given: one source going out to many sheets.
	@param elmt
	element to be linked with this
*/
void ReportElement::linkToElement(Element * elmt)
{
	if (!diagram() && !elmt -> diagram())
	{
		qDebug() << "ReportElement : linkToElement : Unable to link this or element to link isn't in a diagram";
		return;
	}

		//ensure elmt is an inverse report of this element
	if (!elmt || elmt->linkType() != m_inverse_report)
		return;

	if (isReportStar())
	{
		if (connected_elements.contains(elmt))
			return;
		connected_elements << elmt;
		elmt->linkToElement(this);
		emit linkedElementChanged();
		return;
	}

		//ensure elmt isn't already linked
	if (!connected_elements.isEmpty() && (connected_elements.first() == elmt))
		return;

	unlinkAllElements();
	connected_elements << elmt;
	elmt->linkToElement(this);
	emit linkedElementChanged();
}

/**
	@brief ReportElement::unLinkAllElements
	Unlink all of the element in the QList connected_elements.
	Each partner lets go of this report only: a star keeps its other
	reports.
*/
void ReportElement::unlinkAllElements()
{
	if (connected_elements.isEmpty())
		return;

	const QList <Element *> tmp_elmt = connected_elements;
	connected_elements.clear();

	for(Element *elmt : tmp_elmt)
	{
		elmt -> setHighlighted(false);
		elmt -> unlinkElement(this);
	}
	
	emit linkedElementChanged();
}

/**
	@brief ReportElement::unlinkElement
	Unlink the specified element. A report that is not a star has only one
	linked element, so the list is cleared; a star lets go of @p elmt only.
	@param elmt
*/
void ReportElement::unlinkElement(Element *elmt)
{
	if (!isReportStar()) {
		unlinkAllElements();
		return;
	}
	if (!connected_elements.contains(elmt))
		return;
	connected_elements.removeAll(elmt);
	elmt->setHighlighted(false);
	elmt->unlinkElement(this);
	emit linkedElementChanged();
}

/**
	@brief ReportElement::setReportStar
	A report which stops being a star keeps the first of its links only.
*/
void ReportElement::setReportStar(bool star)
{
	if (!star && connected_elements.size() > 1) {
		const QList<Element *> others = connected_elements.mid(1);
		for (Element *elmt : others)
			unlinkElement(elmt);
	}
	Element::setReportStar(star);
}
