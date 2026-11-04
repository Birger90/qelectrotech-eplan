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
#include <QComboBox>
#include <QCompleter>
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
#include <QRegularExpression>
#include <QSettings>
#include <QTemporaryDir>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace {
	const char TOKEN_KEY[] = "eplanimport/dataportal_token";
	const char MANUFACTURER_KEY[] = "eplanimport/dataportal_manufacturer";
	const char ONLY_DXF_KEY[] = "eplanimport/dataportal_only_dxf";
	const char EXACT_KEY[] = "eplanimport/dataportal_exact";
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
	m_search_button = new QPushButton(tr("Search"), this);
	connect(m_search, &QLineEdit::returnPressed, this, &DataPortalDialog::runSearch);
	connect(m_search_button, &QPushButton::clicked, this, &DataPortalDialog::runSearch);
	auto *search_row = new QHBoxLayout;
	search_row->addWidget(m_search, 1);
	search_row->addWidget(m_search_button);

	// the manufacturer list comes from the Data Portal; until then, or
	// without it, a short name typed in is taken as it is
	m_manufacturer = new QComboBox(this);
	m_manufacturer->setEditable(true);
	m_manufacturer->setInsertPolicy(QComboBox::NoInsert);
	m_manufacturer->setMinimumContentsLength(28);
	m_manufacturer->lineEdit()->setPlaceholderText(tr("All manufacturers"));
	m_manufacturer->setEditText(settings.value(QLatin1String(MANUFACTURER_KEY)).toString());
	connect(m_manufacturer->lineEdit(), &QLineEdit::returnPressed, this, &DataPortalDialog::runSearch);
	m_only_dxf = new QCheckBox(tr("Only parts with DXF"), this);
	m_only_dxf->setToolTip(tr("Only parts with DXF data can be imported as elements."));
	m_only_dxf->setChecked(settings.value(QLatin1String(ONLY_DXF_KEY), true).toBool());
	m_exact = new QCheckBox(tr("Exact match"), this);
	m_exact->setChecked(settings.value(QLatin1String(EXACT_KEY), false).toBool());
	auto *filter_row = new QHBoxLayout;
	filter_row->addWidget(m_manufacturer, 1);
	filter_row->addWidget(m_only_dxf);
	filter_row->addWidget(m_exact);

	m_results = new QTreeWidget(this);
	m_results->setRootIsDecorated(false);
	m_results->setUniformRowHeights(true);
	m_results->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_results->setHeaderLabels({tr("Order number"), tr("Type number"), tr("Manufacturer"),
				    tr("Designation"), tr("Product group"), tr("DXF")});
	m_results->header()->setStretchLastSection(false);
	m_results->header()->setSectionResizeMode(3, QHeaderView::Stretch);
	connect(m_results, &QTreeWidget::itemSelectionChanged, this, &DataPortalDialog::updateButtons);
	connect(m_results, &QTreeWidget::itemSelectionChanged, this, &DataPortalDialog::showPicture);
	connect(m_results, &QTreeWidget::itemDoubleClicked, this, [this]() {
		if (m_import->isEnabled()) runImport();
	});
	m_picture = new QLabel(this);
	m_picture->setFixedSize(220, 220);
	m_picture->setAlignment(Qt::AlignCenter);
	m_picture->setFrameShape(QFrame::StyledPanel);
	auto *results_row = new QHBoxLayout;
	results_row->addWidget(m_results, 1);
	results_row->addWidget(m_picture, 0, Qt::AlignTop);

	m_count = new QLabel(this);
	m_more = new QPushButton(tr("Load more"), this);
	m_more->setVisible(false);
	connect(m_more, &QPushButton::clicked, this, &DataPortalDialog::loadMore);
	auto *count_row = new QHBoxLayout;
	count_row->addWidget(m_count, 1);
	count_row->addWidget(m_more);

	m_category = new QLineEdit(this);
	m_category->setPlaceholderText(tr("product group of the part"));
	auto *target = new QLabel(QDir::toNativeSeparators(QETApp::customElementsDir()), this);
	target->setTextInteractionFlags(Qt::TextSelectableByMouse);

	auto *form = new QFormLayout;
	form->addRow(tr("Token:"), token_row);
	form->addRow(QString(), token_hint);
	form->addRow(tr("Search:"), search_row);
	form->addRow(tr("Manufacturer:"), filter_row);

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
	layout->addLayout(results_row, 1);
	layout->addLayout(count_row);
	layout->addLayout(import_form);
	layout->addSpacing(8);
	layout->addWidget(terms_label);
	layout->addWidget(m_terms);
	layout->addWidget(m_status);
	layout->addWidget(buttons);

	connect(m_client, &DataPortalClient::searchFinished, this, &DataPortalDialog::showParts);
	connect(m_client, &DataPortalClient::downloadFinished, this, &DataPortalDialog::importPackage);
	connect(m_client, &DataPortalClient::failed, this, &DataPortalDialog::failed);
	connect(m_client, &DataPortalClient::manufacturersLoaded, this, &DataPortalDialog::fillManufacturers);
	connect(m_client, &DataPortalClient::pictureLoaded, this, &DataPortalDialog::pictureLoaded);
	m_client->loadManufacturers();

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
	settings.setValue(QLatin1String(MANUFACTURER_KEY), manufacturerFilter());
	settings.setValue(QLatin1String(ONLY_DXF_KEY), m_only_dxf->isChecked());
	settings.setValue(QLatin1String(EXACT_KEY), m_exact->isChecked());
}

/// "Long name (SHORT)" in the list, the short name as its data
void DataPortalDialog::fillManufacturers(const QVector<DataPortalManufacturer> &list)
{
	const QString current = manufacturerFilter();
	m_manufacturer->clear();
	m_manufacturer->addItem(QString(), QString());
	for (const DataPortalManufacturer &m : list) {
		const QString text = m.long_name.compare(m.short_name, Qt::CaseInsensitive) == 0
				     ? m.long_name
				     : QStringLiteral("%1 (%2)").arg(m.long_name, m.short_name);
		m_manufacturer->addItem(text, m.short_name);
	}
	auto *completer = new QCompleter(m_manufacturer->model(), m_manufacturer);
	completer->setCaseSensitivity(Qt::CaseInsensitive);
	completer->setFilterMode(Qt::MatchContains);
	completer->setCompletionMode(QCompleter::PopupCompletion);
	m_manufacturer->setCompleter(completer);
	const int index = m_manufacturer->findData(current);
	if (index > 0) m_manufacturer->setCurrentIndex(index);
	else m_manufacturer->setEditText(current);
}

/// the short name to filter by: of the chosen list entry, else as typed
QString DataPortalDialog::manufacturerFilter() const
{
	const QString text = m_manufacturer->currentText().trimmed();
	if (text.isEmpty()) return QString();
	const int index = m_manufacturer->findText(text, Qt::MatchFixedString);
	if (index > 0) return m_manufacturer->itemData(index).toString();
	// "Long name (SHORT)" typed or pasted without being in the list
	static const QRegularExpression short_name(QStringLiteral("\\(([^()]+)\\)$"));
	const auto m = short_name.match(text);
	return m.hasMatch() ? m.captured(1) : text;
}

void DataPortalDialog::runSearch()
{
	// a new search replaces a running one; only an import is waited for
	if (!m_current.id.isEmpty()) return;
	DataPortalQuery query;
	query.text = m_search->text().trimmed();
	query.manufacturer = manufacturerFilter();
	query.only_dxf = m_only_dxf->isChecked();
	query.exact = m_exact->isChecked();
	if (query.text.isEmpty() && query.manufacturer.isEmpty()) {
		m_status->setText(tr("Enter an order number, a type number or a manufacturer."));
		return;
	}
	m_query = query;
	m_client->setToken(m_token->text());
	setBusy(true, tr("Searching…"));
	m_client->search(query);
}

/// the next page of the last search, added below the shown parts
void DataPortalDialog::loadMore()
{
	if (m_busy) return;
	m_query.offset = int(m_parts.size());
	setBusy(true, tr("Searching…"));
	m_client->search(m_query);
}

void DataPortalDialog::showParts(const QVector<DataPortalPart> &parts, int total, int offset)
{
	setBusy(false);
	if (offset == 0) {
		m_parts.clear();
		m_results->clear();
		m_picture->clear();
	}
	m_total = total;
	for (const DataPortalPart &p : parts) {
		auto *item = new QTreeWidgetItem(m_results, {p.order_number, p.type_number, p.manufacturer,
							     p.designation, p.product_group,
							     p.has_dxf ? tr("yes") : p.has_macro ? tr("maybe") : tr("no")});
		item->setData(0, ROLE_INDEX, int(m_parts.size()));
		m_parts.append(p);
		item->setToolTip(0, p.part_number);
		item->setToolTip(3, p.designation);
		if (!p.has_dxf && p.has_macro) {
			item->setToolTip(5, tr("The Data Portal marks no DXF data for this part, but it has "
					       "schematic macros: the import tries the DXF download."));
		} else if (!p.has_dxf) {
			const QString why = tr("The Data Portal has no DXF data for this part: it cannot be imported.");
			for (int c = 0; c < m_results->columnCount(); ++c) {
				item->setForeground(c, palette().brush(QPalette::Disabled, QPalette::Text));
				item->setToolTip(c, why);
			}
		}
	}
	if (offset == 0) {
		for (int c = 0; c < m_results->columnCount(); ++c)
			if (c != 3) m_results->resizeColumnToContents(c);
		if (m_results->topLevelItemCount() > 0) m_results->topLevelItem(0)->setSelected(true);
	}
	updateCount();
	m_status->clear();
	updateButtons();
}

void DataPortalDialog::updateCount()
{
	const int shown = int(m_parts.size());
	if (shown == 0)
		m_count->setText(m_only_dxf->isChecked()
				 ? tr("No part with DXF data found.")
				 : tr("No part found."));
	else if (m_total > shown)
		m_count->setText(tr("%1 of %2 parts shown.").arg(shown).arg(m_total));
	else
		m_count->setText(tr("%n part(s) found.", "", shown));
	m_more->setVisible(m_total > shown);
}

/// the picture of the selected part, loaded once
void DataPortalDialog::showPicture()
{
	const QList<QTreeWidgetItem *> selected = m_results->selectedItems();
	if (selected.isEmpty()) {
		m_picture->clear();
		return;
	}
	const QString url = m_parts.value(selected.first()->data(0, ROLE_INDEX).toInt()).picture_url;
	if (url.isEmpty()) {
		m_picture->setText(tr("No picture"));
		return;
	}
	if (m_pictures.contains(url)) {
		m_picture->setPixmap(m_pictures.value(url));
		return;
	}
	m_picture->setText(tr("Loading…"));
	m_client->loadPicture(url);
}

void DataPortalDialog::pictureLoaded(const QString &url, const QByteArray &data)
{
	QPixmap pixmap;
	if (!pixmap.loadFromData(data)) return;
	const qreal ratio = devicePixelRatioF();
	pixmap = pixmap.scaled(m_picture->contentsRect().size() * ratio,
			       Qt::KeepAspectRatio, Qt::SmoothTransformation);
	pixmap.setDevicePixelRatio(ratio);
	m_pictures.insert(url, pixmap);
	showPicture();
}

void DataPortalDialog::runImport()
{
	if (m_busy) return;
	m_queue.clear();
	QStringList without_dxf;
	for (QTreeWidgetItem *item : m_results->selectedItems()) {
		const DataPortalPart &p = m_parts.at(item->data(0, ROLE_INDEX).toInt());
		if (p.importable()) m_queue.append(p);
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
	m_search_button->setEnabled(m_current.id.isEmpty());
	m_more->setEnabled(!busy);
	updateButtons();
}

void DataPortalDialog::updateButtons()
{
	bool any_dxf = false;
	for (QTreeWidgetItem *item : m_results->selectedItems())
		if (m_parts.value(item->data(0, ROLE_INDEX).toInt()).importable()) any_dxf = true;
	m_import->setEnabled(!m_busy && any_dxf && m_terms->isChecked());
}
