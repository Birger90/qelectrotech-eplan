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
#include "fluidbomproject.h"

#include "fluidbom.h"
#include "../diagram.h"
#include "../diagramcontext.h"
#include "../qetgraphicsitem/conductor.h"
#include "../qetgraphicsitem/element.h"
#include "../qetproject.h"

QByteArray FluidBom::projectCsv(QETProject &project, int *parts,
								int *lines, int *rows)
{
	QList<Part> part_list;
	QList<Line> line_list;
	for (Diagram *diagram : project.diagrams()) {
		for (Element *element : diagram->elements()) {
			const FluidData fluid = element->elementData().m_fluid;
			if (fluid.isNull())
				continue;
			const DiagramContext info = element->elementInformations();
			Part p;
			p.name = element->name();
			p.kind = kindText(fluid.m_kind);
			p.details = details(fluid);
			p.manufacturer = info.value(
						QStringLiteral("manufacturer")).toString();
			p.reference = info.value(
						QStringLiteral("manufacturer_reference")).toString();
			p.label = element->actualLabel();
			part_list << p;
		}
		for (Conductor *conductor : diagram->conductors()) {
			const ConductorProperties cp = conductor->properties();
			if (cp.m_fluid_line == Fluid::LineKind::None)
				continue;
			line_list << Line{cp.m_fluid_line, cp.m_wire_section};
		}
	}
	if (parts)
		*parts = part_list.size();
	if (lines)
		*lines = line_list.size();
	return csv(part_list, line_list, rows);
}
