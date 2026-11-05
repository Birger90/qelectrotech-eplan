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

#include <QDialogButtonBox>
#include <QDir>
#include <QFileInfo>
#include <QGraphicsView>
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
		for (int k = 0; k < 3; ++k) err += qAbs(have[perms[p][k]] - want[k]);
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

	for (int i = 0; i < m_bodies.size(); ++i)
	{
		const Panel3DBody &b = m_bodies.at(i);
		QColor c = b.colour;
		if (i == m_selected) c = QColor(230, 120, 30);
		gl->glEnable(GL_LIGHTING);
		gl->glColor3f(float(c.redF()), float(c.greenF()), float(c.blueF()));
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
QList<Panel3DBody> Panel3DDialog::build(Diagram *diagram, QStringList *notes)
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

	for (QetShapeItem *surface : PanelLayout::surfaces(diagram))
	{
		const PanelLayout layout(surface);
		const PanelData sd = layout.surfaceData();
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
			bodies << b;
		}

		for (const PanelDevice &d : layout.devices())
		{
			Panel3DBody b;
			const QRectF r = d.rect_mm.normalized();
			const float z0 = d.on_rail ? rail_top : 0;
			float depth = float(d.depth_mm);
			if (depth <= 0) {
				depth = 60;
				if (notes) *notes << tr("%1: no depth given, shown 60 mm deep").arg(d.designation);
			}
			b.min = QVector3D(offset_x + float(r.left()), float(r.top()), z0);
			b.max = QVector3D(offset_x + float(r.right()), float(r.bottom()), z0 + depth);
			b.colour = QColor(90, 110, 140);
			b.item = d.element;
			b.name = d.designation;
			if (!d.step_file.isEmpty()) {
				QString error;
				const PanelMesh mesh = PanelStep::load(resolve(d.step_file), &error);
				if (!mesh.isEmpty()) {
					b.mesh = fitMesh(mesh, b.min, b.max);
					b.colour = QColor(120, 125, 135);
				} else if (notes) *notes << error;
			}
			bodies << b;
		}
		offset_x += w + 150;
	}
	return bodies;
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
	const QList<Panel3DBody> bodies = build(m_diagram, &notes);
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
