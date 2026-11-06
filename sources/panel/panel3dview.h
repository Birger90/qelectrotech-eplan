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
#ifndef PANEL3DVIEW_H
#define PANEL3DVIEW_H

#include "panelstep.h"

#include <QColor>
#include <QDialog>
#include <QPointer>
#include <QVector3D>

#ifdef QET_WITH_OPENGL
#include <QOpenGLWidget>
#else
#include <QWidget>
#endif

class Diagram;
class QLabel;

/**
	@brief One body of the 3D scene: a box or a STEP mesh placed in
	millimetres (x right, y up, z out of the mounting surface), with the
	drawing item it stands for so a click selects that item on the folio.
*/
struct Panel3DBody
{
	QVector3D min, max;             ///< box, or where the mesh is fitted
	QColor colour;
	float opacity = 1.0f;           ///< below 1: drawn see-through (an enclosure)
	PanelMesh mesh;                 ///< empty: draw the box
	QVector3D mesh_offset;          ///< added to the mesh vertices
	QList<QVector3D> holes;         ///< hole centres on the top face of a surface
	QList<float> hole_radius;
	QPointer<QObject> item;         ///< the QetShapeItem or Element
	QString name;
};

#ifdef QET_WITH_OPENGL
using Panel3DBase = QOpenGLWidget;
#else
using Panel3DBase = QWidget;
#endif

/**
	@brief The 3D view of the cabinet layout of one folio: surfaces as
	plates with their holes, DIN rails, ducts and devices as boxes from
	their sizes -- or the manufacturer's STEP model when the part names
	one. Drag to turn, right drag to move, wheel to zoom.
*/
class Panel3DWidget : public Panel3DBase
{
	Q_OBJECT

	public:
		explicit Panel3DWidget(QWidget *parent = nullptr);
		void setBodies(const QList<Panel3DBody> &bodies);
		void viewFront();
		void viewTop();
		void viewIso();

	signals:
		void picked(QObject *item);

	protected:
#ifdef QET_WITH_OPENGL
		void initializeGL() override;
		void resizeGL(int w, int h) override;
		void paintGL() override;
#endif
		void mousePressEvent(QMouseEvent *event) override;
		void mouseMoveEvent(QMouseEvent *event) override;
		void mouseReleaseEvent(QMouseEvent *event) override;
		void wheelEvent(QWheelEvent *event) override;

	private:
		void fit();
		int pick(const QPoint &pos) const;

		QList<Panel3DBody> m_bodies;
		QVector3D m_centre;
		float m_distance = 2000, m_yaw = -25, m_pitch = 20;
		QVector3D m_pan;
		QPoint m_last, m_press;
		int m_selected = -1;
};

/**
	@brief The window around the 3D view, with the standard view points
	and a line saying what was read.
*/
class Panel3DDialog : public QDialog
{
	Q_OBJECT

	public:
		explicit Panel3DDialog(Diagram *diagram, QWidget *parent = nullptr);
		static QList<Panel3DBody> build(Diagram *diagram, QStringList *notes, bool enclosure = true);
		static bool writeGlb(const QList<Panel3DBody> &bodies, const QString &path, QString *error = nullptr);

	private:
		void reload();

		QPointer<Diagram> m_diagram;
		Panel3DWidget *m_view = nullptr;
		QLabel *m_status = nullptr;
		class QCheckBox *m_enclosure = nullptr;
};

#endif // PANEL3DVIEW_H
