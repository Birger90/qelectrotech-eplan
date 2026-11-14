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
#ifndef FLUIDPROPERTIESWIDGET_H
#define FLUIDPROPERTIESWIDGET_H

#include "abstractelementpropertieseditorwidget.h"
#include "../fluid/fluiddata.h"

class QDoubleSpinBox;
class QFormLayout;
class QLabel;
class QPushButton;

/**
	@brief The FluidPropertiesWidget class
	The "Fluid power" tab of a placed symbol's properties, for a symbol
	with fluid data: what it is, how a valve is actuated, and the bore,
	rod and stroke of a cylinder. Those three can be set for this placed
	cylinder alone (FluidOverride); left at 0 they are those of the
	symbol's definition.
*/
class FluidPropertiesWidget : public AbstractElementPropertiesEditorWidget
{
		Q_OBJECT
	public:
		explicit FluidPropertiesWidget(Element *element,
									   QWidget *parent = nullptr);

			/// Whether @p element gets this tab: it has fluid data
		static bool isWanted(const Element *element);

		void setElement(Element *element) override;
		void apply() override;
		void reset() override {updateUi();}
		QUndoCommand *associatedUndo() const override;
		QString title() const override;
		void updateUi() override;
		bool setLiveEdit(bool live_edit) override;

		static QString kindText(FluidData::Kind kind);
		static QString actuationText(const FluidData::Actuation &actuation);

	private:
		FluidOverride currentOverride() const;
		QDoubleSpinBox *newLengthBox();
		void setDefinitionValue(QDoubleSpinBox *box, double definition);
		void connectLiveEdit(bool connect_it);

		QLabel *m_kind_label = nullptr;
		QLabel *m_valve_label = nullptr;
		QLabel *m_left_label = nullptr;
		QLabel *m_right_label = nullptr;
		QLabel *m_action_label = nullptr;
		QDoubleSpinBox *m_bore_sb = nullptr;
		QDoubleSpinBox *m_rod_sb = nullptr;
		QDoubleSpinBox *m_stroke_sb = nullptr;
		QPushButton *m_reset_pb = nullptr;
		QFormLayout *m_form = nullptr;
};

#endif // FLUIDPROPERTIESWIDGET_H
