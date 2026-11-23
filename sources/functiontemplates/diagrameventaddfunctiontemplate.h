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
#ifndef DIAGRAMEVENTADDFUNCTIONTEMPLATE_H
#define DIAGRAMEVENTADDFUNCTIONTEMPLATE_H

#include "../diagramevent/diagrameventinterface.h"
#include "functiontemplate.h"

#include <QMap>
#include <QPointer>

class QGraphicsPixmapItem;
class QStatusBar;

/**
	@brief Places a function template where the user clicks.
	Its picture, filled with the values chosen, follows the cursor on
	the grid; a left click puts it down (and the next click another one,
	with the labels counted on), a right click or Escape stops.
*/
class DiagramEventAddFunctionTemplate : public DiagramEventInterface
{
	Q_OBJECT

	public:
		DiagramEventAddFunctionTemplate(const FunctionTemplate &tpl,
										const QMap<QString, QString> &values,
										Diagram *diagram, QPointF pos = QPointF());
		~DiagramEventAddFunctionTemplate() override;

		void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
		void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
		void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
		void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
		void keyPressEvent(QKeyEvent *event) override;
		void init() override;

	signals:
		void placed(const FunctionTemplate::Placed &placed);

	private:
		void stop();

		FunctionTemplate m_template;
		QMap<QString, QString> m_values;
		QGraphicsPixmapItem *m_preview = nullptr;
		QPointer<QStatusBar> m_status_bar;
};

#endif // DIAGRAMEVENTADDFUNCTIONTEMPLATE_H
