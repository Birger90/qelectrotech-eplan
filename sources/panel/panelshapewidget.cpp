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
#include "panelshapewidget.h"

#include "panelcommands.h"
#include "panellayout.h"
#include "../QPropertyUndoCommand/qpropertyundocommand.h"
#include "../diagram.h"
#include "../qetgraphicsitem/qetshapeitem.h"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QUndoStack>
#include <QVBoxLayout>

PanelShapePropertiesWidget::PanelShapePropertiesWidget(QetShapeItem *shape, QWidget *parent) :
	PropertiesEditorWidget(parent)
{
	auto mm = [this](double max) {
		auto *b = new QDoubleSpinBox(this);
		b->setRange(0, max);
		b->setDecimals(1);
		b->setSuffix(QStringLiteral(" mm"));
		b->setKeyboardTracking(false);
		return b;
	};
	m_role = new QLabel(this);
	m_designation = new QLineEdit(this);
	m_length = mm(10000);
	m_width = mm(5000);
	m_depth = mm(1000);
	m_face = new QComboBox(this);
	m_face->addItem(tr("Mounting plate (inside)"), QString());
	m_face->addItem(tr("Door"), QStringLiteral("door"));
	m_face->addItem(tr("Left side wall"), QStringLiteral("left"));
	m_face->addItem(tr("Right side wall"), QStringLiteral("right"));
	m_manufacturer = new QLineEdit(this);
	m_article = new QLineEdit(this);
	m_length_label = new QLabel(this);
	m_width_label = new QLabel(this);
	m_depth_label = new QLabel(tr("Height above the surface"), this);
	m_face_label = new QLabel(tr("Where"), this);

	auto *form = new QFormLayout;
	form->addRow(tr("Kind"), m_role);
	form->addRow(tr("Designation"), m_designation);
	form->addRow(m_length_label, m_length);
	form->addRow(m_width_label, m_width);
	form->addRow(m_depth_label, m_depth);
	form->addRow(m_face_label, m_face);
	form->addRow(tr("Manufacturer"), m_manufacturer);
	form->addRow(tr("Article number"), m_article);
	auto *more = new QPushButton(tr("All manufacturing data..."), this);
	more->setToolTip(tr("Fixing holes, spacing, scale, STEP model -- the full dialog (also by double click)"));
	auto *box = new QVBoxLayout(this);
	box->addLayout(form);
	box->addWidget(more);
	box->addStretch();

	connect(m_designation, &QLineEdit::editingFinished, this, &PanelShapePropertiesWidget::applyData);
	connect(m_manufacturer, &QLineEdit::editingFinished, this, &PanelShapePropertiesWidget::applyData);
	connect(m_article, &QLineEdit::editingFinished, this, &PanelShapePropertiesWidget::applyData);
	connect(m_depth, &QDoubleSpinBox::valueChanged, this, &PanelShapePropertiesWidget::applyData);
	connect(m_face, &QComboBox::currentIndexChanged, this, &PanelShapePropertiesWidget::applyData);
	connect(m_length, &QDoubleSpinBox::valueChanged, this, &PanelShapePropertiesWidget::applySize);
	connect(m_width, &QDoubleSpinBox::valueChanged, this, &PanelShapePropertiesWidget::applySize);
	connect(more, &QPushButton::clicked, this, [this]() {
		if (m_shape && QetShapeItem::panel_editor) QetShapeItem::panel_editor(m_shape);
		updateUi();
	});
	setItem(shape);
}

void PanelShapePropertiesWidget::setItem(QetShapeItem *shape)
{
	m_shape = shape;
	updateUi();
}

QString PanelShapePropertiesWidget::title() const
{
	return tr("Cabinet layout part");
}

/**
	The drawing scale of the shape: its own for a surface or an enclosure,
	else the one of the surface it lies on.
*/
double PanelShapePropertiesWidget::mmPerUnit() const
{
	if (!m_shape) return 1;
	const PanelData data = m_shape->panelData();
	if (data.role == PanelData::Surface || data.role == PanelData::Enclosure)
		return qMax(1e-6, data.mm_per_unit);
	if (auto *d = qobject_cast<Diagram *>(m_shape->scene())) {
		const QPointF c = m_shape->sceneBoundingRect().center();
		for (QetShapeItem *s : PanelLayout::surfaces(d))
			if (s->sceneBoundingRect().contains(c)) return qMax(1e-6, s->panelData().mm_per_unit);
	}
	return 1;
}

void PanelShapePropertiesWidget::updateUi()
{
	if (!m_shape) return;
	m_updating = true;
	const PanelData data = m_shape->panelData();
	const bool surface = data.role == PanelData::Surface;
	const bool profile = data.role == PanelData::DinRail || data.role == PanelData::Duct
						 || data.role == PanelData::CProfile;
	QString kind = PanelData::roleName(data.role);
	switch (data.role) {
		case PanelData::Surface:   kind = tr("Mounting surface"); break;
		case PanelData::DinRail:   kind = tr("DIN rail"); break;
		case PanelData::Duct:      kind = tr("Cable duct"); break;
		case PanelData::CProfile:  kind = tr("C profile"); break;
		case PanelData::Cutout:    kind = tr("Cut-out"); break;
		case PanelData::Keepout:   kind = tr("Keep-out area"); break;
		case PanelData::Enclosure: kind = tr("Enclosure"); break;
		default: break;
	}
	m_role->setText(QStringLiteral("<b>%1</b>").arg(kind));
	m_designation->setText(data.designation);
	m_manufacturer->setText(data.manufacturer);
	m_article->setText(data.article);

	const QRectF r = m_shape->rect().normalized();
	const double k = mmPerUnit();
	const bool horizontal = r.width() >= r.height();
	if (profile) {
		m_length_label->setText(tr("Length"));
		m_width_label->setText(tr("Width"));
		m_length->setValue((horizontal ? r.width() : r.height()) * k);
		m_width->setValue((horizontal ? r.height() : r.width()) * k);
	} else {
		m_length_label->setText(tr("Width"));
		m_width_label->setText(tr("Height"));
		m_length->setValue(r.width() * k);
		m_width->setValue(r.height() * k);
	}
	m_depth->setValue(data.profile_depth);
	m_depth_label->setVisible(profile || data.role == PanelData::Enclosure);
	m_depth->setVisible(profile || data.role == PanelData::Enclosure);
	m_face_label->setVisible(surface);
	m_face->setVisible(surface);
	m_face->setCurrentIndex(qMax(0, m_face->findData(data.face)));
	m_updating = false;
}

void PanelShapePropertiesWidget::applyData()
{
	if (m_updating || !m_shape) return;
	auto *d = qobject_cast<Diagram *>(m_shape->scene());
	if (!d) return;
	const PanelData old = m_shape->panelData();
	PanelData data = old;
	data.designation = m_designation->text().trimmed();
	data.manufacturer = m_manufacturer->text().trimmed();
	data.article = m_article->text().trimmed();
	if (m_depth->isVisible()) data.profile_depth = m_depth->value();
	if (m_face->isVisible()) data.face = m_face->currentData().toString();
	if (data.designation == old.designation && data.manufacturer == old.manufacturer
		&& data.article == old.article && data.profile_depth == old.profile_depth && data.face == old.face)
		return;
	d->undoStack().push(new ChangePanelDataCommand(m_shape, data));
}

void PanelShapePropertiesWidget::applySize()
{
	if (m_updating || !m_shape) return;
	auto *d = qobject_cast<Diagram *>(m_shape->scene());
	if (!d) return;
	const PanelData data = m_shape->panelData();
	const bool profile = data.role == PanelData::DinRail || data.role == PanelData::Duct
						 || data.role == PanelData::CProfile;
	const QRectF r = m_shape->rect().normalized();
	const double k = mmPerUnit();
	const double a = m_length->value() / k, b = m_width->value() / k;
	if (a <= 0 || b <= 0) return;
	QRectF n;
	if (profile && r.width() < r.height())
			// a vertical profile: length downwards, width around its centre line
		n = QRectF(r.center().x() - b / 2, r.top(), b, a);
	else if (profile)
			// a horizontal profile: length to the right, width around its centre line
		n = QRectF(r.left(), r.center().y() - b / 2, a, b);
	else
		n = QRectF(r.topLeft(), QSizeF(a, b));
	if (qAbs(n.width() - r.width()) < 1e-6 && qAbs(n.height() - r.height()) < 1e-6 && n.topLeft() == r.topLeft()) return;
	auto *undo = new QPropertyUndoCommand(m_shape.data(), "rect", QVariant(r), QVariant(n));
	undo->setText(tr("change size of %1").arg(data.designation.isEmpty() ? m_role->text() : data.designation));
	d->undoStack().push(undo);
}
