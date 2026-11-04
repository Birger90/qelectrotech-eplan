/*
	Copyright 2006-2026 The QElectroTech Team
	This file is part of QElectroTech.

	QElectroTech is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	(at your option) any later version.

	QElectroTech is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with QElectroTech. If not, see <http://www.gnu.org/licenses/>.
*/
#include "dataportaldialog.h"

#include "../../qetapp.h"
#include "../../qetmessagebox.h"
#include "eplanpartimporter.h"

#include <QCheckBox>
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFormLayout>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QPushButton>
#include <QSettings>
#include <QTemporaryDir>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace {
	const char TOKEN_KEY[] = "eplanimport/dataportal_token";
	const char MANUFACTURER_KEY[] = "eplanimport/dataportal_manufacturer";
	const int ROLE_INDEX = Qt::UserRole + 1;
}

DataPortalDialog::DataPortalDialog(QWidget *parent) :
	QDialog(parent),
	m_client(new DataPortalClient(this))
{
	setWindowTitle(tr("EPLAN Data Portal"));
	setMinimumSize(760, 560);
	m_client->setLanguage(QLocale(QETApp::langFromSetting()).name());

	QSettings settings;
	const QString stored = settings.value(QLatin1String(TOKEN_KEY)).toString();

	// the token is a password: never shown in clear
	m_token = new QLineEdit(stored, this);
	m_token->setEchoMode(QLineEdit::Password);
	m_token->setPlaceholderText(tr("Personal Access Token, only needed for the download"));
	m_token->setClearButtonEnabled(true);
	m_remember = new QCheckBox(tr("Remember the token"), this);
	m_remember->setChecked(true);
	m_remember->setToolTip(tr("Keeps the token in the settings of QElectroTech on this computer."));
	auto *token_row = new QHBoxLayout;
	token_row->addWidget(m_token, 1);
	token_row->addWidget(m_remember);
	auto *token_hint = new QLabel(
		tr("Create the token in your ePulse profile, as a Personal Access Token for "
		   "the Data Portal. Searching works without it."),
		this);
	token_hint->setWordWrap(true);

	m_search = new QLineEdit(this);
	m_search->setPlaceholderText(tr("Order number, type number or text"));
	m_manufacturer = new QLineEdit(settings.value(QLatin1String(MANUFACTURER_KEY)).toString(), this);
	m_manufacturer->setPlaceholderText(tr("all, or short name e.g. SIE"));
	m_manufacturer->setMaximumWidth(180);
	m_search_button = new QPushButton(tr("Search"), this);
	connect(m_search, &QLineEdit::returnPressed, this, &DataPortalDialog::runSearch);
	connect(m_manufacturer, &QLineEdit::returnPressed, this, &DataPortalDialog::runSearch);
	connect(m_search_button, &QPushButton::clicked, this, &DataPortalDialog::runSearch);
	auto *search_row = new QHBoxLayout;
	search_row->addWidget(m_search, 1);
	search_row->addWidget(new QLabel(tr("Manufacturer:"), this));
	search_row->addWidget(m_manufacturer);
	search_row->addWidget(m_search_button);

	m_results = new QTreeWidget(this);
	m_results->setRootIsDecorated(false);
	m_results->setUniformRowHeights(true);
	m_results->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_results->setHeaderLabels({tr("Order number"), tr("Type number"), tr("Manufacturer"),
				    tr("Designation"), tr("Product group"), tr("DXF")});
	m_results->header()->setStretchLastSection(false);
	m_results->header()->setSectionResizeMode(3, QHeaderView::Stretch);
	connect(m_results, &QTreeWidget::itemSelectionChanged, this, &DataPortalDialog::updateButtons);
	connect(m_results, &QTreeWidget::itemDoubleClicked, this, [this]() {
		if (m_import->isEnabled()) runImport();
	});
	m_count = new QLabel(this);

	m_category = new QLineEdit(this);
	m_category->setPlaceholderText(tr("product group of the part"));
	auto *target = new QLabel(QDir::toNativeSeparators(QETApp::customElementsDir()), this);
	target->setTextInteractionFlags(Qt::TextSelectableByMouse);

	auto *form = new QFormLayout;
	form->addRow(tr("Token:"), token_row);
	form->addRow(QString(), token_hint);
	form->addRow(tr("Search:"), search_row);

	auto *import_form = new QFormLayout;
	import_form->addRow(QCoreApplication::translate("EplanImportDialog", "Category:"), m_category);
	import_form->addRow(QCoreApplication::translate("EplanImportDialog", "Import into:"), target);

	// same terms as the import from a file, in the words of that dialog
	auto *terms_label = new QLabel(QCoreApplication::translate("EplanImportDialog",
		"Manufacturer data from the EPLAN Data Portal or a manufacturer's "
		"website is subject to the terms of use of that source. By importing "
		"you confirm that you are authorised to use it here; the import is "
		"at your own responsibility."),
		this);
	terms_label->setWordWrap(true);
	m_terms = new QCheckBox(QCoreApplication::translate("EplanImportDialog", "I accept these terms."), this);
	connect(m_terms, &QCheckBox::toggled, this, &DataPortalDialog::updateButtons);

	m_status = new QLabel(this);
	m_status->setWordWrap(true);
	m_status->setTextInteractionFlags(Qt::TextSelectableByMouse);

	auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
	m_import = buttons->addButton(QCoreApplication::translate("EplanImportDialog", "Import"),
				      QDialogButtonBox::ActionRole);
	connect(m_import, &QPushButton::clicked, this, &DataPortalDialog::runImport);
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

	auto *layout = new QVBoxLayout(this);
	layout->addLayout(form);
	layout->addWidget(m_results, 1);
	layout->addWidget(m_count);
	layout->addLayout(import_form);
	layout->addSpacing(8);
	layout->addWidget(terms_label);
	layout->addWidget(m_terms);
	layout->addWidget(m_status);
	layout->addWidget(buttons);

	connect(m_client, &DataPortalClient::searchFinished, this, &DataPortalDialog::showParts);
	connect(m_client, &DataPortalClient::downloadFinished, this, &DataPortalDialog::importPackage);
	connect(m_client, &DataPortalClient::failed, this, &DataPortalDialog::failed);

	m_search->setFocus();
	updateButtons();
}

DataPortalDialog::~DataPortalDialog()
{
	storeToken();
}

/// keep the token (or forget it) as "Remember the token" says
void DataPortalDialog::storeToken()
{
	QSettings settings;
	const QString token = m_token->text().trimmed();
	if (m_remember->isChecked() && !token.isEmpty())
		settings.setValue(QLatin1String(TOKEN_KEY), token);
	else
		settings.remove(QLatin1String(TOKEN_KEY));
	settings.setValue(QLatin1String(MANUFACTURER_KEY), m_manufacturer->text().trimmed());
}

void DataPortalDialog::runSearch()
{
	if (m_busy) return;
	const QString text = m_search->text().trimmed();
	if (text.isEmpty() && m_manufacturer->text().trimmed().isEmpty()) {
		m_status->setText(tr("Enter an order number, a type number or a manufacturer."));
		return;
	}
	m_client->setToken(m_token->text());
	setBusy(true, tr("Searching…"));
	m_client->search(text, m_manufacturer->text());
}

void DataPortalDialog::showParts(const QVector<DataPortalPart> &parts, int total)
{
	setBusy(false);
	m_parts = parts;
	m_results->clear();
	for (int i = 0; i < parts.size(); ++i) {
		const DataPortalPart &p = parts.at(i);
		auto *item = new QTreeWidgetItem(m_results, {p.order_number, p.type_number, p.manufacturer,
							     p.designation, p.product_group,
							     p.has_dxf ? tr("yes") : tr("no")});
		item->setData(0, ROLE_INDEX, i);
		item->setToolTip(0, p.part_number);
		item->setToolTip(3, p.designation);
		if (!p.has_dxf) {
			for (int c = 0; c < m_results->columnCount(); ++c)
				item->setForeground(c, palette().brush(QPalette::Disabled, QPalette::Text));
		}
	}
	for (int c = 0; c < m_results->columnCount(); ++c)
		if (c != 3) m_results->resizeColumnToContents(c);
	if (parts.isEmpty())
		m_count->setText(tr("No part found."));
	else if (total > parts.size())
		m_count->setText(tr("%1 of %2 parts shown. Narrow the search to see the others.")
				 .arg(parts.size()).arg(total));
	else
		m_count->setText(tr("%n part(s) found.", "", int(parts.size())));
	if (m_results->topLevelItemCount() == 1) m_results->topLevelItem(0)->setSelected(true);
	m_status->clear();
	updateButtons();
}

void DataPortalDialog::runImport()
{
	if (m_busy) return;
	m_queue.clear();
	QStringList without_dxf;
	for (QTreeWidgetItem *item : m_results->selectedItems()) {
		const DataPortalPart &p = m_parts.at(item->data(0, ROLE_INDEX).toInt());
		if (p.has_dxf) m_queue.append(p);
		else without_dxf << p.order_number;
	}
	if (m_queue.isEmpty()) return;
	if (m_token->text().trimmed().isEmpty()) {
		m_status->setText(tr("Enter your Personal Access Token to download parts."));
		m_token->setFocus();
		m_queue.clear();
		return;
	}
	m_report.clear();
	if (!without_dxf.isEmpty())
		m_report << tr("No DXF data, skipped: %1").arg(without_dxf.join(QStringLiteral(", ")));
	m_client->setToken(m_token->text());
	storeToken();
	importNext();
}

void DataPortalDialog::importNext()
{
	if (m_queue.isEmpty()) {
		finishImport();
		return;
	}
	m_current = m_queue.takeFirst();
	setBusy(true, tr("Downloading %1…").arg(m_current.order_number));
	m_client->downloadDxf(m_current.id);
}

/// hand the downloaded package to the import of a package from a file
void DataPortalDialog::importPackage(const QByteArray &data, const QString &file_name)
{
	setBusy(true, tr("Importing %1…").arg(m_current.order_number));
	QTemporaryDir dir;
	const QString path = dir.filePath(file_name);
	QFile file(path);
	if (!dir.isValid() || !file.open(QIODevice::WriteOnly) || file.write(data) != data.size()) {
		failed(tr("The package cannot be stored in a temporary folder."));
		return;
	}
	file.close();

	EplanPartImporter importer;
	QGuiApplication::setOverrideCursor(Qt::WaitCursor);
	const bool ok = importer.importPackage(path, m_category->text().trimmed(),
					       QETApp::customElementsDir());
	QGuiApplication::restoreOverrideCursor();
	if (ok) {
		m_written << importer.writtenPaths();
		m_report << tr("%1: %n element(s) in %2", "", int(importer.writtenPaths().size()))
				    .arg(m_current.order_number,
					 QDir::toNativeSeparators(QFileInfo(importer.writtenPaths().first()).absolutePath()));
	} else {
		m_report << tr("%1: the import failed: %2").arg(m_current.order_number, importer.errorString());
	}
	importNext();
}

void DataPortalDialog::failed(const QString &message)
{
	const bool importing = !m_current.id.isEmpty();
	setBusy(false);
	m_status->setText(message);
	if (!importing) return;
	// a wrong token or a reached limit holds for the parts still waiting too
	m_queue.clear();
	m_report << QStringLiteral("%1: %2").arg(m_current.order_number, message);
	finishImport();
}

void DataPortalDialog::finishImport()
{
	setBusy(false);
	m_current = DataPortalPart();
	if (m_report.isEmpty()) return;
	QET::QetMessageBox::information(this, tr("EPLAN Data Portal"), m_report.join(QLatin1Char('\n')));
	m_report.clear();
}

void DataPortalDialog::setBusy(bool busy, const QString &text)
{
	m_busy = busy;
	if (busy) m_status->setText(text);
	m_search_button->setEnabled(!busy);
	updateButtons();
}

void DataPortalDialog::updateButtons()
{
	bool any_dxf = false;
	for (QTreeWidgetItem *item : m_results->selectedItems())
		if (m_parts.value(item->data(0, ROLE_INDEX).toInt()).has_dxf) any_dxf = true;
	m_import->setEnabled(!m_busy && any_dxf && m_terms->isChecked());
}
