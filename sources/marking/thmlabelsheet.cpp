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
#include "thmlabelsheet.h"

#include <QFontMetricsF>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsTextItem>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPen>
#include <QPointer>
#include <QScrollBar>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextOption>
#include <QTimer>
#include <QWheelEvent>

namespace {
	/// Scene units per millimetre
constexpr double kScale = 10;
	/// Free space around the strip and between two rows, millimetres
constexpr double kBorder = 3;
constexpr double kRowSpace = 2;
}

	/// One label: its outline, its text field and the text in it
class ThmLabelField : public QGraphicsTextItem
{
	public:
		ThmLabelField(ThmLabelSheet *sheet, int index) :
			m_sheet(sheet), m_index(index)
		{
			document()->setDocumentMargin(0);
			QTextOption option = document()->defaultTextOption();
			option.setAlignment(Qt::AlignHCenter);
			option.setWrapMode(QTextOption::NoWrap);
			document()->setDefaultTextOption(option);
			QObject::connect(document(), &QTextDocument::contentsChanged, sheet, [this]() {
				if (m_sheet && !m_sheet->m_building) m_sheet->fieldEdited(this);
			});
		}
		int index() const { return m_index; }
		QRectF label;   ///< the whole label, scene units
		QRectF field;   ///< its text field, scene units

	protected:
		void keyPressEvent(QKeyEvent *event) override
		{
			switch (event->key()) {
				case Qt::Key_Tab:
				case Qt::Key_Return:
				case Qt::Key_Enter:
				case Qt::Key_Down:
					m_sheet->focusNext(this, 1);
					return;
				case Qt::Key_Backtab:
				case Qt::Key_Up:
					m_sheet->focusNext(this, -1);
					return;
				default:
					QGraphicsTextItem::keyPressEvent(event);
			}
		}

	private:
		QPointer<ThmLabelSheet> m_sheet;
		int m_index;
};

ThmLabelSheet::ThmLabelSheet(QWidget *parent) :
	QGraphicsView(parent),
	m_scene(new QGraphicsScene(this))
{
	setScene(m_scene);
	setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
	setBackgroundBrush(QColor(0xa8, 0xa8, 0xa8));
	setAlignment(Qt::AlignHCenter | Qt::AlignTop);
	setMinimumHeight(160);
	setObjectName(QStringLiteral("thm_label_sheet"));
	rebuild();
}

void ThmLabelSheet::setProfile(const ThmJob::Profile &profile, bool known)
{
	m_profile = profile;
	m_known = known;
	rebuild();
}

void ThmLabelSheet::setTexts(const QStringList &texts)
{
	if (texts == this->texts() && !m_fields.isEmpty()) return;
	m_texts = texts;
	m_fields.clear();  // the texts given win over what the fields hold
	rebuild();
}

QStringList ThmLabelSheet::texts() const
{
	QStringList out;
	for (const ThmLabelField *f : m_fields) out << f->toPlainText();
		//Empty labels at the end are only room to go on typing
	while (!out.isEmpty() && out.last().trimmed().isEmpty()) out.removeLast();
	return out;
}

void ThmLabelSheet::setEditable(bool editable)
{
	if (m_editable == editable) return;
	m_editable = editable;
	rebuild();
}

void ThmLabelSheet::focusField(int index)
{
	if (!m_editable || index < 0 || index >= m_fields.size()) return;
	ThmLabelField *f = m_fields.at(index);
	setFocus();
	m_scene->setFocusItem(f);
	QTextCursor c = f->textCursor();
	c.movePosition(QTextCursor::End);
	f->setTextCursor(c);
	ensureVisible(f->label, 10, 10);
}

void ThmLabelSheet::rebuild()
{
	m_building = true;
		//The focus survives a rebuild (a row added while typing)
	int focused = -1;
	int cursor = 0;
	for (int i = 0; i < m_fields.size(); ++i) {
		if (m_fields.at(i)->hasFocus()) {
			focused = i;
			cursor = m_fields.at(i)->textCursor().position();
		}
	}
	if (!m_fields.isEmpty()) m_texts = texts();
	m_fields.clear();
	m_scene->clear();

	ThmJob::Profile p = m_profile;
	if (p.width <= 0 || p.height <= 0) {
			//No material yet: a plain label to type into all the same
		p = ThmJob::Profile();
		p.width = 30;
		p.height = 10;
		p.margin_left = p.margin_right = 1;
	}
	const int columns = qMax(1, p.columns);
	const double pitch = columns > 1 && p.column_pitch > 0 ? p.column_pitch : p.width;
	const double row_width = qMax(ThmJob::rowWidth(p), p.first_x + (columns - 1) * pitch + p.width);
	const double row_step = p.height + qMax(p.gap, kRowSpace);
	const double field_top = p.field_top;
	const double field_bottom = p.field_bottom > 0 ? p.field_bottom : p.height;

	int count = int(m_texts.size()) + (m_editable ? 1 : 0);
	count = qMax(count, columns);
	count = int((count + columns - 1) / columns) * columns;
	count = qMin(count, int(kMaxFields));
	const int rows = count / columns;

	QFont font(QStringLiteral("Arial"));
		//The printer's size is the height of a capital; a font's em is
		//about 1.4 times that
	font.setPixelSize(qMax(1, qRound(p.text_size / 0.72 * kScale)));
	QPen strip_pen(QColor(0x80, 0x80, 0x80), 0);
	QPen field_pen(QColor(0x20, 0x40, 0xd0), 0, Qt::DotLine);
	field_pen.setCosmetic(true);
	const QBrush label_brush(m_known ? QColor(0xff, 0xe8, 0x67) : QColor(0xf0, 0xf0, 0xf0));

	for (int r = 0; r < rows; ++r) {
		const double y = (kBorder + r * row_step) * kScale;
		auto *strip = m_scene->addRect(QRectF(kBorder * kScale, y, row_width * kScale, p.height * kScale),
									   strip_pen, QBrush(Qt::white));
		strip->setZValue(-2);
		for (int c = 0; c < columns; ++c) {
			const int i = r * columns + c;
			const double left = kBorder + p.first_x + c * pitch;
			const double marker_height = p.marker_height > 0 ? p.marker_height : p.height;
			const QRectF label(left * kScale, y + p.marker_top * kScale, p.width * kScale, marker_height * kScale);
			const QRectF field((left + p.margin_left) * kScale, y + field_top * kScale,
							   (p.width - p.margin_left - p.margin_right) * kScale,
							   (field_bottom - field_top) * kScale);
			auto *outline = m_scene->addRect(label, QPen(QColor(0xc0, 0xa0, 0x20), 0), label_brush);
			outline->setZValue(-1);
			auto *box = m_scene->addRect(field, field_pen);
			box->setZValue(-1);
			auto *f = new ThmLabelField(this, i);
			f->label = label;
			f->field = field;
			f->setFont(font);
			f->setDefaultTextColor(Qt::black);
			f->setTextInteractionFlags(m_editable ? Qt::TextEditorInteraction : Qt::NoTextInteraction);
			f->setPlainText(i < m_texts.size() ? m_texts.at(i) : QString());
			m_scene->addItem(f);
			m_fields << f;
			place(f);
		}
	}
	const int all = int(m_texts.size());
	double below = kBorder + rows * row_step;
	if (all > kMaxFields) {
		auto *more = m_scene->addSimpleText(tr("... and %n more label(s)", nullptr, all - kMaxFields));
		more->setPos(kBorder * kScale, below * kScale);
		below += 6;
	}
	if (!m_known) {
		auto *note = m_scene->addSimpleText(tr("Choose the material: until then a plain label is shown."));
		note->setPos(kBorder * kScale, below * kScale);
	}
	m_scene->setSceneRect(m_scene->itemsBoundingRect().adjusted(-kBorder * kScale, -kBorder * kScale,
																 kBorder * kScale, kBorder * kScale));
	fitWidth();
	m_building = false;
	if (focused < 0) verticalScrollBar()->setValue(verticalScrollBar()->minimum());
	if (focused >= 0 && focused < m_fields.size()) {
		ThmLabelField *f = m_fields.at(focused);
		setFocus();
		m_scene->setFocusItem(f);
		QTextCursor c = f->textCursor();
		c.setPosition(qMin(cursor, int(f->toPlainText().size())));
		f->setTextCursor(c);
	}
}

void ThmLabelSheet::place(ThmLabelField *f)
{
	const int r = ((m_profile.text_rotation % 360) + 360) % 360;
	const bool across = r == 90 || r == 270;
		//The text centred in its field, turned as it is printed (JScript
		//turns anticlockwise, Qt clockwise)
	f->setTextWidth(across ? f->field.height() : f->field.width());
	const QRectF br = f->boundingRect();
	f->setTransformOriginPoint(br.center());
	f->setRotation(-r);
		//Centre the capitals, not the line box: the line has more room
		//above them (ascent) than below (descent)
	const QFontMetricsF fm(f->font());
	const double shift = (fm.descent() - fm.ascent() + fm.capHeight()) / 2;
	const QPointF along = QTransform().rotate(-r).map(QPointF(0, shift));
	f->setPos(f->field.center() - br.center() + along);
}

void ThmLabelSheet::fieldEdited(ThmLabelField *field)
{
	place(field);
	const QStringList now = texts();
		//Writing in the last label: a new row to go on
	if (m_editable && field->index() == m_fields.size() - 1 && !field->toPlainText().isEmpty()) {
		QTimer::singleShot(0, this, [this]() { rebuild(); });
	}
	emit textsChanged(now);
}

void ThmLabelSheet::focusNext(ThmLabelField *field, int step)
{
	const int next = field->index() + step;
	if (next < 0) return;
	if (next >= m_fields.size()) {
		m_texts = texts();
		while (m_texts.size() <= field->index()) m_texts << QString();
		m_fields.clear();
		rebuild();
	}
	focusField(qMin(next, int(m_fields.size()) - 1));
}

void ThmLabelSheet::fitWidth()
{
	const QRectF r = m_scene->sceneRect();
	if (r.width() <= 0 || viewport()->width() <= 0) return;
	const double s = viewport()->width() / r.width() * m_zoom;
	setTransform(QTransform::fromScale(s, s));
}

void ThmLabelSheet::resizeEvent(QResizeEvent *event)
{
	QGraphicsView::resizeEvent(event);
	fitWidth();
	if (!m_scene->focusItem()) verticalScrollBar()->setValue(verticalScrollBar()->minimum());
}

void ThmLabelSheet::wheelEvent(QWheelEvent *event)
{
	if (event->modifiers() & Qt::ControlModifier) {
		m_zoom = qBound(0.5, m_zoom * (event->angleDelta().y() > 0 ? 1.15 : 1 / 1.15), 8.0);
		fitWidth();
		event->accept();
		return;
	}
	QGraphicsView::wheelEvent(event);
}

void ThmLabelSheet::mousePressEvent(QMouseEvent *event)
{
		//A click anywhere on a label writes into it
	const QPointF at = mapToScene(event->position().toPoint());
	for (ThmLabelField *f : std::as_const(m_fields)) {
		if (f->label.contains(at) && !f->sceneBoundingRect().contains(at)) {
			focusField(f->index());
			event->accept();
			return;
		}
	}
	QGraphicsView::mousePressEvent(event);
}
