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
#ifndef CONDUCTORENDLABELS_H
#define CONDUCTORENDLABELS_H

#include <QGraphicsObject>
#include <QList>
#include <QPointF>
#include <QTransform>

class Conductor;

/**
	@brief The ConductorEndLabels class
	The conductor's text (its number or mark) written in clear at each
	end of the conductor, along the wire, just above it. A conductor
	shows them when its properties ask for it
	(ConductorProperties::m_end_labels).
*/
class ConductorEndLabels : public QGraphicsObject
{
	Q_OBJECT

	public:
		explicit ConductorEndLabels(Conductor *conductor);

		enum { Type = UserType + 1018 };
		int type() const override { return Type; }

		QRectF boundingRect() const override;
		void paint(QPainter *painter,
			   const QStyleOptionGraphicsItem *option,
			   QWidget *widget) override;

		void refresh();

		static QTransform labelTransform(const QPointF &end,
						 const QPointF &inward,
						 qreal text_width,
						 QRectF *text_rect);
		static bool endOf(const QList<QPointF> &points, bool at_start,
				  QPointF *end, QPointF *inward);

	private:
		QList<QPointF> points() const;
		QString text() const;

		Conductor *m_conductor;
};

#endif // CONDUCTORENDLABELS_H
