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
#include "panelstep.h"

#include <QFileInfo>
#include <QHash>
#include <QObject>

#ifdef QET_WITH_OCCT
#include <BRepBndLib.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRep_Tool.hxx>
#include <Bnd_Box.hxx>
#include <Poly_Triangulation.hxx>
#include <STEPControl_Reader.hxx>
#include <Standard_Failure.hxx>
#include <TopExp_Explorer.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#endif

namespace {
QHash<QString, PanelMesh> &cache()
{
	static QHash<QString, PanelMesh> c;
	return c;
}
}

bool PanelStep::available()
{
#ifdef QET_WITH_OCCT
	return true;
#else
	return false;
#endif
}

/**
	@brief PanelStep::load
	Read @p path, mesh every face (chord tolerance about a 500th of the
	model size, at least 0.1 mm) and return the triangles in millimetres.
	An empty mesh says it could not be read; @p error says why.
*/
PanelMesh PanelStep::load(const QString &path, QString *error)
{
	const QString key = QFileInfo(path).absoluteFilePath();
	auto it = cache().constFind(key);
	if (it != cache().constEnd()) return it.value();

	PanelMesh mesh;
#ifdef QET_WITH_OCCT
	if (!QFileInfo::exists(path)) {
		if (error) *error = QObject::tr("File not found: %1").arg(path);
		return mesh;
	}
	try {
		STEPControl_Reader reader;
		if (reader.ReadFile(path.toUtf8().constData()) != IFSelect_RetDone) {
			if (error) *error = QObject::tr("Not a readable STEP file: %1").arg(path);
			return mesh;
		}
		reader.TransferRoots();
		const TopoDS_Shape shape = reader.OneShape();
		if (shape.IsNull()) {
			if (error) *error = QObject::tr("The STEP file holds no shape: %1").arg(path);
			return mesh;
		}
		Bnd_Box box;
		BRepBndLib::Add(shape, box);
		double x0, y0, z0, x1, y1, z1;
		box.Get(x0, y0, z0, x1, y1, z1);
		const double size = QVector3D(float(x1 - x0), float(y1 - y0), float(z1 - z0)).length();
		BRepMesh_IncrementalMesh mesher(shape, qMax(0.1, size / 500.0), false, 0.5);
		Q_UNUSED(mesher)

		QVector3D lo(1e30f, 1e30f, 1e30f), hi(-1e30f, -1e30f, -1e30f);
		for (TopExp_Explorer ex(shape, TopAbs_FACE); ex.More(); ex.Next())
		{
			const TopoDS_Face face = TopoDS::Face(ex.Current());
			TopLoc_Location location;
			const Handle(Poly_Triangulation) tri = BRep_Tool::Triangulation(face, location);
			if (tri.IsNull()) continue;
			const gp_Trsf trsf = location.Transformation();
			const bool reversed = face.Orientation() == TopAbs_REVERSED;
			for (int i = 1; i <= tri->NbTriangles(); ++i)
			{
				int n1, n2, n3;
				tri->Triangle(i).Get(n1, n2, n3);
				if (reversed) std::swap(n2, n3);
				QVector3D p[3];
				const int idx[3] = {n1, n2, n3};
				for (int k = 0; k < 3; ++k) {
					const gp_Pnt q = tri->Node(idx[k]).Transformed(trsf);
					p[k] = QVector3D(float(q.X()), float(q.Y()), float(q.Z()));
					lo.setX(qMin(lo.x(), p[k].x())); lo.setY(qMin(lo.y(), p[k].y())); lo.setZ(qMin(lo.z(), p[k].z()));
					hi.setX(qMax(hi.x(), p[k].x())); hi.setY(qMax(hi.y(), p[k].y())); hi.setZ(qMax(hi.z(), p[k].z()));
				}
				const QVector3D n = QVector3D::normal(p[0], p[1], p[2]);
				for (int k = 0; k < 3; ++k) {mesh.vertices << p[k]; mesh.normals << n;}
			}
		}
		mesh.min = lo;
		mesh.max = hi;
		if (mesh.isEmpty() && error) *error = QObject::tr("The STEP file has no faces to show: %1").arg(path);
	} catch (const Standard_Failure &failure) {
		if (error) *error = QObject::tr("Open CASCADE could not read %1: %2")
								.arg(path, QString::fromUtf8(failure.GetMessageString()));
		mesh = PanelMesh();
	}
#else
	if (error) *error = QObject::tr("This QElectroTech was built without STEP support (Open CASCADE).");
#endif
	if (!mesh.isEmpty()) cache().insert(key, mesh);
	return mesh;
}
