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
#include "cableoverviewdialog.h"

#include "cable.h"
#include "cablecreatedialog.h"
#include "../diagram.h"
#include "../qetproject.h"

#include <QCollator>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTableView>
#include <QVBoxLayout>

#include <algorithm>

/**
	@brief CableOverviewDialog::CableOverviewDialog
	@param project the project whose cables are listed
	@param parent
*/
CableOverviewDialog::CableOverviewDialog(QETProject *project, QWidget *parent) :
	QDialog(parent),
	m_project(project)
{
	setWindowTitle(tr("Cable list"));
	resize(900, 520);

	auto *layout = new QVBoxLayout(this);

	auto *search_row = new QHBoxLayout;
	search_row->addWidget(new QLabel(tr("Search:"), this));
	m_search = new QLineEdit(this);
	m_search->setClearButtonEnabled(true);
	m_search->setPlaceholderText(tr("Filter cables"));
	search_row->addWidget(m_search, 1);
	layout->addLayout(search_row);

	m_model = new QStandardItemModel(this);
	m_proxy = new QSortFilterProxyModel(this);
	m_proxy->setSourceModel(m_model);
	m_proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
	m_proxy->setFilterKeyColumn(-1);
	connect(m_search, &QLineEdit::textChanged, m_proxy, &QSortFilterProxyModel::setFilterFixedString);

	m_table = new QTableView(this);
	m_table->setModel(m_proxy);
	m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_table->setSelectionMode(QAbstractItemView::SingleSelection);
	m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_table->setAlternatingRowColors(true);
	m_table->setSortingEnabled(true);
	m_table->horizontalHeader()->setStretchLastSection(true);
	m_table->verticalHeader()->setVisible(false);
	layout->addWidget(m_table, 1);
	connect(m_table, &QTableView::doubleClicked, this, &CableOverviewDialog::chooseType);

	auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
	m_choose = buttons->addButton(tr("Choose type..."), QDialogButtonBox::ActionRole);
	m_choose->setToolTip(tr("Choose the type of the selected cable from your cable "
							"type list or a manufacturer catalog"));
	m_choose->setEnabled(false);
	connect(m_choose, &QPushButton::clicked, this, &CableOverviewDialog::chooseType);
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
	layout->addWidget(buttons);

	fill();
	connect(m_table->selectionModel(), &QItemSelectionModel::selectionChanged,
			this, &CableOverviewDialog::selectionChanged);
}

/**
	@brief CableOverviewDialog::fill
	One line per cable: designation, type, cores (used / of), colours,
	plant, location and the folios it is drawn on.
*/
void CableOverviewDialog::fill()
{
	const int kept_row = m_table->currentIndex().isValid()
			? m_proxy->mapToSource(m_table->currentIndex()).row() : -1;
	m_model->clear();
	m_cables.clear();
	m_model->setHorizontalHeaderLabels({tr("Designation"), tr("Type"), tr("Cores"),
										tr("Core colors"), tr("Plant"), tr("Location"),
										tr("Folios")});
	if (!m_project) return;

	QVector<Cable *> cables = m_project->cables();
	QCollator collator;
	collator.setNumericMode(true);
	std::sort(cables.begin(), cables.end(), [&collator](Cable *a, Cable *b) {
		return collator.compare(a->designation(), b->designation()) < 0;
	});

	for (Cable *cable : std::as_const(cables))
	{
		QStringList folios;
		for (const CablePartData &part : cable->parts())
		{
			Diagram *d = m_project->diagramByUuid(part.diagram);
			if (!d) continue;
			const QString f = d->border_and_titleblock.finalfolio();
			if (!folios.contains(f)) folios << f;
		}
		auto item = [](const QString &text) {
			auto *i = new QStandardItem(text);
			i->setEditable(false);
			return i;
		};
		m_model->appendRow({item(cable->designation()),
							item(cable->type().isEmpty() ? tr("(no type)") : cable->type()),
							item(QStringLiteral("%1 / %2").arg(cable->usedCoreCount())
															 .arg(cable->coreCount())),
							item(cable->coreColors().join(QStringLiteral(", "))),
							item(cable->installation()),
							item(cable->location()),
							item(folios.join(QStringLiteral(", ")))});
		m_cables.append(cable);
	}
	m_table->resizeColumnsToContents();
	if (kept_row >= 0 && kept_row < m_model->rowCount())
		m_table->setCurrentIndex(m_proxy->mapFromSource(m_model->index(kept_row, 0)));
}

/**
	@brief CableOverviewDialog::currentCable
	@return the cable of the selected line, or nullptr
*/
Cable *CableOverviewDialog::currentCable() const
{
	const QModelIndex index = m_table->currentIndex();
	if (!index.isValid()) return nullptr;
	const int row = m_proxy->mapToSource(index).row();
	return row >= 0 && row < m_cables.size() ? m_cables.at(row).data() : nullptr;
}

void CableOverviewDialog::selectionChanged()
{
	m_choose->setEnabled(currentCable() != nullptr);
}

/**
	@brief CableOverviewDialog::chooseType
	Open the type window of the selected cable, on the first folio it is
	drawn on, then show the list again with what was chosen.
*/
void CableOverviewDialog::chooseType()
{
	Cable *cable = currentCable();
	if (!cable || !m_project) return;
	Diagram *diagram = nullptr;
	for (const CablePartData &part : cable->parts())
		if ((diagram = m_project->diagramByUuid(part.diagram))) break;
	if (!diagram) return;

	CableCreateDialog dialog(diagram, cable, this);
	if (dialog.exec() == QDialog::Accepted)
		fill();
}
