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
#ifndef PLCLINKWIDGET_H
#define PLCLINKWIDGET_H

#include "abstractelementpropertieseditorwidget.h"

#include <QPointer>
#include <QVector>

class QCheckBox;
class QComboBox;
class QLabel;
class QPushButton;
class Element;

/**
	@brief The PlcLinkWidget class
	Puts a PLC channel (PLC slave) on a channel of a PLC module (PLC
	master) of the project. The module and then the channel are chosen in
	two lists; choosing the channel links at once, one undo step. A
	channel already in use shows who uses it and cannot be chosen. A
	linked channel takes the module's label, the channel's address and its
	terminals.
*/
class PlcLinkWidget : public AbstractElementPropertiesEditorWidget
{
	Q_OBJECT

	public:
		explicit PlcLinkWidget(Element *elmt, QWidget *parent = nullptr);
		~PlcLinkWidget() override = default;

		void setElement(Element *element) override;
		void apply() override;
		QUndoCommand *associatedUndo() const override;
		QString title() const override;

		/// The kind of channel a slave stands for, from its terminal
		/// names: 1 inputs (DI, I...), 2 outputs (DQ, DO, Q...), 0 unknown
		static int channelDirection(const Element *slave);

	public slots:
		void updateUi() override;

	private:
		void fillModules();
		void fillChannels();
		void channelChosen(int combo_index);
		void unlink();
		void showModule();
		Element *currentMaster() const;
		int currentChannel() const;

		QLabel *m_state = nullptr;
		QPushButton *m_unlink_pb = nullptr;
		QPushButton *m_show_module_pb = nullptr;
		QComboBox *m_module_cb = nullptr;
		QComboBox *m_channel_cb = nullptr;
		QCheckBox *m_all_cb = nullptr;
		QLabel *m_hint = nullptr;

		QVector<QPointer<Element>> m_modules;	///< in the order of m_module_cb
		bool m_filling = false;

		Element *m_element_to_link = nullptr;
		int m_pending_io_index = -1;
};

#endif // PLCLINKWIDGET_H
