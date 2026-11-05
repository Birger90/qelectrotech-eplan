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
#ifndef PANELSTEP_H
#define PANELSTEP_H

#include <QString>
#include <QVector>
#include <QVector3D>

/**
	@brief A triangle mesh in millimetres, read from a STEP file, for the
	3D view of a cabinet layout (manufacturer models of enclosures,
	mounting plates and devices).
*/
struct PanelMesh
{
	QVector<QVector3D> vertices;   ///< three per triangle
	QVector<QVector3D> normals;    ///< one per vertex
	QVector3D min, max;            ///< bounding box
	bool isEmpty() const {return vertices.isEmpty();}
};

/**
	@brief Reads STEP files with Open CASCADE (when QElectroTech is built
	with it) and keeps the meshes it made, so a model used ten times is
	read once.
*/
class PanelStep
{
	public:
		static bool available();
		static PanelMesh load(const QString &path, QString *error = nullptr);
};

#endif // PANELSTEP_H
