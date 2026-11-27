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
#ifndef PANELE3D_H
#define PANELE3D_H

#include "panelstep.h"

#include <QByteArray>
#include <QString>

/**
	@brief Reads EPLAN's binary 3D format .e3d (the 3D of the Data Portal
	macros "..._3D.ema" and of the installation spaces of an .epdz) into
	triangles in millimetres, for the 3D view of the cabinet layout when a
	part has no STEP model.

	Format versions 1 to 4, little endian: version byte, model transform,
	view box, background colours, lights, textures, meshes (interleaved
	vertex array, face groups as points/lines/triangles/strips/fans with
	8 or 16 bit indices, edge groups), parts (a mesh placed with a 3x4
	transform; ids from version 3, text lines from version 4). Only the
	faces are taken; lines, edges, textures and texts are left out.

	Written after the format description and the reader of ecad-view
	(https://github.com/Canary-ECAD/ecad-view, docs/formats.md and
	packages/e3d-core/src/reader.ts, MIT, Copyright covagashi and
	contributors); the code here is our own.
*/
namespace PanelE3d
{
		/// @return the faces of @p data; empty with @p error when it is no E3D
	PanelMesh read(const QByteArray &data, QString *error = nullptr);
	PanelMesh readFile(const QString &path, QString *error = nullptr);
}

#endif // PANELE3D_H
