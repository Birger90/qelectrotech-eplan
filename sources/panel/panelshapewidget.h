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
#ifndef PANELSHAPEWIDGET_H
#define PANELSHAPEWIDGET_H

#include "../PropertiesEditor/propertieseditorwidget.h"

#include <QPointer>

class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QetShapeItem;

/**
	@brief The properties panel of a part of the cabinet layout (cable
	duct, DIN rail, C profile, mounting surface ...): what one adjusts
	every day -- name, length, width, height, part -- right in the side
	panel after a single click. Each change is one undo step; the full
	manufacturing data stay one button away.
*/
class PanelShapePropertiesWidget : public PropertiesEditorWidget
{
	Q_OBJECT

	public:
		explicit PanelShapePropertiesWidget(QetShapeItem *shape, QWidget *parent = nullptr);
		void setItem(QetShapeItem *shape);
		QString title() const override;
		void updateUi() override;

	private:
		double mmPerUnit() const;
		void applyData();
		void applySize();

		QPointer<QetShapeItem> m_shape;
		QLabel *m_role = nullptr;
		QLineEdit *m_designation = nullptr;
		QDoubleSpinBox *m_length = nullptr;
		QDoubleSpinBox *m_width = nullptr;
		QDoubleSpinBox *m_depth = nullptr;
		QComboBox *m_face = nullptr;
		QLineEdit *m_manufacturer = nullptr;
		QLineEdit *m_article = nullptr;
		QLabel *m_length_label = nullptr;
		QLabel *m_width_label = nullptr;
		QLabel *m_depth_label = nullptr;
		QLabel *m_face_label = nullptr;
		bool m_updating = false;
};

#endif // PANELSHAPEWIDGET_H
