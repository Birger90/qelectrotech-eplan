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
#include "diagrameventaddfunctiontemplate.h"

#include "../diagram.h"
#include "../qetapp.h"
#include "../qetdiagrameditor.h"

#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QPixmap>
#include <QStatusBar>

DiagramEventAddFunctionTemplate::DiagramEventAddFunctionTemplate(const FunctionTemplate &tpl,
																 const QMap<QString, QString> &values,
																 Diagram *diagram, QPointF pos) :
	DiagramEventInterface(diagram),
	m_template(tpl),
	m_values(values)
{
	const QImage image = m_template.preview(m_values, QSize());
	if (image.isNull()) return;
	m_preview = new QGraphicsPixmapItem(QPixmap::fromImage(image));
		//The picture has a margin of 10 around the drawing, whose top
		//left corner goes where the user clicks
	m_preview->setOffset(-10, -10);
	m_preview->setOpacity(0.6);
	m_preview->setZValue(1e6);
	m_preview->setPos(Diagram::snapToGrid(pos));
	m_diagram->addItem(m_preview);
	m_running = true;
	init();

	if (!diagram->views().isEmpty())
		if (QETDiagramEditor *editor = QETApp::diagramEditorAncestorOf(diagram->views().first()))
			m_status_bar = editor->statusBar();
	if (m_status_bar)
		m_status_bar->showMessage(tr("Click to place \"%1\", right click or Escape to stop.")
								  .arg(m_template.name()));
}

DiagramEventAddFunctionTemplate::~DiagramEventAddFunctionTemplate()
{
	if (m_preview) {
		m_diagram->removeItem(m_preview);
		delete m_preview;
	}
	if (m_status_bar) m_status_bar->clearMessage();
	for (QGraphicsView *view : m_diagram->views())
		view->setContextMenuPolicy(Qt::DefaultContextMenu);
}

void DiagramEventAddFunctionTemplate::init()
{
	for (QGraphicsView *view : m_diagram->views())
		view->setContextMenuPolicy(Qt::NoContextMenu);
}

void DiagramEventAddFunctionTemplate::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	if (m_preview) m_preview->setPos(Diagram::snapToGrid(event->scenePos()));
	event->setAccepted(true);
}

void DiagramEventAddFunctionTemplate::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	event->setAccepted(true);
}

void DiagramEventAddFunctionTemplate::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	event->setAccepted(true);
	if (!m_preview) return;
	if (event->button() == Qt::RightButton) {
		stop();
		return;
	}
	if (event->button() != Qt::LeftButton) return;

	QPointF pos = Diagram::snapToGrid(event->scenePos());
		//A null position keeps the places the template was saved at
	if (pos.isNull()) pos = QPointF(0.1, 0.1);
	m_preview->hide();
	const FunctionTemplate::Placed result = m_template.place(m_diagram, m_values, pos);
	m_preview->show();
	emit placed(result);
	if (m_status_bar) {
		if (!result.ok) m_status_bar->showMessage(result.error);
		else if (!result.notes.isEmpty()) m_status_bar->showMessage(result.notes.join(QStringLiteral(" ")));
	}

		//The next click places another one: its labels and terminals are
		//proposed again, counted on from the ones just placed
	for (const FunctionTemplate::Parameter &p : m_template.parameters())
		if (p.kind == QLatin1String("label") || p.kind == QLatin1String("terminal"))
			m_values.remove(p.name);
}

void DiagramEventAddFunctionTemplate::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
	event->setAccepted(true);
}

void DiagramEventAddFunctionTemplate::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Escape) {
		stop();
		event->setAccepted(true);
		return;
	}
	DiagramEventInterface::keyPressEvent(event);
}

void DiagramEventAddFunctionTemplate::stop()
{
	if (m_preview) {
		m_diagram->removeItem(m_preview);
		delete m_preview;
		m_preview = nullptr;
	}
	m_running = false;
	emit finish();
}
