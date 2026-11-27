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
// SPDX-License-Identifier: GPL-2.0-or-later
#include "networkaddressdialog.h"

#include "../network/networkaddresses.h"
#include "../diagram.h"
#include "../qetdiagrameditor.h"
#include "../qetgraphicsitem/element.h"
#include "../qetproject.h"
#include "../shortcutmanager.h"

#include <QDialogButtonBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QSignalBlocker>
#include <QTableWidget>
#include <QUndoStack>
#include <QVBoxLayout>

namespace {
	enum Column { Label, Location, Folio, Ip, Mask, Network, Profinet, Station, Problems, Count };
}

NetworkAddressDialog::NetworkAddressDialog(QETProject *project, QWidget *parent) :
	QDialog(parent),
	m_project(project)
{
	setWindowTitle(tr("Network addresses"));
	auto *layout = new QVBoxLayout(this);
	auto *net_row = new QHBoxLayout();
	net_row->addWidget(new QLabel(tr("IP networks:"), this));
	m_networks = new QLineEdit(this);
	m_networks->setPlaceholderText(tr("Machine = 10.0.0.0/24, gw 10.0.0.1, from 10; ..."));
	m_networks->setToolTip(tr("One or more networks, apart by ;: name = address/prefix, the gateway "
							  "and the first host to give. A device takes the network its "
							  "information network_name names, else the first."));
	net_row->addWidget(m_networks, 1);
	layout->addLayout(net_row);

	m_table = new QTableWidget(this);
	m_table->setColumnCount(Count);
	m_table->setHorizontalHeaderLabels({tr("Label"), tr("Location"), tr("Folio"), tr("IP address"),
										tr("Subnet mask"), tr("Network"), tr("PROFINET name / address"),
										tr("PLC station"), tr("Problems")});
	m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	m_table->horizontalHeader()->setStretchLastSection(true);
	m_table->verticalHeader()->hide();
	layout->addWidget(m_table, 1);

	auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
	auto *assign = buttons->addButton(tr("Assign IP addresses"), QDialogButtonBox::ActionRole);
	assign->setToolTip(tr("A free address of its network for each device with an IP field and none"));
	auto *names = buttons->addButton(tr("Automatic PROFINET names"), QDialogButtonBox::ActionRole);
	names->setToolTip(tr("A name from location and label (+Q1 -2A1: q1-2a1) for each device with a "
						 "PROFINET field and none; names already written stay"));
	auto *csv = buttons->addButton(tr("Export CSV..."), QDialogButtonBox::ActionRole);
	layout->addWidget(buttons);
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
	connect(assign, &QPushButton::clicked, this, [this]() {
		saveNetworks();
		int count = 0;
		if (QUndoCommand *cmd = NetworkAddresses::assignIpCommand(m_project, &count))
			m_project->undoStack()->push(cmd);
		else if (NetworkAddresses::networks(m_project).isEmpty())
			QMessageBox::information(this, windowTitle(), tr("Give an IP network first."));
		fill();
	});
	connect(names, &QPushButton::clicked, this, [this]() {
		if (QUndoCommand *cmd = NetworkAddresses::profinetNamesCommand(m_project))
			m_project->undoStack()->push(cmd);
		fill();
	});
	connect(csv, &QPushButton::clicked, this, [this]() {
		const QString path = QFileDialog::getSaveFileName(this, tr("Export network addresses"),
				QString(), tr("CSV (*.csv)"));
		if (path.isEmpty()) return;
		QString error;
		if (!NetworkAddresses::exportCsv(m_project, path, &error))
			QMessageBox::warning(this, windowTitle(), error);
	});
	connect(m_networks, &QLineEdit::editingFinished, this, [this]() { saveNetworks(); fill(); });
	connect(m_table, &QTableWidget::cellChanged, this, &NetworkAddressDialog::cellEdited);

	QStringList texts;
	for (const NetworkAddresses::Network &n : NetworkAddresses::networks(project))
		texts << NetworkAddresses::networkText(n);
	m_networks->setText(texts.join(QStringLiteral("; ")));
	resize(980, 520);
	fill();
}

void NetworkAddressDialog::addMenu(QETDiagramEditor *editor, QMenu *project_menu)
{
	QAction *action = project_menu->addAction(tr("Network addresses..."));
	action->setStatusTip(tr("Every device with a network address: IP, subnet, PROFINET name, the "
							"problems; give IP addresses and PROFINET names, export as CSV"));
	ShortcutManager::instance().registerAction(action, "diagrameditor.network_addresses",
											   tr("Diagram editors"), QKeySequence());
	QObject::connect(action, &QAction::triggered, editor, [editor]() {
		QETProject *project = editor->currentProject();
		if (!project) return;
		NetworkAddressDialog dialog(project, editor);
		dialog.exec();
	});
}

void NetworkAddressDialog::saveNetworks()
{
	if (!m_project || m_project->isReadOnly()) return;
	QList<NetworkAddresses::Network> list;
	for (const QString &part : m_networks->text().split(QLatin1Char(';'), Qt::SkipEmptyParts)) {
		NetworkAddresses::Network n;
		if (NetworkAddresses::parseNetwork(part, &n)) list << n;
	}
	NetworkAddresses::setNetworks(m_project, list);
}

void NetworkAddressDialog::fill()
{
	const QSignalBlocker blocker(m_table);
	m_rows.clear();
	const QList<NetworkAddresses::Row> rows = NetworkAddresses::rows(m_project);
	m_table->setRowCount(int(rows.size()));
	for (int i = 0; i < rows.size(); ++i) {
		const NetworkAddresses::Row &r = rows.at(i);
		m_rows << r.element;
		const QStringList texts{r.label, r.location, QString::number(r.folio + 1), r.ip, r.mask,
								r.network, r.profinet, r.station, r.problems.join(QStringLiteral(", "))};
		for (int c = 0; c < Count; ++c) {
			auto *item = new QTableWidgetItem(texts.at(c));
			if (c != Ip && c != Mask && c != Network && c != Profinet)
				item->setFlags(item->flags() & ~Qt::ItemIsEditable);
			if (c == Problems && !r.problems.isEmpty())
				item->setForeground(QColor(0xb0, 0x00, 0x20));
			m_table->setItem(i, c, item);
		}
	}
}

void NetworkAddressDialog::cellEdited(int row, int column)
{
	Element *element = m_rows.value(row);
	if (!element || !m_project || m_project->isReadOnly()) return;
	QString key;
	switch (column) {
		case Ip: key = QStringLiteral("ip_address"); break;
		case Mask: key = QStringLiteral("subnet_mask"); break;
		case Network: key = QStringLiteral("network_name"); break;
		case Profinet: key = QStringLiteral("profinet_name"); break;
		default: return;
	}
	if (QUndoCommand *cmd = NetworkAddresses::setValueCommand(
				element, key, m_table->item(row, column)->text().trimmed()))
		m_project->undoStack()->push(cmd);
	fill();
}
