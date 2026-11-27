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
#include "conductorendlabels.h"

#include <conductorsegment.h>
#include <diagram.h>
#include "conductor.h"
#include "conductortextitem.h"
#include <shownkinds.h>

#include <QFontMetricsF>
#include <QPainter>
#include <QtMath>

namespace {
		/// Room between the terminal and the text
	const qreal end_gap = 3.0;
		/// Room between the wire and the text
	const qreal wire_gap = 1.0;
}

ConductorEndLabels::ConductorEndLabels(Conductor *conductor) :
	QGraphicsObject(conductor),
	m_conductor(conductor)
{
		//Shown and hidden with the wire numbers (View > Show)
	ShownKinds::tag(this, ShownKinds::WireNumbers);
	setAcceptedMouseButtons(Qt::NoButton);
	connect(conductor->textItem()->document(),
		&QTextDocument::contentsChanged,
		this, &ConductorEndLabels::refresh);
	refresh();
}

/**
	@return the corners of the conductor, from its start to its end
*/
QList<QPointF> ConductorEndLabels::points() const
{
	QList<QPointF> list;
	const QList<ConductorSegment *> segments = m_conductor->segmentsList();
	if (segments.isEmpty())
		return list;
	list << segments.first()->firstPoint();
	for (ConductorSegment *segment : segments)
		list << segment->secondPoint();
	return list;
}

/**
	@return the text to write; empty when the conductor has none, or only
	the "_" placeholder
*/
QString ConductorEndLabels::text() const
{
	const QString text = m_conductor->textItem()->toPlainText().trimmed();
	return text == QLatin1String("_") ? QString() : text;
}

/**
	Find the end of the polyline @p points and the direction the wire
	leaves it by, skipping stretches of no length.
	@param at_start : the first end of the polyline, or else the last
	@param end : the end
	@param inward : the direction from the end into the wire
	@return false when the polyline has no length
*/
bool ConductorEndLabels::endOf(const QList<QPointF> &points, bool at_start,
			       QPointF *end, QPointF *inward)
{
	if (points.size() < 2)
		return false;
	const QPointF origin = at_start ? points.first() : points.last();
	for (int i = 1 ; i < points.size() ; ++i)
	{
		const QPointF other = at_start ? points.at(i)
					       : points.at(points.size() - 1 - i);
		const QPointF d = other - origin;
		if (!qFuzzyIsNull(d.x()) || !qFuzzyIsNull(d.y())) {
			*end = origin;
			*inward = d;
			return true;
		}
	}
	return false;
}

/**
	@return the transformation that puts a text of @p text_width, written
	from left to right, at the end @p end of a wire leaving it by
	@p inward: along the wire, never upside down and, on a vertical
	wire, read from bottom to top. @p text_rect gets the place of the text
	in that transformation, above the wire.
*/
QTransform ConductorEndLabels::labelTransform(const QPointF &end,
					      const QPointF &inward,
					      qreal text_width,
					      QRectF *text_rect)
{
	qreal angle = qRadiansToDegrees(qAtan2(inward.y(), inward.x()));
	bool flipped = false;
	if (angle >= 90) {
		angle -= 180;
		flipped = true;
	} else if (angle < -90) {
		angle += 180;
		flipped = true;
	}
		//Reading along the wire: the text lies inward. Reading against
		//it: toward the terminal, so the other way round
	const qreal x = flipped ? -(end_gap + text_width) : end_gap;
	if (text_rect)
		*text_rect = QRectF(x, 0, text_width, 0);
	QTransform transform;
	transform.translate(end.x(), end.y());
	transform.rotate(angle);
	return transform;
}

void ConductorEndLabels::refresh()
{
	prepareGeometryChange();
	update();
}

QRectF ConductorEndLabels::boundingRect() const
{
	const QString label = text();
	if (label.isEmpty())
		return QRectF();
	const QFontMetricsF metrics(m_conductor->textItem()->font());
	const qreal width = metrics.horizontalAdvance(label);
	const qreal height = metrics.height();
	const QList<QPointF> list = points();

	QRectF bounds;
	for (bool at_start : {true, false})
	{
		QPointF end, inward;
		if (!endOf(list, at_start, &end, &inward))
			continue;
		QRectF local;
		const QTransform transform = labelTransform(end, inward, width, &local);
		local.setTop(-height - wire_gap);
		local.setBottom(-wire_gap);
		bounds = bounds.united(transform.mapRect(local));
	}
	return bounds.adjusted(-1, -1, 1, 1);
}

void ConductorEndLabels::paint(QPainter *painter,
			       const QStyleOptionGraphicsItem *option,
			       QWidget *widget)
{
	Q_UNUSED(option)
	Q_UNUSED(widget)
	const QString label = text();
	if (label.isEmpty())
		return;
	const QFontMetricsF metrics(m_conductor->textItem()->font());
	const qreal width = metrics.horizontalAdvance(label);
	const qreal height = metrics.height();
	const QList<QPointF> list = points();

	painter->save();
	painter->setFont(m_conductor->textItem()->font());
	painter->setPen(m_conductor->textItem()->defaultTextColor());
	for (bool at_start : {true, false})
	{
		QPointF end, inward;
		if (!endOf(list, at_start, &end, &inward))
			continue;
		QRectF local;
		const QTransform transform = labelTransform(end, inward, width, &local);
		local.setTop(-height - wire_gap);
		local.setBottom(-wire_gap);
		painter->save();
		painter->setTransform(transform, true);
		painter->drawText(local, Qt::AlignLeft | Qt::AlignVCenter, label);
		painter->restore();
	}
	painter->restore();
}
