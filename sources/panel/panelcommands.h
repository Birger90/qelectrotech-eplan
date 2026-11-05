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
#ifndef PANELCOMMANDS_H
#define PANELCOMMANDS_H

#include "paneldata.h"

#include <QPointer>
#include <QUndoCommand>

class QetShapeItem;

/**
	@brief Give a shape another meaning in the cabinet layout (mounting
	surface, DIN rail, duct, cut-out ...) or other manufacturing data, as
	one undo step.
*/
class ChangePanelDataCommand : public QUndoCommand
{
	public:
		ChangePanelDataCommand(QetShapeItem *shape, const PanelData &new_data,
							   QUndoCommand *parent = nullptr);
		void undo() override;
		void redo() override;

	private:
		QPointer<QetShapeItem> m_shape;
		PanelData m_old, m_new;
};

#endif // PANELCOMMANDS_H
