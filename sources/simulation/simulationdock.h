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
#ifndef SIMULATIONDOCK_H
#define SIMULATIONDOCK_H

#include <QDockWidget>
#include <QElapsedTimer>
#include <QHash>
#include <QPointer>
#include <memory>

class QComboBox;
class QDoubleSpinBox;
class QGraphicsItem;
class QLabel;
class QListWidget;
class QPushButton;
class QTimer;
class QVBoxLayout;
class QETDiagramEditor;
class QETProject;
class Conductor;
class Element;
class SimulationModel;
class SimulationOverlay;

/**
	@brief The SimulationDock class
	Runs the simulation of the current project in real time and shows it
	on the folios: pressurised lines blue (air) or red (oil), live wires
	red, cylinder stroke, valve position and relay state next to the
	symbols. Push buttons and valves are operated from the dock.
	The project itself is not changed: everything drawn is an overlay that
	disappears when the simulation stops.
*/
class SimulationDock : public QDockWidget
{
	Q_OBJECT

	public:
		explicit SimulationDock(QETDiagramEditor *editor);
		~SimulationDock() override;

	protected:
		bool eventFilter(QObject *watched, QEvent *event) override;

	private:
		void start();
		void pause();
		void stop();
		void tick();
		void rebuildControls();
		void updateOverlay();
		void clearOverlay();
		void editProgram();

		QETDiagramEditor *m_editor;
		QPointer<QETProject> m_project;
		std::unique_ptr<SimulationModel> m_model;
		QTimer *m_timer;
		QElapsedTimer m_clock;
		QPushButton *m_start, *m_pause, *m_stop, *m_program;
		QComboBox *m_speed;
		QDoubleSpinBox *m_supply;
		QLabel *m_time, *m_summary;
		QWidget *m_controls;
		QVBoxLayout *m_controls_layout;
		QListWidget *m_log;
		std::unique_ptr<SimulationOverlay> m_overlay;
		QString m_held_button;
		QString m_held_valve;
		bool m_held_left = true;
		QList<QPointer<QObject>> m_watched;
};

#endif // SIMULATIONDOCK_H
