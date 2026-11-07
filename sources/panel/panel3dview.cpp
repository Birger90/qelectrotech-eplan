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
#include "panel3dview.h"

#include "panellayout.h"
#include "../diagram.h"
#include "../qetproject.h"
#include "../qetgraphicsitem/element.h"
#include "../qetgraphicsitem/qetshapeitem.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QFileInfo>
#include <QGraphicsView>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QMatrix4x4>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <QtMath>

#ifdef QET_WITH_OPENGL
#include <QOpenGLFunctions_1_1>
#include <QOpenGLVersionFunctionsFactory>
#endif

#include <algorithm>
#include <array>

namespace {

QMatrix4x4 projection(int w, int h, float distance)
{
	QMatrix4x4 m;
	m.perspective(35.0f, h > 0 ? float(w) / float(h) : 1.0f,
				  qMax(1.0f, distance / 100.0f), distance * 20.0f);
	return m;
}

QMatrix4x4 modelView(const QVector3D &centre, const QVector3D &pan,
					 float distance, float yaw, float pitch)
{
	QMatrix4x4 m;
	m.translate(0, 0, -distance);
	m.rotate(pitch, 1, 0, 0);
	m.rotate(yaw, 0, 1, 0);
		// model: x right, y up, z out of the surface (towards the viewer)
	m.translate(-(centre + pan));
	return m;
}

	/// ray / box test (slab method), returns the distance or -1
float hitBox(const QVector3D &o, const QVector3D &d, const QVector3D &lo, const QVector3D &hi)
{
	float t0 = -1e30f, t1 = 1e30f;
	for (int i = 0; i < 3; ++i) {
		if (qAbs(d[i]) < 1e-9f) {
			if (o[i] < lo[i] || o[i] > hi[i]) return -1;
			continue;
		}
		float a = (lo[i] - o[i]) / d[i], b = (hi[i] - o[i]) / d[i];
		if (a > b) std::swap(a, b);
		t0 = qMax(t0, a);
		t1 = qMin(t1, b);
		if (t0 > t1) return -1;
	}
	return t1 < 0 ? -1 : qMax(t0, 0.0f);
}

/**
	Turn a manufacturer model so its extents fit the device's width,
	height and depth best (models come in many axis conventions) and move
	it into the box: centred in x/y, standing on the bottom of the box.
*/
/// add a box (12 triangles) to a mesh
void appendBox(PanelMesh &m, const QVector3D &lo, const QVector3D &hi)
{
	const QVector3D c[8] = {{lo.x(), lo.y(), lo.z()}, {hi.x(), lo.y(), lo.z()}, {hi.x(), hi.y(), lo.z()}, {lo.x(), hi.y(), lo.z()},
							{lo.x(), lo.y(), hi.z()}, {hi.x(), lo.y(), hi.z()}, {hi.x(), hi.y(), hi.z()}, {lo.x(), hi.y(), hi.z()}};
	const int f[6][4] = {{4, 5, 6, 7}, {1, 0, 3, 2}, {5, 1, 2, 6}, {0, 4, 7, 3}, {7, 6, 2, 3}, {0, 1, 5, 4}};
	const QVector3D n[6] = {{0, 0, 1}, {0, 0, -1}, {1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}};
	for (int s = 0; s < 6; ++s)
		for (int t : {0, 1, 2, 0, 2, 3}) {m.vertices << c[f[s][t]]; m.normals << n[s];}
	if (m.vertices.size() == 36) {m.min = lo; m.max = hi;}
	for (int k = 0; k < 3; ++k) {m.min[k] = qMin(m.min[k], lo[k]); m.max[k] = qMax(m.max[k], hi[k]);}
}

PanelMesh fitMesh(const PanelMesh &in, const QVector3D &lo, const QVector3D &hi)
{
	const QVector3D want = hi - lo;
	const QVector3D have = in.max - in.min;
	static const std::array<std::array<int, 3>, 6> perms = {{
		{{0, 1, 2}}, {{0, 2, 1}}, {{1, 0, 2}}, {{1, 2, 0}}, {{2, 0, 1}}, {{2, 1, 0}}}};
	int best = 0;
	float best_err = 1e30f;
	for (int p = 0; p < 6; ++p) {
		float err = 0;
			// no depth given (want z = 0): only the width and height decide,
			// the model keeps its own depth
		for (int k = 0; k < 3; ++k) {
			if (k == 2 && want.z() <= 0.01f) continue;
			err += qAbs(have[perms[p][k]] - want[k]);
		}
		if (err < best_err - 0.01f) {best_err = err; best = p;}
	}
	const auto &pm = perms[best];
		// an odd permutation mirrors the model: flip x back
	const bool odd = (best == 1 || best == 2 || best == 5);
	auto map = [&](const QVector3D &v) {
		QVector3D r(v[pm[0]], v[pm[1]], v[pm[2]]);
		if (odd) r.setX(-r.x());
		return r;
	};
	PanelMesh out;
	out.vertices.reserve(in.vertices.size());
	out.normals.reserve(in.normals.size());
	QVector3D mlo(1e30f, 1e30f, 1e30f), mhi(-1e30f, -1e30f, -1e30f);
	for (const QVector3D &v : in.vertices) {
		const QVector3D r = map(v);
		out.vertices << r;
		for (int k = 0; k < 3; ++k) {mlo[k] = qMin(mlo[k], r[k]); mhi[k] = qMax(mhi[k], r[k]);}
	}
	for (const QVector3D &n : in.normals) out.normals << map(n);
	const QVector3D shift(
		(lo.x() + hi.x()) / 2 - (mlo.x() + mhi.x()) / 2,
		(lo.y() + hi.y()) / 2 - (mlo.y() + mhi.y()) / 2,
		lo.z() - mlo.z());
	for (QVector3D &v : out.vertices) v += shift;
	out.min = mlo + shift;
	out.max = mhi + shift;
		// a mirrored model has its triangles the other way round
	if (odd)
		for (int i = 0; i + 2 < out.vertices.size(); i += 3) {
			std::swap(out.vertices[i + 1], out.vertices[i + 2]);
			std::swap(out.normals[i + 1], out.normals[i + 2]);
		}
	return out;
}

} // namespace

/* ------------------------------------------------------------------ */

Panel3DWidget::Panel3DWidget(QWidget *parent) :
	Panel3DBase(parent)
{
	setMinimumSize(480, 360);
	setFocusPolicy(Qt::StrongFocus);
}

void Panel3DWidget::setBodies(const QList<Panel3DBody> &bodies)
{
	m_bodies = bodies;
	m_selected = -1;
	fit();
	update();
}

void Panel3DWidget::fit()
{
	QVector3D lo(1e30f, 1e30f, 1e30f), hi(-1e30f, -1e30f, -1e30f);
	for (const Panel3DBody &b : m_bodies)
		for (int k = 0; k < 3; ++k) {lo[k] = qMin(lo[k], b.min[k]); hi[k] = qMax(hi[k], b.max[k]);}
	if (m_bodies.isEmpty()) {lo = QVector3D(0, 0, 0); hi = QVector3D(600, 400, 100);}
	m_centre = (lo + hi) / 2;
	m_pan = QVector3D();
	m_distance = qMax(200.0f, (hi - lo).length() * 1.6f);
}

void Panel3DWidget::viewFront() {m_yaw = 0; m_pitch = 0; fit(); update();}
void Panel3DWidget::viewTop()   {m_yaw = 0; m_pitch = 89; fit(); update();}
void Panel3DWidget::viewIso()   {m_yaw = -25; m_pitch = 20; fit(); update();}

void Panel3DWidget::mousePressEvent(QMouseEvent *event)
{
	m_last = m_press = event->position().toPoint();
}

void Panel3DWidget::mouseMoveEvent(QMouseEvent *event)
{
	const QPoint p = event->position().toPoint();
	const QPoint d = p - m_last;
	m_last = p;
	if (event->buttons() & Qt::LeftButton) {
		m_yaw += d.x() * 0.4f;
		m_pitch = qBound(-89.0f, m_pitch + d.y() * 0.4f, 89.0f);
		update();
	} else if (event->buttons() & (Qt::RightButton | Qt::MiddleButton)) {
			// move in the view plane: undo the view rotation
		QMatrix4x4 r;
		r.rotate(-m_yaw, 0, 1, 0);
		r.rotate(-m_pitch, 1, 0, 0);
		const float s = m_distance / qMax(1, height()) * 0.7f;
		m_pan += r.map(QVector3D(-d.x() * s, d.y() * s, 0));
		update();
	}
}

void Panel3DWidget::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() != Qt::LeftButton) return;
	if ((event->position().toPoint() - m_press).manhattanLength() > 3) return;
	m_selected = pick(event->position().toPoint());
	update();
	emit picked(m_selected >= 0 ? m_bodies.at(m_selected).item.data() : nullptr);
}

void Panel3DWidget::wheelEvent(QWheelEvent *event)
{
	const float steps = event->angleDelta().y() / 120.0f;
	m_distance = qBound(20.0f, m_distance * float(qPow(0.85, steps)), 50000.0f);
	update();
}

int Panel3DWidget::pick(const QPoint &pos) const
{
	const QMatrix4x4 mvp = projection(width(), height(), m_distance)
						   * modelView(m_centre, m_pan, m_distance, m_yaw, m_pitch);
	const QMatrix4x4 inv = mvp.inverted();
	const float x = 2.0f * pos.x() / qMax(1, width()) - 1.0f;
	const float y = 1.0f - 2.0f * pos.y() / qMax(1, height());
	const QVector3D a = inv.map(QVector3D(x, y, -1));
	const QVector3D b = inv.map(QVector3D(x, y, 1));
	const QVector3D dir = (b - a).normalized();
	int best = -1;
	float best_t = 1e30f;
	for (int i = 0; i < m_bodies.size(); ++i) {
		if (!m_bodies.at(i).item) continue;
		const float t = hitBox(a, dir, m_bodies.at(i).min, m_bodies.at(i).max);
			// the surface lies under everything: only pick it when nothing else is hit
		const float bias = m_bodies.at(i).holes.isEmpty() ? 0 : 1e6f;
		if (t >= 0 && t + bias < best_t) {best_t = t + bias; best = i;}
	}
	return best;
}

#ifdef QET_WITH_OPENGL

void Panel3DWidget::initializeGL()
{
}

void Panel3DWidget::resizeGL(int, int)
{
}

void Panel3DWidget::paintGL()
{
	auto *gl = QOpenGLVersionFunctionsFactory::get<QOpenGLFunctions_1_1>(context());
	if (!gl) {
		QPainter p(this);
		p.fillRect(rect(), palette().window());
		p.drawText(rect(), Qt::AlignCenter, tr("OpenGL is not available on this computer."));
		return;
	}
	const qreal dpr = devicePixelRatioF();
	gl->glViewport(0, 0, int(width() * dpr), int(height() * dpr));
	gl->glClearColor(0.93f, 0.94f, 0.96f, 1.0f);
	gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl->glEnable(GL_DEPTH_TEST);
	gl->glDepthFunc(GL_LEQUAL);
	gl->glEnable(GL_NORMALIZE);
	gl->glShadeModel(GL_FLAT);

	gl->glMatrixMode(GL_PROJECTION);
	gl->glLoadMatrixf(projection(width(), height(), m_distance).constData());
	gl->glMatrixMode(GL_MODELVIEW);
	gl->glLoadIdentity();

		// light from the viewer, slightly above left, fixed to the camera
	const GLfloat light_pos[4] = {-0.3f, 0.5f, 1.0f, 0.0f};
	const GLfloat ambient[4] = {0.35f, 0.35f, 0.35f, 1.0f};
	const GLfloat diffuse[4] = {0.75f, 0.75f, 0.75f, 1.0f};
	gl->glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
	gl->glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	gl->glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	gl->glEnable(GL_LIGHT0);
	gl->glEnable(GL_COLOR_MATERIAL);
	gl->glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	gl->glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

	gl->glLoadMatrixf(modelView(m_centre, m_pan, m_distance, m_yaw, m_pitch).constData());

		// opaque bodies first, the see-through ones (enclosure) after them
	QList<int> order;
	for (int i = 0; i < m_bodies.size(); ++i) if (m_bodies.at(i).opacity >= 1.0f) order << i;
	for (int i = 0; i < m_bodies.size(); ++i) if (m_bodies.at(i).opacity < 1.0f) order << i;
	for (int i : order)
	{
		const Panel3DBody &b = m_bodies.at(i);
		QColor c = b.colour;
			// the selection lights up everything of the same item: a whole
			// terminal strip, a device with its BaseUnit
		if (i == m_selected || (m_selected >= 0 && b.item && b.item == m_bodies.at(m_selected).item))
			c = QColor(230, 120, 30);
		gl->glEnable(GL_LIGHTING);
		if (b.opacity < 1.0f) {
			gl->glEnable(GL_BLEND);
			gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			gl->glDepthMask(GL_FALSE);
		} else {
			gl->glDisable(GL_BLEND);
			gl->glDepthMask(GL_TRUE);
		}
		gl->glColor4f(float(c.redF()), float(c.greenF()), float(c.blueF()), b.opacity);
		gl->glEnable(GL_POLYGON_OFFSET_FILL);
		gl->glPolygonOffset(1.0f, 1.0f);
		if (!b.mesh.isEmpty()) {
			gl->glBegin(GL_TRIANGLES);
			for (int k = 0; k < b.mesh.vertices.size(); ++k) {
				const QVector3D n = b.mesh.normals.value(k);
				const QVector3D v = b.mesh.vertices.at(k) + b.mesh_offset;
				gl->glNormal3f(n.x(), n.y(), n.z());
				gl->glVertex3f(v.x(), v.y(), v.z());
			}
			gl->glEnd();
		} else {
			const QVector3D lo = b.min, hi = b.max;
			gl->glBegin(GL_QUADS);
			gl->glNormal3f(0, 0, 1);
			gl->glVertex3f(lo.x(), lo.y(), hi.z()); gl->glVertex3f(hi.x(), lo.y(), hi.z());
			gl->glVertex3f(hi.x(), hi.y(), hi.z()); gl->glVertex3f(lo.x(), hi.y(), hi.z());
			gl->glNormal3f(0, 0, -1);
			gl->glVertex3f(lo.x(), lo.y(), lo.z()); gl->glVertex3f(lo.x(), hi.y(), lo.z());
			gl->glVertex3f(hi.x(), hi.y(), lo.z()); gl->glVertex3f(hi.x(), lo.y(), lo.z());
			gl->glNormal3f(1, 0, 0);
			gl->glVertex3f(hi.x(), lo.y(), lo.z()); gl->glVertex3f(hi.x(), hi.y(), lo.z());
			gl->glVertex3f(hi.x(), hi.y(), hi.z()); gl->glVertex3f(hi.x(), lo.y(), hi.z());
			gl->glNormal3f(-1, 0, 0);
			gl->glVertex3f(lo.x(), lo.y(), lo.z()); gl->glVertex3f(lo.x(), lo.y(), hi.z());
			gl->glVertex3f(lo.x(), hi.y(), hi.z()); gl->glVertex3f(lo.x(), hi.y(), lo.z());
			gl->glNormal3f(0, 1, 0);
			gl->glVertex3f(lo.x(), hi.y(), lo.z()); gl->glVertex3f(lo.x(), hi.y(), hi.z());
			gl->glVertex3f(hi.x(), hi.y(), hi.z()); gl->glVertex3f(hi.x(), hi.y(), lo.z());
			gl->glNormal3f(0, -1, 0);
			gl->glVertex3f(lo.x(), lo.y(), lo.z()); gl->glVertex3f(hi.x(), lo.y(), lo.z());
			gl->glVertex3f(hi.x(), lo.y(), hi.z()); gl->glVertex3f(lo.x(), lo.y(), hi.z());
			gl->glEnd();
		}
		gl->glDisable(GL_POLYGON_OFFSET_FILL);
		gl->glDisable(GL_LIGHTING);

			// outline of the box, like the edges in a 3D layout
		if (b.mesh.isEmpty()) {
			gl->glColor3f(0.15f, 0.15f, 0.15f);
			const QVector3D lo = b.min, hi = b.max;
			const QVector3D p[8] = {
				{lo.x(), lo.y(), lo.z()}, {hi.x(), lo.y(), lo.z()}, {hi.x(), hi.y(), lo.z()}, {lo.x(), hi.y(), lo.z()},
				{lo.x(), lo.y(), hi.z()}, {hi.x(), lo.y(), hi.z()}, {hi.x(), hi.y(), hi.z()}, {lo.x(), hi.y(), hi.z()}};
			static const int e[24] = {0,1, 1,2, 2,3, 3,0, 4,5, 5,6, 6,7, 7,4, 0,4, 1,5, 2,6, 3,7};
			gl->glBegin(GL_LINES);
			for (int k = 0; k < 24; ++k) gl->glVertex3f(p[e[k]].x(), p[e[k]].y(), p[e[k]].z());
			gl->glEnd();
		}

			// holes: dark discs just above the top face of the plate
		if (!b.holes.isEmpty()) {
			gl->glColor3f(0.12f, 0.12f, 0.14f);
			const float z = b.max.z() + 0.05f;
			for (int h = 0; h < b.holes.size(); ++h) {
				const QVector3D c0 = b.holes.at(h);
				const float r = qMax(0.5f, b.hole_radius.value(h, 2.5f));
				gl->glBegin(GL_TRIANGLE_FAN);
				gl->glVertex3f(c0.x(), c0.y(), z);
				for (int s = 0; s <= 16; ++s) {
					const float a = float(s) * float(M_PI) / 8.0f;
					gl->glVertex3f(c0.x() + r * qCos(a), c0.y() + r * qSin(a), z);
				}
				gl->glEnd();
			}
		}
	}
}

#endif // QET_WITH_OPENGL

/* ------------------------------------------------------------------ */

/**
	@brief Panel3DDialog::build
	Make the bodies of the 3D scene from the cabinet layout of a folio:
	every mounting surface with its holes, the rails, ducts and profiles
	on it, the devices as boxes from their width/height/depth (or the
	STEP model named in step_file). Surfaces of one folio are put side by
	side. @p notes collects what could not be shown.
*/
QList<Panel3DBody> Panel3DDialog::build(Diagram *diagram, QStringList *notes, bool enclosure)
{
	QList<Panel3DBody> bodies;
	if (!diagram) return bodies;
	float offset_x = 0;
	QString base_dir;
	if (diagram->project() && !diagram->project()->filePath().isEmpty())
		base_dir = QFileInfo(diagram->project()->filePath()).absolutePath();
	auto resolve = [&](const QString &file) {
		if (file.isEmpty() || QFileInfo(file).isAbsolute() || base_dir.isEmpty()) return file;
		return QDir(base_dir).filePath(file);
	};

		// surfaces outside the cabinet (door, side walls) go onto the
		// enclosure of the mounting plate once that is placed
	QList<QetShapeItem *> outer;
	bool have_enclosure = false;
	QVector3D enc_lo, enc_hi;
	for (QetShapeItem *surface : PanelLayout::surfaces(diagram))
	{
		const PanelLayout layout(surface);
		const PanelData sd = layout.surfaceData();
		if (!sd.face.isEmpty()) {outer << surface; continue;}
		if (notes)
			for (const QString &w : layout.warnings())
				if (w.contains(QLatin1String("überschneidet")) || w.contains(QLatin1String("liegt im "))
					|| w.contains(QLatin1String("zu kurz")) || w.contains(QLatin1String("ragt über")))
					*notes << w;
		const float w = float(layout.widthMm()), h = float(layout.heightMm());
		const float t = float(qMax(0.5, sd.thickness));

		Panel3DBody plate;
		plate.min = QVector3D(offset_x, 0, -t);
		plate.max = QVector3D(offset_x + w, h, 0);
		plate.colour = QColor(200, 200, 196);   // galvanised sheet steel
		plate.item = surface;
		plate.name = layout.name();
		for (const PanelHole &hole : layout.holes()) {
			plate.holes << QVector3D(offset_x + float(hole.x), float(hole.y), 0);
			const double r = (hole.type == 1 || hole.type == 2) ? hole.d1 / 2.0
																: qMax(hole.d1, hole.d2) / 2.0;
			plate.hole_radius << float(r);
		}
		if (!sd.step_file.isEmpty()) {
			QString error;
			const PanelMesh mesh = PanelStep::load(resolve(sd.step_file), &error);
			if (!mesh.isEmpty()) plate.mesh = fitMesh(mesh, plate.min, plate.max);
			else if (notes) *notes << error;
		}
		bodies << plate;

		float rail_top = 7.5f;
		for (const PanelProfile &p : layout.profiles())
		{
			Panel3DBody b;
			const float half = float(p.width_mm) / 2;
			const QPointF s = p.start_mm, e = p.end_mm;
			const bool horizontal = qAbs(e.x() - s.x()) >= qAbs(e.y() - s.y());
			const float x0 = float(qMin(s.x(), e.x())), x1 = float(qMax(s.x(), e.x()));
			const float y0 = float(qMin(s.y(), e.y())), y1 = float(qMax(s.y(), e.y()));
			float depth = float(p.data.profile_depth);
			if (depth <= 0) depth = p.data.role == PanelData::DinRail ? 7.5f
								  : p.data.role == PanelData::Duct ? 60.0f : 15.0f;
			if (p.data.role == PanelData::DinRail) rail_top = depth;
			if (horizontal) {
				b.min = QVector3D(offset_x + x0, y0 - half, 0);
				b.max = QVector3D(offset_x + x1, y0 + half, depth);
			} else {
				b.min = QVector3D(offset_x + x0 - half, y0, 0);
				b.max = QVector3D(offset_x + x0 + half, y1, depth);
			}
			b.colour = p.data.role == PanelData::DinRail ? QColor(170, 175, 182)
					 : p.data.role == PanelData::Duct ? QColor(150, 152, 156)
					 : QColor(160, 160, 160);
			b.item = p.shape;
			b.name = p.data.designation;
			if (!p.data.step_file.isEmpty()) {
				QString error;
				const PanelMesh mesh = PanelStep::load(resolve(p.data.step_file), &error);
				if (!mesh.isEmpty()) b.mesh = fitMesh(mesh, b.min, b.max);
				else if (notes) *notes << error;
			}
				// a cable duct without a model: floor, slotted side walls
				// (fingers) and the lid -- drawn like a slotted duct, the
				// slot pitch is only for the picture
			if (p.data.role == PanelData::Duct && b.mesh.isEmpty()) {
				PanelMesh m;
				const float wall = 1.8f, lid = 2.5f, finger = 8.0f, slot = 4.0f;
				const bool along_x = horizontal;
				const float len0 = along_x ? b.min.x() : b.min.y(), len1 = along_x ? b.max.x() : b.max.y();
				const float w0 = along_x ? b.min.y() : b.min.x(), w1 = along_x ? b.max.y() : b.max.x();
				const float z0 = b.min.z(), z1 = b.max.z();
				auto box = [&](float l0, float l1, float a0, float a1, float h0, float h1) {
					const QVector3D lo = along_x ? QVector3D(l0, a0, h0) : QVector3D(a0, l0, h0);
					const QVector3D hi = along_x ? QVector3D(l1, a1, h1) : QVector3D(a1, l1, h1);
					appendBox(m, lo, hi);
				};
				box(len0, len1, w0, w1, z0, z0 + wall);                       // floor
				box(len0, len1, w0, w1, z1 - lid, z1);                        // lid
				for (float l = len0; l < len1; l += finger + slot) {          // fingers
					const float e = qMin(l + finger, len1);
					box(l, e, w0, w0 + wall, z0 + wall, z1 - lid);
					box(l, e, w1 - wall, w1, z0 + wall, z1 - lid);
				}
				b.mesh = m;
				b.colour = QColor(172, 176, 180);   // light grey PVC
			}
			bodies << b;
		}

		for (const PanelDevice &d : layout.devices())
		{
			Panel3DBody b;
			const QRectF r = d.rect_mm.normalized();
			const float z0 = d.on_rail ? rail_top : 0;
			float depth = float(d.depth_mm);
			const bool no_depth = depth <= 0;
			if (no_depth) depth = 60;
			b.min = QVector3D(offset_x + float(r.left()), float(r.top()), z0);
			b.max = QVector3D(offset_x + float(r.right()), float(r.bottom()), z0 + depth);
			b.colour = QColor(90, 110, 140);
			b.item = d.element;
			b.name = d.designation;
			if (!d.step_file.isEmpty()) {
				QString error;
				const PanelMesh mesh = PanelStep::load(resolve(d.step_file), &error);
				if (!mesh.isEmpty()) {
						// the footprint on the plate decides how the model stands;
						// its depth is the model's own (a depth in the layout may
						// be a placeholder and would lay a tall device flat)
					b.mesh = fitMesh(mesh, b.min, QVector3D(b.max.x(), b.max.y(), b.min.z()));
					b.max.setZ(qMax(b.max.z(), b.mesh.max.z()));
					b.colour = QColor(120, 125, 135);
				} else if (notes) *notes << error;
			}
				// accessories mounted with it: a part as wide and high as the
				// device (an ET 200SP BaseUnit) carries it -- the carrier sits
				// on the rail, the device in front of it, flush at the top and
				// at the depth given for the whole
			for (const QString &acc : d.accessories) {
				const QString file = PanelStep::findModel(acc);
				if (file.isEmpty()) continue;
				const PanelMesh am = PanelStep::load(file);
				if (am.isEmpty()) continue;
				const PanelMesh fitted = fitMesh(am, b.min, QVector3D(b.max.x(), b.max.y(), b.min.z()));
				const QVector3D fs = fitted.max - fitted.min, bs = b.max - b.min;
					// a carrier covers the device's footprint (it may be a bit
					// wider and higher: latches, terminal area)
				if (fs.x() < 0.8f * bs.x() || fs.x() > 1.5f * bs.x()
					|| fs.y() < 0.8f * bs.y() || fs.y() > 1.4f * bs.y()) continue;
				Panel3DBody carrier;
				carrier.mesh = fitted;
				carrier.min = fitted.min;
				carrier.max = fitted.max;
				carrier.colour = QColor(150, 152, 158);
				carrier.item = d.element;
				carrier.name = d.designation + QStringLiteral(" ") + acc;
				if (!b.mesh.isEmpty()) {
					const float front = no_depth ? fitted.max.z() + (b.mesh.max.z() - b.mesh.min.z()) * 0.5f
												 : b.min.z() + depth;
					const QVector3D shift(0, fitted.max.y() - b.mesh.max.y(), front - b.mesh.max.z());
					for (QVector3D &v : b.mesh.vertices) v += shift;
					b.mesh.min += shift;
					b.mesh.max += shift;
				}
				bodies << carrier;
				break;
			}
				// a terminal strip: every terminal with its own model, side by
				// side along the rail from the left end of the strip
			if (!d.terminals.isEmpty()) {
				float cursor = b.min.x();
				int shown = 0;
				for (const QString &t : d.terminals) {
					const QString file = PanelStep::findModel(t);
					PanelMesh tm;
					if (!file.isEmpty()) tm = PanelStep::load(file);
					if (tm.isEmpty()) {cursor += 6.1f; continue;}
						// the thinnest side along the rail, the longest across
					PanelMesh fitted = fitMesh(tm, QVector3D(cursor, b.min.y(), b.min.z()),
											   QVector3D(cursor + 0.1f, b.max.y(), b.min.z()));
					const float shift = cursor - fitted.min.x();
					for (QVector3D &v : fitted.vertices) v.setX(v.x() + shift);
					fitted.min.setX(fitted.min.x() + shift);
					fitted.max.setX(fitted.max.x() + shift);
					Panel3DBody tb;
					tb.mesh = fitted;
					tb.min = fitted.min;
					tb.max = fitted.max;
					tb.colour = QColor(196, 180, 140);   // terminal beige
					tb.item = d.element;
					tb.name = d.designation + QStringLiteral(":") + QString::number(++shown) + QStringLiteral(" ") + t;
					bodies << tb;
					cursor = fitted.max.x();
				}
				if (shown) continue;
			}
			if (no_depth && b.mesh.isEmpty() && notes)
				*notes << tr("%1: no depth given, shown 60 mm deep").arg(d.designation);
			bodies << b;
		}
			//the enclosure around the plate: a frame of 40 mm profiles like a
			//TS 8 / VX 25 enclosure, 40 mm beyond the plate on every side and
			//deep enough for the deepest part
		if (enclosure) {
			float front = 0;
			for (const Panel3DBody &b : bodies) front = qMax(front, b.max.z());
			front += 60;
			const float m = 40, p = 40;   // margin, profile size
			QVector3D lo(offset_x - m - p, -m - p, -t - 30 - p), hi(offset_x + w + m + p, h + m + p, front + p);
				// an enclosure drawn around the plate gives the real size
			const QRectF plate_rect = surface->mapToScene(surface->rect().normalized()).boundingRect();
			QString enclosure_model, enclosure_name;
			QetShapeItem *enclosure_item = nullptr;
			for (QGraphicsItem *item : diagram->items()) {
				auto *shape = dynamic_cast<QetShapeItem *>(item);
				if (!shape || shape->panelData().role != PanelData::Enclosure) continue;
				const QRectF er = shape->mapToScene(shape->rect().normalized()).boundingRect();
				if (!er.contains(plate_rect.center())) continue;
				const double k = qMax(1e-6, sd.mm_per_unit);
				const PanelData ed = shape->panelData();
				lo = QVector3D(offset_x + float((er.left() - plate_rect.left()) * k),
							   float((plate_rect.bottom() - er.bottom()) * k), -t - 25);
				hi = QVector3D(offset_x + float((er.right() - plate_rect.left()) * k),
							   float((plate_rect.bottom() - er.top()) * k),
							   -t - 25 + float(ed.profile_depth > 0 ? ed.profile_depth : front + t + 25));
					enclosure_model = ed.step_file.isEmpty() ? PanelStep::findModel(ed.article, ed.manufacturer)
														 : resolve(ed.step_file);
				enclosure_item = shape;
				enclosure_name = ed.designation.isEmpty() ? ed.article : ed.designation;
				if (!enclosure_model.isEmpty()) break;
					// the rear wall behind the plate
				Panel3DBody wall;
				wall.min = QVector3D(lo.x() + p, lo.y() + p, lo.z());
				wall.max = QVector3D(hi.x() - p, hi.y() - p, lo.z() + 2);
				wall.colour = QColor(205, 207, 210);
				wall.name = QStringLiteral("frame");
				bodies << wall;
				break;
			}
			have_enclosure = true;
			enc_lo = lo;
			enc_hi = hi;
				// the manufacturer's model of the enclosure (by its order
				// number), see-through so the inside stays visible
			if (!enclosure_model.isEmpty()) {
				QString error;
				const PanelMesh mesh = PanelStep::load(enclosure_model, &error);
				if (!mesh.isEmpty()) {
					Panel3DBody cab;
					cab.mesh = fitMesh(mesh, lo, hi);
					cab.min = cab.mesh.min;
					cab.max = cab.mesh.max;
					cab.colour = QColor(200, 203, 208);
					cab.opacity = 0.28f;
					cab.item = enclosure_item;
					cab.name = enclosure_name;
					bodies << cab;
					enc_lo = cab.min;
					enc_hi = cab.max;
					offset_x += w + 150 + 160;
					continue;
				}
				if (notes) *notes << error;
			}
			auto post = [&](QVector3D a, QVector3D b) {
				Panel3DBody frame;
				frame.min = a;
				frame.max = b;
				frame.colour = QColor(75, 78, 84);
				frame.name = QStringLiteral("frame");
				bodies << frame;
			};
				// four upright posts
			for (float x : {lo.x(), hi.x() - p})
				for (float z : {lo.z(), hi.z() - p})
					post(QVector3D(x, lo.y(), z), QVector3D(x + p, hi.y(), z + p));
				// top and bottom: along x and along z
			for (float y : {lo.y(), hi.y() - p}) {
				for (float z : {lo.z(), hi.z() - p})
					post(QVector3D(lo.x() + p, y, z), QVector3D(hi.x() - p, y + p, z + p));
				for (float x : {lo.x(), hi.x() - p})
					post(QVector3D(x, y, lo.z() + p), QVector3D(x + p, y + p, hi.z() - p));
			}
		}
		offset_x += w + 150 + (enclosure ? 160 : 0);
	}

		// Door and side walls: the surface is stretched over that face of
		// the enclosure (its drawing only has to have the right
		// proportions); each device stands on it with its front outwards
		// and its body inside the cabinet
	for (QetShapeItem *surface : std::as_const(outer))
	{
		const PanelLayout layout(surface);
		const PanelData sd = layout.surfaceData();
		if (!have_enclosure) {
			if (notes) *notes << tr("%1: a door or wall surface needs the enclosure on the same folio").arg(layout.name());
			continue;
		}
		const bool door = sd.face == QLatin1String("door");
		const bool left = sd.face == QLatin1String("left");
		const QVector3D size = enc_hi - enc_lo;
		const float su = float((door ? size.x() : size.z()) / qMax(1.0, layout.widthMm()));
		const float sv = float(size.y() / qMax(1.0, layout.heightMm()));
		const float skin = 2.0f;   // sheet of the door or wall
			// a point on the surface (u right, v up, w outwards) in the scene
		auto place = [&](float u, float v, float out) {
			const float y = enc_lo.y() + v;
			if (door) return QVector3D(enc_lo.x() + u, y, enc_hi.z() + out);
			if (left) return QVector3D(enc_lo.x() - out, y, enc_lo.z() + u);
			return QVector3D(enc_hi.x() + out, y, enc_hi.z() - u);
		};
		auto turn = [&](const QVector3D &n) {
			if (door) return n;
			if (left) return QVector3D(-n.z(), n.y(), n.x());
			return QVector3D(n.z(), n.y(), -n.x());
		};
		for (const PanelDevice &d : layout.devices())
		{
			const QRectF r = d.rect_mm.normalized();
			const float u0 = float(r.left()) * su, v0 = float(r.top()) * sv;
			Panel3DBody b;
			b.item = d.element;
			b.name = d.designation;
			b.colour = QColor(90, 110, 140);
			PanelMesh mesh;
			if (!d.step_file.isEmpty()) {
				QString error;
				const PanelMesh m = PanelStep::load(d.step_file, &error);
				if (!m.isEmpty()) mesh = fitMesh(m, QVector3D(0, 0, 0), QVector3D(float(r.width()), float(r.height()), 0));
				else if (notes) *notes << error;
			}
			if (mesh.isEmpty()) {
					// no model: a box reaching into the cabinet
				const float depth = d.depth_mm > 0 ? float(d.depth_mm) : 40.0f;
				appendBox(mesh, QVector3D(0, 0, 0), QVector3D(float(r.width()), float(r.height()), depth));
			} else {
				b.colour = QColor(120, 125, 135);
			}
				// the front (highest z of the model) lies on the outside of
				// the sheet, the rest goes inwards
			const float front = mesh.max.z();
			PanelMesh placed;
			QVector3D lo(1e30f, 1e30f, 1e30f), hi(-1e30f, -1e30f, -1e30f);
			for (int i = 0; i < mesh.vertices.size(); ++i) {
				const QVector3D v = mesh.vertices.at(i);
				const QVector3D p = place(u0 + v.x() - mesh.min.x(), v0 + v.y() - mesh.min.y(), v.z() - front + skin);
				placed.vertices << p;
				placed.normals << turn(mesh.normals.value(i));
				for (int k = 0; k < 3; ++k) {lo[k] = qMin(lo[k], p[k]); hi[k] = qMax(hi[k], p[k]);}
			}
			placed.min = lo;
			placed.max = hi;
			b.mesh = placed;
			b.min = lo;
			b.max = hi;
			bodies << b;
		}
	}
	return bodies;
}

/**
	@brief Panel3DDialog::writeGlb
	Write the scene as one binary glTF 2.0 file (.glb), in metres with y up:
	every body one mesh with its colour, a box where it has no STEP model.
	Opens in any 3D viewer, a browser or a CAD program.
*/
bool Panel3DDialog::writeGlb(const QList<Panel3DBody> &bodies, const QString &path, QString *error)
{
	QByteArray bin;
	QJsonArray buffer_views, accessors, meshes, nodes, materials, scene_nodes;
	auto append = [&](const QVector<QVector3D> &data, bool positions) {
		const int offset = bin.size();
		QVector3D lo(1e30f, 1e30f, 1e30f), hi(-1e30f, -1e30f, -1e30f);
		for (const QVector3D &v : data) {
			for (int k = 0; k < 3; ++k) {
				const float f = v[k];
				bin.append(reinterpret_cast<const char *>(&f), 4);
				lo[k] = qMin(lo[k], f); hi[k] = qMax(hi[k], f);
			}
		}
		buffer_views.append(QJsonObject{{"buffer", 0}, {"byteOffset", offset},
										{"byteLength", int(data.size() * 12)}, {"target", 34962}});
		QJsonObject a{{"bufferView", buffer_views.size() - 1}, {"componentType", 5126},
					  {"count", int(data.size())}, {"type", "VEC3"}};
		if (positions) {
			a.insert("min", QJsonArray{lo.x(), lo.y(), lo.z()});
			a.insert("max", QJsonArray{hi.x(), hi.y(), hi.z()});
		}
		accessors.append(a);
		return accessors.size() - 1;
	};
	for (const Panel3DBody &b : bodies)
	{
		QVector<QVector3D> pos, nor;
		if (!b.mesh.isEmpty()) {
			for (int i = 0; i < b.mesh.vertices.size(); ++i) {
				pos << (b.mesh.vertices.at(i) + b.mesh_offset) / 1000.0f;
				nor << b.mesh.normals.value(i).normalized();
			}
		} else {
				// a box: two triangles per side
			const QVector3D l = b.min / 1000.0f, h = b.max / 1000.0f;
			const QVector3D c[8] = {{l.x(), l.y(), l.z()}, {h.x(), l.y(), l.z()}, {h.x(), h.y(), l.z()}, {l.x(), h.y(), l.z()},
									{l.x(), l.y(), h.z()}, {h.x(), l.y(), h.z()}, {h.x(), h.y(), h.z()}, {l.x(), h.y(), h.z()}};
			const int f[6][4] = {{4, 5, 6, 7}, {1, 0, 3, 2}, {5, 1, 2, 6}, {0, 4, 7, 3}, {7, 6, 2, 3}, {0, 1, 5, 4}};
			const QVector3D n[6] = {{0, 0, 1}, {0, 0, -1}, {1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}};
			for (int s = 0; s < 6; ++s)
				for (int t : {0, 1, 2, 0, 2, 3}) {pos << c[f[s][t]]; nor << n[s];}
		}
		if (pos.isEmpty()) continue;
		const int pa = append(pos, true), na = append(nor, false);
		const QColor col = b.colour.isValid() ? b.colour : QColor(128, 128, 128);
		materials.append(QJsonObject{{"pbrMetallicRoughness", QJsonObject{
			{"baseColorFactor", QJsonArray{col.redF(), col.greenF(), col.blueF(), double(b.opacity)}},
			{"metallicFactor", 0.1}, {"roughnessFactor", 0.7}}}, {"doubleSided", true},
			{"alphaMode", b.opacity < 1.0f ? "BLEND" : "OPAQUE"}});
		meshes.append(QJsonObject{{"primitives", QJsonArray{QJsonObject{
			{"attributes", QJsonObject{{"POSITION", pa}, {"NORMAL", na}}},
			{"material", materials.size() - 1}}}}});
		nodes.append(QJsonObject{{"mesh", meshes.size() - 1},
								 {"name", b.name.isEmpty() ? QStringLiteral("body%1").arg(nodes.size()) : b.name}});
		scene_nodes.append(nodes.size() - 1);
	}
	while (bin.size() % 4) bin.append(char(0));
	const QJsonObject gltf{
		{"asset", QJsonObject{{"version", "2.0"}, {"generator", "QElectroTech cabinet layout"}}},
		{"scene", 0}, {"scenes", QJsonArray{QJsonObject{{"nodes", scene_nodes}}}},
		{"nodes", nodes}, {"meshes", meshes}, {"materials", materials},
		{"accessors", accessors}, {"bufferViews", buffer_views},
		{"buffers", QJsonArray{QJsonObject{{"byteLength", int(bin.size())}}}}};
	QByteArray json = QJsonDocument(gltf).toJson(QJsonDocument::Compact);
	while (json.size() % 4) json.append(' ');
	QFile file(path);
	if (!file.open(QIODevice::WriteOnly)) {
		if (error) *error = file.errorString();
		return false;
	}
	auto u32 = [&](quint32 v) {file.write(reinterpret_cast<const char *>(&v), 4);};
	u32(0x46546C67); u32(2); u32(quint32(12 + 8 + json.size() + 8 + bin.size()));
	u32(quint32(json.size())); u32(0x4E4F534A); file.write(json);
	u32(quint32(bin.size())); u32(0x004E4942); file.write(bin);
	return true;
}

Panel3DDialog::Panel3DDialog(Diagram *diagram, QWidget *parent) :
	QDialog(parent),
	m_diagram(diagram)
{
	setWindowTitle(tr("3D view of the cabinet layout"));
	setAttribute(Qt::WA_DeleteOnClose);
	resize(1000, 720);

	m_view = new Panel3DWidget(this);
	m_status = new QLabel(this);
	m_status->setWordWrap(true);

	auto *front = new QPushButton(tr("Front"), this);
	auto *top = new QPushButton(tr("Top"), this);
	auto *iso = new QPushButton(tr("Isometric"), this);
	auto *refresh = new QPushButton(tr("Refresh"), this);
	auto *save = new QPushButton(tr("Export..."), this);
	save->setToolTip(tr("Save the scene as glTF (.glb) for a 3D viewer, a browser or a CAD program"));
	connect(save, &QPushButton::clicked, this, [this]() {
		const QString path = QFileDialog::getSaveFileName(this, tr("Export 3D view"), QString(),
														  tr("glTF binary (*.glb)"));
		if (path.isEmpty()) return;
		QString error;
		if (!writeGlb(build(m_diagram, nullptr, !m_enclosure || m_enclosure->isChecked()), path, &error))
			QMessageBox::warning(this, tr("Export 3D view"), error);
	});
	m_enclosure = new QCheckBox(tr("Enclosure"), this);
	m_enclosure->setChecked(true);
	connect(m_enclosure, &QCheckBox::toggled, this, &Panel3DDialog::reload);
	connect(front, &QPushButton::clicked, m_view, &Panel3DWidget::viewFront);
	connect(top, &QPushButton::clicked, m_view, &Panel3DWidget::viewTop);
	connect(iso, &QPushButton::clicked, m_view, &Panel3DWidget::viewIso);
	connect(refresh, &QPushButton::clicked, this, &Panel3DDialog::reload);

	auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::close);

	auto *row = new QHBoxLayout;
	row->addWidget(front);
	row->addWidget(top);
	row->addWidget(iso);
	row->addWidget(refresh);
	row->addWidget(save);
	row->addWidget(m_enclosure);
	row->addStretch();
	row->addWidget(buttons);

	auto *layout = new QVBoxLayout(this);
	layout->addWidget(m_view, 1);
	layout->addWidget(m_status);
	layout->addLayout(row);

		// a click on a body selects its item on the folio
	connect(m_view, &Panel3DWidget::picked, this, [this](QObject *object) {
		if (!m_diagram) return;
		auto *item = dynamic_cast<QGraphicsItem *>(object);
		if (!item) return;
		m_diagram->clearSelection();
		item->setSelected(true);
		for (QGraphicsView *view : m_diagram->views()) view->ensureVisible(item);
	});
	reload();
}

void Panel3DDialog::reload()
{
	QStringList notes;
	QGuiApplication::setOverrideCursor(Qt::WaitCursor);
	const QList<Panel3DBody> bodies = build(m_diagram, &notes, !m_enclosure || m_enclosure->isChecked());
	QGuiApplication::restoreOverrideCursor();
	m_view->setBodies(bodies);
	QString text;
	if (bodies.isEmpty())
		text = tr("This folio has no mounting surface. Draw a rectangle and give it the role "
				  "\"Mounting surface\" (Cabinet layout > Manufacturing data).");
	else
		text = tr("%n bodies. Drag: turn, right drag: move, wheel: zoom, click: select on the folio.",
				  nullptr, int(bodies.size()));
	if (!PanelStep::available())
		text += QStringLiteral("  ") + tr("(built without STEP support)");
	notes.removeDuplicates();
	if (!notes.isEmpty()) text += QStringLiteral("\n") + notes.mid(0, 6).join(QStringLiteral("\n"));
	m_status->setText(text);
}
