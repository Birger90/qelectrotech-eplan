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

#include "../qetapp.h"

#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QRegularExpression>
#include <QThread>
#include <QObject>

#ifdef QET_WITH_OCCT
#include <BRepBndLib.hxx>
#include <OSD.hxx>
#ifdef _WIN32
#include <float.h>
#include <algorithm>
#endif
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

/**
	@brief PanelStep::modelDirs
	The folders that hold the STEP models of the parts: QET_3D_DIR, the
	folder "3D" next to the company collection and the one next to the
	user collection. A model is kept there as <manufacturer>/<order number>.stp.
*/
QStringList PanelStep::modelDirs()
{
	QStringList dirs;
	const QString env = qEnvironmentVariable("QET_3D_DIR");
	if (!env.isEmpty()) dirs << QDir::fromNativeSeparators(env);
	for (const QString &collection : {QETApp::companyElementsDir(), QETApp::customElementsDir()}) {
		QDir d(collection);
		if (collection.isEmpty() || !d.cdUp()) continue;
		const QString p = d.filePath(QStringLiteral("3D"));
		if (QFileInfo(p).isDir() && !dirs.contains(p)) dirs << p;
	}
	return dirs;
}

namespace {
	/// an order number without its separators: 3RT2015-1BB41 = 3RT20151BB41
QString orderKey(const QString &s)
{
	static const QRegularExpression other(QStringLiteral("[^A-Z0-9]"));
	return s.toUpper().remove(other);
}
}

/**
	@brief PanelStep::findModel
	The STEP model of the part with the order number @p reference in the
	model folders, found with or without its dashes and dots; a model in
	the folder of @p manufacturer is taken first. Empty when there is none.
*/
QString PanelStep::findModel(const QString &reference, const QString &manufacturer)
{
	const QString key = orderKey(reference);
	if (key.isEmpty()) return QString();
		// order number -> files, made again when a model folder changed
	static QHash<QString, QStringList> index;
	static QHash<QString, QDateTime> stamps;
	const QStringList dirs = modelDirs();
	bool changed = stamps.size() != dirs.size();
	for (const QString &d : dirs) {
		QDateTime newest = QFileInfo(d).lastModified();
		for (const QFileInfo &sub : QDir(d).entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot))
			newest = qMax(newest, sub.lastModified());
		if (stamps.value(d) != newest) {changed = true; stamps.insert(d, newest);}
	}
	if (changed) {
		index.clear();
		for (const QString &d : dirs) {
			QDirIterator it(d, {QStringLiteral("*.stp"), QStringLiteral("*.step"), QStringLiteral("*.STP"), QStringLiteral("*.STEP")},
							QDir::Files, QDirIterator::Subdirectories);
			while (it.hasNext()) {
				const QString f = it.next();
				index[orderKey(QFileInfo(f).completeBaseName())] << f;
			}
		}
	}
	QStringList files = index.value(key);
	if (files.isEmpty()) {
			// same housing, other order number (e.g. SIRIUS by frame size):
			// "gleiche_modelle.txt" in a model folder, lines
			// "<order number or prefix*> = <order number of the model>"
		static QHash<QString, QString> same;			// key -> model key
		static QList<QPair<QString, QString>> prefixes;	// prefix -> model key
		if (changed || (same.isEmpty() && prefixes.isEmpty())) {
			same.clear();
			prefixes.clear();
			for (const QString &d : dirs) {
				QFile f(QDir(d).filePath(QStringLiteral("gleiche_modelle.txt")));
				if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) continue;
				const QStringList lines = QString::fromUtf8(f.readAll()).split(QLatin1Char('\n'));
				for (QString line : lines) {
					line = line.section(QLatin1Char('#'), 0, 0).trimmed();
					const int eq = line.indexOf(QLatin1Char('='));
					if (eq < 1) continue;
					const QString left = line.left(eq).trimmed();
					const QString model = orderKey(line.mid(eq + 1));
					if (left.endsWith(QLatin1Char('*')))
						prefixes << qMakePair(orderKey(left.chopped(1)), model);
					else
						same.insert(orderKey(left), model);
				}
			}
				// longest prefix first
			std::sort(prefixes.begin(), prefixes.end(),
					  [](const auto &a, const auto &b) {return a.first.size() > b.first.size();});
		}
		QString model = same.value(key);
		if (model.isEmpty())
			for (const auto &p : prefixes)
				if (key.startsWith(p.first)) {model = p.second; break;}
		files = index.value(model);
	}
	if (files.isEmpty()) return QString();
	const QString maker = orderKey(manufacturer);
	if (!maker.isEmpty())
		for (const QString &f : files)
			if (orderKey(QFileInfo(f).dir().dirName()).startsWith(maker.left(5))
				|| maker.startsWith(orderKey(QFileInfo(f).dir().dirName()).left(5)))
				return f;
	return files.first();
}

/**
	@brief PanelStep::thinnestSide
	The smallest side of the model's bounding box in mm (a terminal's width
	along the rail), 0 when the model cannot be read. Kept per file.
*/
double PanelStep::thinnestSide(const QString &path)
{
	static QHash<QString, double> sides;
	const QString key = QFileInfo(path).absoluteFilePath();
	auto it = sides.constFind(key);
	if (it != sides.constEnd()) return it.value();
	const PanelMesh mesh = load(path);
	double side = 0;
	if (!mesh.isEmpty()) {
		const QVector3D s = mesh.max - mesh.min;
		side = qMin(double(s.x()), qMin(double(s.y()), double(s.z())));
	}
	sides.insert(key, side);
	return side;
}

/**
	@brief PanelStep::size
	The bounding box of the model in mm (its own axes), null when it cannot
	be read. Kept per file.
*/
QVector3D PanelStep::size(const QString &path)
{
	static QHash<QString, QVector3D> sizes;
	const QString key = QFileInfo(path).absoluteFilePath();
	auto it = sizes.constFind(key);
	if (it != sizes.constEnd()) return it.value();
	const PanelMesh mesh = load(path);
	const QVector3D s = mesh.isEmpty() ? QVector3D() : mesh.max - mesh.min;
	sizes.insert(key, s);
	return s;
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
		// The STEP reader recurses deeply on big assemblies; on the main
		// thread (and inside a script) the default stack is too small and
		// some manufacturer models crash it. Read on a thread of its own
		// with a large stack.
	if (!QThread::currentThread()->property("qet_step_reader").toBool()) {
		QString thread_error;
		QThread *thread = QThread::create([&]() {
			QThread::currentThread()->setProperty("qet_step_reader", true);
				// The STEP translator relies on Open CASCADE turning access
				// violations inside it into exceptions it catches itself;
				// without its signal handling some models crash QElectroTech.
				// Floating point exceptions stay masked.
			static const bool signals_set = []() {OSD::SetSignal(Standard_False); return true;}();
			Q_UNUSED(signals_set)
			OSD::SetThreadLocalSignal(OSD_SignalMode_Set, Standard_False);
#ifdef _WIN32
			_clearfp();
			_controlfp(_MCW_EM, _MCW_EM);
#endif
			mesh = load(path, &thread_error);
		});
		thread->setStackSize(256u * 1024u * 1024u);
		thread->start();
		thread->wait();
		delete thread;
		if (error) *error = thread_error;
		return mesh;
	}
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
