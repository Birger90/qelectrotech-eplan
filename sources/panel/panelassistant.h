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
#ifndef PANELASSISTANT_H
#define PANELASSISTANT_H

#include <QDialog>
#include <QDockWidget>
#include <QPointer>
#include <functional>

class Diagram;
class QComboBox;
class QLabel;
class QListWidget;
class QListWidgetItem;
class QSpinBox;
class QTimer;
class QetShapeItem;

/**
	@brief The check list of the cabinet layout as a side panel: every
	finding of every surface of the folio (parts lying on each other, in a
	duct, beyond the surface, without holes, terminal strips too short).
	A click selects the part and shows it; the list follows each change.
*/
class PanelCheckDock : public QDockWidget
{
	Q_OBJECT

	public:
		explicit PanelCheckDock(QWidget *parent = nullptr);
		void setDiagram(Diagram *diagram);
		int findings() const {return m_findings;}

	public slots:
		void refresh();

	private:
		void showItem(QListWidgetItem *item);

		QPointer<Diagram> m_diagram;
		QListWidget *m_list = nullptr;
		QLabel *m_summary = nullptr;
		QTimer *m_timer = nullptr;
		int m_findings = 0;
};

/**
	@brief "Build cabinet...": the cabinet layout in a few guided steps
	instead of a dozen single commands -- enclosure with mounting plate,
	rails and ducts as a grid, the devices and terminal strips of the
	schematic, the check, 3D and the manufacturing files. Every step shows
	whether it is done and can be run again.
*/
class PanelAssistant : public QDialog
{
	Q_OBJECT

	public:
		struct Actions {
			std::function<void()> enclosure;     ///< enclosure and mounting plate (dialog)
			std::function<void()> devices;       ///< place the devices of the schematic
			std::function<void()> terminals;     ///< place the terminal strips
			std::function<void()> checks;        ///< show the check list
			std::function<void()> view3d;
			std::function<void()> manufacturing; ///< export the manufacturing files
		};
		PanelAssistant(Diagram *diagram, const Actions &actions, QWidget *parent = nullptr);

			/// rails and ducts on @p surface as a grid: ducts on all four
			/// edges and between the rows, one DIN rail in the middle of each
			/// row; one undo step. Returns the number of parts added.
		static int addGrid(QetShapeItem *surface, int rows, double duct_mm);
			/// the number of rows that fits a plate of @p height_mm
		static int suggestedRows(double height_mm, double duct_mm);

	private:
		void update();

		QPointer<Diagram> m_diagram;
		Actions m_actions;
		QList<QLabel *> m_marks;
		QList<QLabel *> m_states;
		QSpinBox *m_rows = nullptr;
		QComboBox *m_duct = nullptr;
};

#endif // PANELASSISTANT_H
