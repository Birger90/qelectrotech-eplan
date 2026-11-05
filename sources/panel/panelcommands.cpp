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
#include "panelcommands.h"

#include "../qetgraphicsitem/qetshapeitem.h"

#include <QObject>

ChangePanelDataCommand::ChangePanelDataCommand(QetShapeItem *shape, const PanelData &new_data,
											   QUndoCommand *parent) :
	QUndoCommand(parent),
	m_shape(shape),
	m_old(shape ? shape->panelData() : PanelData()),
	m_new(new_data)
{
	setText(QObject::tr("Edit cabinet layout data"));
}

void ChangePanelDataCommand::undo()
{
	if (m_shape) m_shape->setPanelData(m_old);
}

void ChangePanelDataCommand::redo()
{
	if (m_shape) m_shape->setPanelData(m_new);
}
