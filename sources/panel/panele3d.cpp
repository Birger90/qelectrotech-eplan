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
#include "panele3d.h"

#include <QFile>
#include <QMatrix4x4>
#include <QObject>
#include <QtEndian>
#include <cstring>
#include <stdexcept>

namespace {

	/// reads the primitives of the format; throws at the end of the data
class Reader
{
	public:
		explicit Reader(const QByteArray &data) : m_data(data) {}
		bool atEnd() const {return m_pos == m_data.size();}
		int pos() const {return m_pos;}
		const char *take(int n) {
			if (n < 0 || m_pos + n > m_data.size()) throw std::runtime_error("unexpected end of the file");
			const char *p = m_data.constData() + m_pos;
			m_pos += n;
			return p;
		}
		quint8 byte() {return quint8(*take(1));}
		qint16 shortValue() {return qFromLittleEndian<qint16>(take(2));}
		qint32 longValue() {return qFromLittleEndian<qint32>(take(4));}
		float floatValue() {
			const quint32 bits = qFromLittleEndian<quint32>(take(4));
			float f;
			std::memcpy(&f, &bits, 4);
			return f;
		}
		void skip(int n) {take(n);}
		void color() {skip(4);}
		QVector3D vector() {const float x = floatValue(), y = floatValue(); return QVector3D(x, y, floatValue());}
			/// 4 vectors: the columns of the rotation and the translation
		QMatrix4x4 transform() {
			const QVector3D x = vector(), y = vector(), z = vector(), t = vector();
			return QMatrix4x4(x.x(), y.x(), z.x(), t.x(),
							  x.y(), y.y(), z.y(), t.y(),
							  x.z(), y.z(), z.z(), t.z(),
							  0, 0, 0, 1);
		}
		int count() {
			const qint32 n = longValue();
			if (n < 0 || n > m_data.size()) throw std::runtime_error("bad count");
			return n;
		}

	private:
		const QByteArray &m_data;
		int m_pos = 0;
};

struct Elements {int mode = 6; QVector<int> indices;};
struct Mesh {int stride = 0; bool points = false; QVector<float> vertices; QList<Elements> faces;};

Elements readElements(Reader &r, int version)
{
	if (version >= 2) r.vector();   // centre
	Elements e;
	e.mode = r.byte();
	if (e.mode > 6) throw std::runtime_error("unknown primitive mode");
	r.longValue();                  // index count
	const int type = r.byte();
	const int bytes = r.count();
	if (type == 0) {
		const char *p = r.take(bytes);
		for (int i = 0; i < bytes; ++i) e.indices << quint8(p[i]);
	} else if (type == 1) {
		const char *p = r.take(bytes);
		for (int i = 0; i + 1 < bytes; i += 2) e.indices << qFromLittleEndian<quint16>(p + i);
	} else {
		throw std::runtime_error("unknown index type");
	}
	return e;
}

Mesh readMesh(Reader &r, int version)
{
	Mesh m;
	const int flags = r.byte();
	m.points = flags & 1;
	m.stride = (flags & 1 ? 3 : 0) + (flags & 2 ? 3 : 0) + (flags & 4 ? 2 : 0);
	const int bytes = r.count();
	const char *p = r.take(bytes);
	m.vertices.resize(bytes / 4);
	for (int i = 0; i < m.vertices.size(); ++i) {
		const quint32 bits = qFromLittleEndian<quint32>(p + 4 * i);
		std::memcpy(&m.vertices[i], &bits, 4);
	}
	const int faces = r.count();
	for (int i = 0; i < faces; ++i) {
		r.byte(); r.color(); r.shortValue();   // material: colour valid, colour, texture
		m.faces << readElements(r, version);
	}
	const int edges = r.count();
	for (int i = 0; i < edges; ++i) {
		r.color(); r.floatValue(); r.byte();   // edge style
		readElements(r, version);
	}
	return m;
}

} // namespace

PanelMesh PanelE3d::read(const QByteArray &data, QString *error)
{
	PanelMesh out;
	try {
		Reader r(data);
		const int version = r.byte();
		if (version < 1 || version > 4) {
			if (error) *error = QObject::tr("E3D format version %1 is not supported (1 to 4)").arg(version);
			return out;
		}
		const QMatrix4x4 model = r.transform();
		r.vector(); r.color(); r.color();   // view box, background
		const int lights = r.count();
		for (int i = 0; i < lights; ++i) {
			const int type = r.byte();
			r.color();
			if (type == 2 || type == 3) r.vector();
			if (type == 1 || type == 3) r.vector();
			if (type == 3) {r.floatValue(); r.floatValue();}
		}
		const int textures = r.count();
		for (int i = 0; i < textures; ++i) {
			r.byte();
			const int w = r.shortValue(), h = r.shortValue();
			r.skip(w * h * 4);
		}
		QList<Mesh> meshes;
		const int mesh_count = r.count();
		for (int i = 0; i < mesh_count; ++i) meshes << readMesh(r, version);

		QVector3D lo(1e30f, 1e30f, 1e30f), hi(-1e30f, -1e30f, -1e30f);
		const int parts = r.count();
		for (int i = 0; i < parts; ++i) {
			const int mesh_id = r.shortValue();
			const QMatrix4x4 place = model * r.transform();
			r.color();
			if (version >= 3) {r.shortValue(); r.longValue();}
			if (version >= 4) {
				const int lines = r.count();
				for (int l = 0; l < lines; ++l) {r.transform(); r.floatValue(); r.shortValue(); r.skip(r.count());}
			}
			if (mesh_id < 0 || mesh_id >= meshes.size()) continue;
			const Mesh &m = meshes.at(mesh_id);
			if (!m.points || m.stride <= 0) continue;
			const int vertex_count = int(m.vertices.size()) / m.stride;
			auto vertex = [&](int index) {
				if (index < 0 || index >= vertex_count) throw std::runtime_error("index out of range");
				const float *v = m.vertices.constData() + index * m.stride;
				return place.map(QVector3D(v[0], v[1], v[2]));
			};
			auto triangle = [&](int a, int b, int c) {
				const QVector3D p[3] = {vertex(a), vertex(b), vertex(c)};
				const QVector3D n = QVector3D::normal(p[0], p[1], p[2]);
				for (const QVector3D &q : p) {
					out.vertices << q;
					out.normals << n;
					for (int k = 0; k < 3; ++k) {lo[k] = qMin(lo[k], q[k]); hi[k] = qMax(hi[k], q[k]);}
				}
			};
			for (const Elements &e : m.faces) {
				const QVector<int> &ix = e.indices;
				if (e.mode == 6)                  // triangles
					for (int k = 0; k + 2 < ix.size(); k += 3) triangle(ix[k], ix[k + 1], ix[k + 2]);
				else if (e.mode == 4)             // triangle strip
					for (int k = 0; k + 2 < ix.size(); ++k) {
						if (k % 2) triangle(ix[k + 1], ix[k], ix[k + 2]);
						else triangle(ix[k], ix[k + 1], ix[k + 2]);
					}
				else if (e.mode == 5)             // triangle fan
					for (int k = 1; k + 1 < ix.size(); ++k) triangle(ix[0], ix[k], ix[k + 1]);
			}
		}
		if (!r.atEnd()) {
			if (error) *error = QObject::tr("Unexpected data at the end of the E3D file (at %1 of %2 bytes)")
									.arg(r.pos()).arg(data.size());
			return PanelMesh();
		}
		out.min = lo;
		out.max = hi;
		if (out.isEmpty() && error) *error = QObject::tr("The E3D file has no faces to show");
	} catch (const std::exception &e) {
		if (error) *error = QObject::tr("Not a readable E3D file: %1").arg(QString::fromLatin1(e.what()));
		return PanelMesh();
	}
	return out;
}

PanelMesh PanelE3d::readFile(const QString &path, QString *error)
{
	QFile file(path);
	if (!file.open(QIODevice::ReadOnly)) {
		if (error) *error = QObject::tr("File not found: %1").arg(path);
		return PanelMesh();
	}
	QString why;
	PanelMesh mesh = read(file.readAll(), &why);
	if (mesh.isEmpty() && error) *error = QStringLiteral("%1: %2").arg(why, path);
	return mesh;
}
