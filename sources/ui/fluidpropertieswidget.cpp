// SPDX-License-Identifier: GPL-2.0-or-later
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
#include "fluidpropertieswidget.h"

#include "../diagram.h"
#include "../qetgraphicsitem/element.h"
#include "../undocommand/changeelementdatacommand.h"

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QStringList>
#include <QVBoxLayout>

namespace
{
	QString number(double d)
	{
		return QString::number(d, 'g', 6);
	}
}

FluidPropertiesWidget::FluidPropertiesWidget(Element *element,
											 QWidget *parent) :
	AbstractElementPropertiesEditorWidget(parent)
{
	m_kind_label = new QLabel(this);
	m_valve_label = new QLabel(this);
	m_left_label = new QLabel(this);
	m_right_label = new QLabel(this);
	m_action_label = new QLabel(this);
	for (QLabel *label : {m_kind_label, m_valve_label, m_left_label,
						  m_right_label, m_action_label}) {
		label->setWordWrap(true);
		label->setTextInteractionFlags(Qt::TextSelectableByMouse);
	}
	m_bore_sb = newLengthBox();
	m_rod_sb = newLengthBox();
	m_stroke_sb = newLengthBox();
	m_reset_pb = new QPushButton(tr("Use the values of the symbol"), this);

	m_form = new QFormLayout();
	m_form->addRow(tr("Kind:"), m_kind_label);
	m_form->addRow(tr("Positions:"), m_valve_label);
	m_form->addRow(tr("Left actuation:"), m_left_label);
	m_form->addRow(tr("Right actuation:"), m_right_label);
	m_form->addRow(tr("Action:"), m_action_label);
	m_form->addRow(tr("Bore:"), m_bore_sb);
	m_form->addRow(tr("Rod:"), m_rod_sb);
	m_form->addRow(tr("Stroke:"), m_stroke_sb);
	m_form->addRow(QString(), m_reset_pb);

	QLabel *help = new QLabel(tr("A value set here applies to this placed "
								 "cylinder only. At 0, the value of the "
								 "symbol applies."), this);
	help->setWordWrap(true);
	m_form->addRow(help);

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addLayout(m_form);
	layout->addStretch();

	connect(m_reset_pb, &QPushButton::clicked, this, [this]() {
		for (QDoubleSpinBox *box : {m_bore_sb, m_rod_sb, m_stroke_sb})
			box->setValue(0.0);
		if (m_live_edit)
			apply();
	});

	setElement(element);
}

bool FluidPropertiesWidget::isWanted(const Element *element)
{
	return element && !element->elementData().m_fluid.isNull();
}

void FluidPropertiesWidget::setElement(Element *element)
{
	m_element = element;
	updateUi();
}

QString FluidPropertiesWidget::title() const
{
	return tr("Fluid power");
}

/**
	@return a box for a length in millimetres, 0 showing the value of the
	definition (setDefinitionValue())
*/
QDoubleSpinBox *FluidPropertiesWidget::newLengthBox()
{
	QDoubleSpinBox *box = new QDoubleSpinBox(this);
	box->setRange(0.0, 100000.0);
	box->setDecimals(1);
	box->setSuffix(tr(" mm", "length unit"));
	box->setAccelerated(true);
	return box;
}

/**
	Show in @p box, while it is 0, the value @p definition of the symbol
*/
void FluidPropertiesWidget::setDefinitionValue(QDoubleSpinBox *box,
											   double definition)
{
	box->setSpecialValueText(definition > 0.0
		? tr("%1 mm (symbol)", "value of the symbol definition")
		  .arg(number(definition))
		: tr("Not given"));
}

/**
	@return the kind of fluid part @p kind is, as a word for the user
*/
QString FluidPropertiesWidget::kindText(FluidData::Kind kind)
{
	switch (kind) {
		case FluidData::Kind::None:        return QString();
		case FluidData::Kind::Valve:       return tr("Directional valve");
		case FluidData::Kind::Cylinder:    return tr("Cylinder");
		case FluidData::Kind::Source:      return tr("Supply");
		case FluidData::Kind::Treatment:   return tr("Air treatment");
		case FluidData::Kind::FlowControl: return tr("Flow or shut-off valve");
		case FluidData::Kind::LogicValve:  return tr("Logic valve");
		case FluidData::Kind::Sensor:      return tr("Sensor or gauge");
		case FluidData::Kind::Other:       return tr("Other fluid part");
		case FluidData::Kind::Solenoid:    return tr("Valve solenoid");
	}
	return QString();
}

/**
	@return how one side of a valve is actuated, in words: "solenoid,
	manual, pilot", or "None"
*/
QString FluidPropertiesWidget::actuationText(
		const FluidData::Actuation &actuation)
{
	static const QList<QPair<QString, const char *>> names {
		{QStringLiteral("solenoid"),    QT_TR_NOOP("solenoid")},
		{QStringLiteral("manual"),      QT_TR_NOOP("manual")},
		{QStringLiteral("button"),      QT_TR_NOOP("push button")},
		{QStringLiteral("lever"),       QT_TR_NOOP("lever")},
		{QStringLiteral("pedal"),       QT_TR_NOOP("pedal")},
		{QStringLiteral("detent"),      QT_TR_NOOP("detent")},
		{QStringLiteral("plunger"),     QT_TR_NOOP("plunger")},
		{QStringLiteral("roller"),      QT_TR_NOOP("roller")},
		{QStringLiteral("idle_roller"), QT_TR_NOOP("idle roller")},
		{QStringLiteral("pneumatic"),   QT_TR_NOOP("pneumatic")}};

	QStringList words;
	for (const QString &type : actuation.types) {
		QString word = type;
		for (const auto &name : names)
			if (name.first == type)
				word = tr(name.second);
		words << word;
	}
	if (actuation.pilot)      words << tr("pilot");
	if (actuation.spring)     words << tr("spring");
	if (actuation.air_spring) words << tr("air spring");
	if (words.isEmpty())
		return tr("None", "valve actuation");
	return words.join(QStringLiteral(", "));
}

/**
	@brief FluidPropertiesWidget::updateUi
	Show the fluid data of the element: the rows a valve or a cylinder
	has, the others hidden
*/
void FluidPropertiesWidget::updateUi()
{
	if (!m_element)
		return;
	const ElementData data = m_element->elementData();
	const FluidData &definition = data.m_fluid;
	const FluidOverride &own = data.m_fluid_override;
	const bool valve = definition.m_kind == FluidData::Kind::Valve;
	const bool cylinder = definition.m_kind == FluidData::Kind::Cylinder;

	m_kind_label->setText(kindText(definition.m_kind));

	m_form->setRowVisible(m_valve_label, valve);
	m_form->setRowVisible(m_left_label, valve);
	m_form->setRowVisible(m_right_label, valve);
	if (valve) {
		m_valve_label->setText(
			tr("%n position(s), at rest in position %1", nullptr,
			   static_cast<int>(definition.m_positions.size()))
			.arg(definition.m_initial_position));
		m_left_label->setText(actuationText(definition.m_left));
		m_right_label->setText(actuationText(definition.m_right));
	}

	for (QWidget *w : {static_cast<QWidget *>(m_action_label),
					   static_cast<QWidget *>(m_bore_sb),
					   static_cast<QWidget *>(m_rod_sb),
					   static_cast<QWidget *>(m_stroke_sb),
					   static_cast<QWidget *>(m_reset_pb)})
		m_form->setRowVisible(w, cylinder);
		// The help is the last row
	m_form->setRowVisible(m_form->rowCount() - 1, cylinder);
	if (cylinder) {
		QString action = definition.m_action
				== FluidData::CylinderAction::Single
				? tr("single acting") : tr("double acting");
		if (definition.m_spring == FluidData::SpringSide::Rod)
			action += QStringLiteral(", ") + tr("spring on the rod side");
		else if (definition.m_spring == FluidData::SpringSide::Piston)
			action += QStringLiteral(", ")
					  + tr("spring on the piston side");
		m_action_label->setText(action);

		connectLiveEdit(false);
		setDefinitionValue(m_bore_sb, definition.m_bore_mm);
		setDefinitionValue(m_rod_sb, definition.m_rod_mm);
		setDefinitionValue(m_stroke_sb, definition.m_stroke_mm);
		m_bore_sb->setValue(own.m_bore_mm);
		m_rod_sb->setValue(own.m_rod_mm);
		m_stroke_sb->setValue(own.m_stroke_mm);
		connectLiveEdit(m_live_edit);
	}
}

void FluidPropertiesWidget::connectLiveEdit(bool connect_it)
{
	for (QDoubleSpinBox *box : {m_bore_sb, m_rod_sb, m_stroke_sb}) {
		if (connect_it)
			connect(box, &QDoubleSpinBox::editingFinished,
					this, &FluidPropertiesWidget::apply,
					Qt::UniqueConnection);
		else
			disconnect(box, &QDoubleSpinBox::editingFinished,
					   this, &FluidPropertiesWidget::apply);
	}
}

bool FluidPropertiesWidget::setLiveEdit(bool live_edit)
{
	if (m_live_edit == live_edit)
		return true;
	m_live_edit = live_edit;
	connectLiveEdit(m_live_edit);
	return true;
}

/**
	@return what the boxes show; nothing for anything but a cylinder
*/
FluidOverride FluidPropertiesWidget::currentOverride() const
{
	FluidOverride own;
	if (!m_element || m_element->elementData().m_fluid.m_kind
			!= FluidData::Kind::Cylinder)
		return own;
	own.m_bore_mm = m_bore_sb->value();
	own.m_rod_mm = m_rod_sb->value();
	own.m_stroke_mm = m_stroke_sb->value();
	return own;
}

QUndoCommand *FluidPropertiesWidget::associatedUndo() const
{
	if (!m_element)
		return nullptr;
	ElementData data = m_element->elementData();
	const FluidOverride own = currentOverride();
	if (own == data.m_fluid_override)
		return nullptr;
	data.m_fluid_override = own;
	auto undo = new ChangeElementDataCommand(m_element.data(), data);
	undo->setText(tr("Set the fluid power values"));
	return undo;
}

void FluidPropertiesWidget::apply()
{
	if (!m_element || !m_element->diagram())
		return;
	if (QUndoCommand *undo = associatedUndo())
		m_element->diagram()->undoStack().push(undo);
}
