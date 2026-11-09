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
#include "articledialog.h"
#include "../import/portals/manufacturerportals.h"

#include "../qetapp.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QPushButton>
#include <QSettings>
#include <QTreeWidget>
#include <QVBoxLayout>

ArticleDialog::ArticleDialog(QWidget *parent, const QString &what, const QString &search) :
	QDialog(parent)
{
	setWindowTitle(tr("Part of %1 from the EPLAN Data Portal").arg(what));
	resize(900, 520);
	m_client = new DataPortalClient(this);
	m_client->setLanguage(QLocale(QETApp::langFromSetting()).name());
	const QString token = ManufacturerPortals::dataPortalToken();
	if (!token.isEmpty()) m_client->setToken(token);

	m_search = new QLineEdit(search, this);
	m_search->setPlaceholderText(tr("Order number, type or text, e.g. 3RT2015-1BB41"));
	m_manufacturer = new QLineEdit(this);
	m_manufacturer->setMaximumWidth(80);
	m_manufacturer->setToolTip(tr("Short name of the manufacturer (SIE = Siemens, WEI = Weidmüller), empty: all"));
	m_exact = new QCheckBox(tr("Exact"), this);
	m_exact->setChecked(true);
	auto *go = new QPushButton(tr("Search"), this);
	auto *row = new QHBoxLayout;
	row->addWidget(m_search, 1);
	row->addWidget(new QLabel(tr("Manufacturer"), this));
	row->addWidget(m_manufacturer);
	row->addWidget(m_exact);
	row->addWidget(go);

	m_list = new QTreeWidget(this);
	m_list->setRootIsDecorated(false);
	m_list->setHeaderLabels({tr("Manufacturer"), tr("Order number"), tr("Type"), tr("Designation"), tr("Product group")});
	m_list->header()->setStretchLastSection(true);
	m_status = new QLabel(this);
	m_status->setWordWrap(true);

	auto *buttons = new QDialogButtonBox(QDialogButtonBox::Cancel, this);
	m_take = buttons->addButton(tr("Assign"), QDialogButtonBox::AcceptRole);
	m_take->setEnabled(false);
		// Enter in the search field searches, it does not assign
	m_take->setAutoDefault(false);
	go->setAutoDefault(true);
	go->setDefault(true);
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
	connect(m_take, &QPushButton::clicked, this, &ArticleDialog::take);
	connect(m_list, &QTreeWidget::itemSelectionChanged, this, [this]() {
		m_take->setEnabled(!m_list->selectedItems().isEmpty());
	});
	connect(m_list, &QTreeWidget::itemDoubleClicked, this, &ArticleDialog::take);
	connect(go, &QPushButton::clicked, this, &ArticleDialog::runSearch);
	connect(m_search, &QLineEdit::returnPressed, this, &ArticleDialog::runSearch);
	connect(m_client, &DataPortalClient::searchFinished, this, &ArticleDialog::showParts);
	connect(m_client, &DataPortalClient::failed, this, [this](const QString &message) {m_status->setText(message);});

	auto *box = new QVBoxLayout(this);
	box->addLayout(row);
	box->addWidget(m_list, 1);
	box->addWidget(m_status);
	box->addWidget(buttons);
	if (!search.trimmed().isEmpty()) runSearch();
	else m_status->setText(tr("Type the order number or the type of the part and search."));
}

void ArticleDialog::runSearch()
{
	if (m_search->text().trimmed().isEmpty()) return;
	DataPortalQuery q;
	q.text = m_search->text().trimmed();
	q.manufacturer = m_manufacturer->text().trimmed();
	q.exact = m_exact->isChecked();
	q.limit = 100;
	m_list->clear();
	m_parts.clear();
	m_take->setEnabled(false);
	m_status->setText(tr("Searching…"));
	m_client->search(q);
}

void ArticleDialog::showParts(const QVector<DataPortalPart> &parts, int total, int offset)
{
	Q_UNUSED(offset)
		// configuration entries without an order number are no part to buy
	for (const DataPortalPart &p : parts) {
		if (p.order_number.isEmpty()) continue;
		auto *item = new QTreeWidgetItem(m_list, {p.manufacturer, p.order_number, p.type_number,
												  p.designation, p.product_group});
		item->setData(0, Qt::UserRole, int(m_parts.size()));
		item->setToolTip(3, p.description.isEmpty() ? p.designation : p.description);
		m_parts << p;
	}
	for (int c = 0; c < 3; ++c) m_list->resizeColumnToContents(c);
	m_status->setText(m_parts.isEmpty()
						  ? tr("Nothing found. Switch off \"Exact\" or search by type.")
						  : tr("%1 of %2 parts.").arg(m_parts.size()).arg(total));
}

void ArticleDialog::take()
{
	const QList<QTreeWidgetItem *> sel = m_list->selectedItems();
	if (sel.isEmpty()) return;
	m_part = m_parts.value(sel.first()->data(0, Qt::UserRole).toInt());
	accept();
}
