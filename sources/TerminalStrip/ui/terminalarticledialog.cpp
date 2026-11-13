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
#include "terminalarticledialog.h"

#include "terminalpreviewwidget.h"

#include <QComboBox>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QPushButton>
#include <QSet>
#include <QSplitter>
#include <QPixmap>
#include <QTableWidget>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>
#include <algorithm>

using namespace TerminalArticleCatalog;

namespace
{
	enum Column { ColManufacturer = 0, ColOrder, ColDesignation, ColKind, ColSection,
				  ColWidth, ColClamp, ColConductors, ColSource, ColCount };

	QString mm(double value)
	{
		return QLocale().toString(value, 'g', 3);
	}
}

Article TerminalArticleDialog::choose(QWidget *parent, const QString &current_article,
									  bool protective_earth)
{
	TerminalArticleDialog dialog_(parent);
	dialog_.preselect(current_article, protective_earth);
	if (dialog_.exec() != QDialog::Accepted) {
		return Article();
	}
	const Article article_ = dialog_.selectedArticle();
		//A terminal found online is kept, so it is there next time offline
	if (article_.source == QLatin1String("online")) {
		TerminalArticleCatalog::remember(article_);
	}
	return article_;
}

TerminalArticleDialog::TerminalArticleDialog(QWidget *parent) :
	QDialog(parent)
{
	setWindowTitle(tr("Terminal catalogue"));

	m_search = new QLineEdit(this);
	m_search->setPlaceholderText(tr("Search: order number, series, designation…"));
	m_search->setClearButtonEnabled(true);
	m_manufacturer = new QComboBox(this);
	m_kind = new QComboBox(this);
	m_section = new QComboBox(this);
	m_clamp = new QComboBox(this);

	m_online = new QPushButton(tr("Search the Data Portal"), this);
	m_online->setToolTip(tr("Look for terminals in the EPLAN Data Portal (online); "
							"a terminal used from there is kept in the catalogue"));

	auto *filters_ = new QHBoxLayout;
	filters_->addWidget(m_search, 2);
	filters_->addWidget(m_online);
	filters_->addWidget(new QLabel(tr("Manufacturer:"), this));
	filters_->addWidget(m_manufacturer, 1);
	filters_->addWidget(new QLabel(tr("Kind:"), this));
	filters_->addWidget(m_kind, 1);
	filters_->addWidget(new QLabel(tr("mm²:"), this));
	filters_->addWidget(m_section);
	filters_->addWidget(new QLabel(tr("Clamp:"), this));
	filters_->addWidget(m_clamp, 1);

	m_table = new QTableWidget(0, ColCount, this);
	m_table->setHorizontalHeaderLabels({tr("Manufacturer"), tr("Order number"), tr("Designation"),
										tr("Kind"), tr("mm²"), tr("Width mm"), tr("Clamp"),
										tr("Conductors"), tr("Source")});
	m_table->verticalHeader()->setVisible(false);
	m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_table->setSelectionMode(QAbstractItemView::SingleSelection);
	m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
		//Fixed widths the user can drag, the designation takes the rest:
		//sizing to the contents of twelve thousand rows is slow and lets
		//the long texts squeeze the others
	auto *header_ = m_table->horizontalHeader();
	header_->setSectionResizeMode(QHeaderView::Interactive);
	header_->setStretchLastSection(false);
	const QVector<QPair<int, int>> widths_ {
		{ColManufacturer, 125}, {ColOrder, 125}, {ColKind, 140}, {ColSection, 55},
		{ColWidth, 80}, {ColClamp, 95}, {ColConductors, 55}, {ColSource, 130}};
	for (const auto &w_ : widths_) {
		m_table->setColumnWidth(w_.first, w_.second);
	}
	header_->setSectionResizeMode(ColDesignation, QHeaderView::Stretch);
	header_->setMinimumSectionSize(40);
	m_table->setAlternatingRowColors(true);
	m_table->setWordWrap(false);
	m_table->setTextElideMode(Qt::ElideRight);

	m_preview = new TerminalPreviewWidget(this);
	m_details = new QLabel(this);
	m_details->setWordWrap(true);
	m_details->setTextInteractionFlags(Qt::TextSelectableByMouse);
	m_details->setMinimumWidth(260);

	auto *side_ = new QWidget(this);
	auto *side_layout_ = new QVBoxLayout(side_);
	side_layout_->setContentsMargins(0, 0, 0, 0);
	side_layout_->addWidget(new QLabel(tr("Preview (from above, at scale):"), side_));
	side_layout_->addWidget(m_preview, 1);
	m_picture = new QLabel(side_);
	m_picture->setAlignment(Qt::AlignCenter);
	m_picture->setMinimumHeight(140);
	m_picture->setStyleSheet(QStringLiteral("background: white; border: 1px solid #c8c8c8;"));
	side_layout_->addWidget(m_picture, 1);
	side_layout_->addWidget(m_details);

	auto *splitter_ = new QSplitter(this);
	splitter_->addWidget(m_table);
	splitter_->addWidget(side_);
	splitter_->setStretchFactor(0, 3);
	splitter_->setStretchFactor(1, 1);
	splitter_->setSizes({960, 380});

	m_count = new QLabel(this);
	auto *folder_ = new QPushButton(tr("Catalogue folder…"), this);
	folder_->setToolTip(tr("Own terminals: CSV files in this folder of the data directory are read as well"));
	auto *reload_ = new QPushButton(tr("Reload"), this);

	auto *buttons_ = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	buttons_->button(QDialogButtonBox::Ok)->setText(tr("Use"));
	buttons_->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));

	auto *bottom_ = new QHBoxLayout;
	bottom_->addWidget(m_count, 1);
	bottom_->addWidget(folder_);
	bottom_->addWidget(reload_);
	bottom_->addWidget(buttons_);

	auto *layout_ = new QVBoxLayout(this);
	layout_->addLayout(filters_);
	layout_->addWidget(splitter_, 1);
	layout_->addLayout(bottom_);

	connect(buttons_, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttons_, &QDialogButtonBox::rejected, this, &QDialog::reject);
	connect(m_table, &QTableWidget::itemDoubleClicked, this, &QDialog::accept);
	connect(m_table, &QTableWidget::itemSelectionChanged, this, &TerminalArticleDialog::selectionChanged);
	connect(m_search, &QLineEdit::textChanged, this, &TerminalArticleDialog::applyFilter);
	for (auto *combo_ : {m_manufacturer, m_kind, m_section, m_clamp}) {
		connect(combo_, qOverload<int>(&QComboBox::currentIndexChanged), this, &TerminalArticleDialog::applyFilter);
	}
	connect(folder_, &QPushButton::clicked, this, &TerminalArticleDialog::openCatalogFolder);
	connect(reload_, &QPushButton::clicked, this, [this]() {
		TerminalArticleCatalog::reload();
		const auto current_ = selectedArticle().articleText();
		m_articles = TerminalArticleCatalog::articles();
		fillFilters();
		fillTable();
		preselect(current_, false);
	});

	const QString language_ = QLocale::system().name();
	m_client = new DataPortalClient(this);
	m_client->setLanguage(language_);
	m_picture_client = new DataPortalClient(this);
	m_picture_client->setLanguage(language_);
	m_picture_timer = new QTimer(this);
	m_picture_timer->setSingleShot(true);
	m_picture_timer->setInterval(300);
	connect(m_picture_timer, &QTimer::timeout, this, &TerminalArticleDialog::lookUpPicture);
	connect(m_online, &QPushButton::clicked, this, &TerminalArticleDialog::searchOnline);
		//Enter in the search field: online when nothing local matches
	connect(m_search, &QLineEdit::returnPressed, this, [this]() {
		if (m_count->property("shown").toInt() == 0) {
			searchOnline();
		}
	});
	connect(m_client, &DataPortalClient::searchFinished, this,
			[this](const QVector<DataPortalPart> &parts, int, int) { onlineResults(parts); });
	connect(m_client, &DataPortalClient::failed, this, [this](const QString &message) {
		m_online->setEnabled(true);
		m_count->setText(tr("Data Portal: %1").arg(message));
	});
	connect(m_picture_client, &DataPortalClient::searchFinished, this,
			[this](const QVector<DataPortalPart> &parts, int, int) {
		const Article current_ = selectedArticle();
		for (const auto &p_ : parts)
		{
			if (p_.order_number != current_.order_number || p_.picture_url.isEmpty()) {
				continue;
			}
			for (auto &a_ : m_articles) {
				if (a_.order_number == p_.order_number && a_.manufacturer == current_.manufacturer) {
					a_.picture_url = p_.picture_url;
				}
			}
			m_picture_url = p_.picture_url;
			m_picture_client->loadPicture(p_.picture_url);
			return;
		}
		m_picture->setText(tr("No picture"));
	});
	connect(m_picture_client, &DataPortalClient::pictureLoaded, this, &TerminalArticleDialog::showPicture);
	connect(m_picture_client, &DataPortalClient::failed, this, [this](const QString &) {
		m_picture->setText(tr("No picture (offline?)"));
	});

	m_articles = TerminalArticleCatalog::articles();
	fillFilters();
	fillTable();
	resize(1360, 660);
}

void TerminalArticleDialog::searchOnline()
{
	const QString text_ = m_search->text().trimmed();
	if (text_.isEmpty()) {
		m_count->setText(tr("Type an order number or type to search the Data Portal"));
		return;
	}
	DataPortalQuery query_;
	query_.text = text_;
	query_.manufacturer = TerminalArticleCatalog::dataPortalShortName(m_manufacturer->currentData().toString());
	query_.catalog_eplan = TerminalArticleCatalog::dataPortalTerminalGroup();
	query_.limit = 50;
	m_online->setEnabled(false);
	m_count->setText(tr("Searching the Data Portal…"));
	m_client->search(query_);
}

void TerminalArticleDialog::onlineResults(const QVector<DataPortalPart> &parts)
{
	m_online->setEnabled(true);
	int added_ = 0;
	for (const auto &p_ : parts)
	{
		auto article_ = TerminalArticleCatalog::fromDataPortal(p_.manufacturer, p_.order_number,
															   p_.type_number, p_.designation,
															   p_.description, p_.id);
		article_.picture_url = p_.picture_url;
		const bool known_ = std::any_of(m_articles.begin(), m_articles.end(), [&](const Article &a) {
			return a.order_number == article_.order_number
				&& a.manufacturer.compare(article_.manufacturer, Qt::CaseInsensitive) == 0;
		});
		if (!known_) {
			m_articles << article_;
			++added_;
		}
	}
	fillFilters();
	fillTable();
	if (added_) {
			//Show what was found: the first row from the portal
		for (int row_ = 0; row_ < m_table->rowCount(); ++row_)
		{
			auto *first_ = m_table->item(row_, ColManufacturer);
			if (first_ && m_articles.value(first_->data(Qt::UserRole).toInt()).source == QLatin1String("online")
				&& !m_table->isRowHidden(row_)) {
				m_table->selectRow(row_);
				m_table->scrollToItem(first_);
				break;
			}
		}
	}
	m_count->setText(m_count->text() + QStringLiteral(" · ")
					 + tr("%n new from the Data Portal", nullptr, added_));
}

void TerminalArticleDialog::lookUpPicture()
{
	const Article a_ = selectedArticle();
	if (!a_.isValid()) {
		return;
	}
	if (!a_.picture_url.isEmpty())
	{
		m_picture_url = a_.picture_url;
		m_picture_client->loadPicture(a_.picture_url);
		return;
	}
	DataPortalQuery query_;
	query_.text = a_.order_number;
	query_.manufacturer = TerminalArticleCatalog::dataPortalShortName(a_.manufacturer);
	query_.exact = true;
	query_.limit = 5;
	m_picture_client->search(query_);
}

void TerminalArticleDialog::showPicture(const QString &url, const QByteArray &data)
{
	if (url != m_picture_url) {
		return;		//an older picture, the selection moved on
	}
	QPixmap pixmap_;
	if (!pixmap_.loadFromData(data)) {
		m_picture->setText(tr("No picture"));
		return;
	}
	m_picture->setPixmap(pixmap_.scaled(m_picture->size() - QSize(8, 8),
										Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void TerminalArticleDialog::fillFilters()
{
	const auto refill = [](QComboBox *combo, const QList<QPair<QString, QVariant>> &items) {
		const QSignalBlocker blocker_(combo);
		const QVariant keep_ = combo->currentData();
		combo->clear();
		combo->addItem(TerminalArticleDialog::tr("All"), QVariant());
		for (const auto &item_ : items) {
			combo->addItem(item_.first, item_.second);
		}
		const int at_ = keep_.isValid() ? combo->findData(keep_) : 0;
		combo->setCurrentIndex(at_ >= 0 ? at_ : 0);
	};

	QStringList makers_;
	QList<double> sections_;
	for (const auto &a_ : m_articles) {
		if (!makers_.contains(a_.manufacturer)) makers_ << a_.manufacturer;
		if (a_.section > 0 && !sections_.contains(a_.section)) sections_ << a_.section;
	}
	std::sort(sections_.begin(), sections_.end());

	QList<QPair<QString, QVariant>> maker_items_;
	for (const auto &m_ : makers_) maker_items_ << qMakePair(m_, QVariant(m_));
	refill(m_manufacturer, maker_items_);

	QList<QPair<QString, QVariant>> kind_items_;
	for (auto k_ : {Kind::FeedThrough, Kind::ProtectiveEarth, Kind::DoubleDeck, Kind::Fuse, Kind::Disconnect}) {
		kind_items_ << qMakePair(kindName(k_), QVariant(int(k_)));
	}
	refill(m_kind, kind_items_);

	QList<QPair<QString, QVariant>> section_items_;
	for (double s_ : sections_) section_items_ << qMakePair(mm(s_), QVariant(s_));
	refill(m_section, section_items_);

	QList<QPair<QString, QVariant>> clamp_items_;
	for (auto c_ : {Clamp::PushIn, Clamp::Spring, Clamp::Screw}) {
		clamp_items_ << qMakePair(clampName(c_), QVariant(int(c_)));
	}
	refill(m_clamp, clamp_items_);
}

void TerminalArticleDialog::fillTable()
{
	m_table->setSortingEnabled(false);
	m_table->clearContents();
	m_table->setRowCount(int(m_articles.size()));
	for (int i = 0; i < m_articles.size(); ++i)
	{
		const auto &a_ = m_articles.at(i);
		auto set_ = [&](int col, const QVariant &value) {
			auto *item_ = new QTableWidgetItem;
			item_->setData(Qt::DisplayRole, value);
			m_table->setItem(i, col, item_);
			return item_;
		};
		set_(ColManufacturer, a_.manufacturer)->setData(Qt::UserRole, i);
		set_(ColOrder, a_.order_number);
		set_(ColDesignation, a_.designation)->setToolTip(a_.designation);
		set_(ColKind, kindName(a_.kind));
			//Unknown stays empty, not 0 (still sorted as numbers)
		set_(ColSection, a_.section > 0 ? QVariant(a_.section) : QVariant());
		set_(ColWidth, a_.width > 0 ? QVariant(a_.width) : QVariant());
		set_(ColClamp, clampName(a_.clamp));
		set_(ColConductors, a_.conductors);
		set_(ColSource, a_.source == QLatin1String("built-in") ? tr("built-in")
						: a_.source == QLatin1String("online") ? tr("Data Portal (online)")
						: a_.source);
	}
	m_table->setSortingEnabled(true);
	applyFilter();
}

void TerminalArticleDialog::applyFilter()
{
	const QString text_ = m_search->text().trimmed();
	const QVariant maker_ = m_manufacturer->currentData();
	const QVariant kind_ = m_kind->currentData();
	const QVariant section_ = m_section->currentData();
	const QVariant clamp_ = m_clamp->currentData();

	int shown_ = 0;
	for (int row_ = 0; row_ < m_table->rowCount(); ++row_)
	{
		auto *first_ = m_table->item(row_, ColManufacturer);
		if (!first_) continue;
		const int i_ = first_->data(Qt::UserRole).toInt();
		const auto &a_ = m_articles.at(i_);
		bool ok_ = true;
		if (maker_.isValid() && a_.manufacturer != maker_.toString()) ok_ = false;
		if (kind_.isValid() && int(a_.kind) != kind_.toInt()) ok_ = false;
		if (section_.isValid() && qAbs(a_.section - section_.toDouble()) > 0.001) ok_ = false;
		if (clamp_.isValid() && int(a_.clamp) != clamp_.toInt()) ok_ = false;
			//What the portal found for the text is shown as found, even
			//when its own words differ (the portal also matches loosely)
		if (ok_ && !text_.isEmpty() && a_.source != QLatin1String("online"))
		{
			const QString hay_ = a_.manufacturer + QLatin1Char(' ') + a_.order_number
					+ QLatin1Char(' ') + a_.designation;
			for (const auto &word_ : text_.split(QLatin1Char(' '), Qt::SkipEmptyParts)) {
				if (!hay_.contains(word_, Qt::CaseInsensitive)) {
					ok_ = false;
					break;
				}
			}
		}
		m_table->setRowHidden(row_, !ok_);
		if (ok_) ++shown_;
	}
	m_count->setText(tr("%1 of %2 terminals").arg(shown_).arg(m_articles.size()));
	m_count->setProperty("shown", shown_);

		//Keep something selected, so the preview always shows a terminal
	const auto selected_ = m_table->selectionModel()->selectedRows();
	if (selected_.isEmpty() || m_table->isRowHidden(selected_.first().row()))
	{
		m_table->clearSelection();
		for (int row_ = 0; row_ < m_table->rowCount(); ++row_) {
			if (!m_table->isRowHidden(row_)) {
				m_table->selectRow(row_);
				break;
			}
		}
	}
	selectionChanged();
}

void TerminalArticleDialog::preselect(const QString &article_text, bool protective_earth)
{
	const Article wanted_ = TerminalArticleCatalog::find(article_text);
	if (protective_earth && (!wanted_.isValid() || wanted_.kind != Kind::ProtectiveEarth))
	{
		const int at_ = m_kind->findData(int(Kind::ProtectiveEarth));
		if (at_ >= 0) m_kind->setCurrentIndex(at_);
	}
	if (!wanted_.isValid()) {
		return;
	}
	for (int row_ = 0; row_ < m_table->rowCount(); ++row_)
	{
		auto *first_ = m_table->item(row_, ColManufacturer);
		if (!first_) continue;
		const auto &a_ = m_articles.at(first_->data(Qt::UserRole).toInt());
		if (a_.manufacturer == wanted_.manufacturer && a_.order_number == wanted_.order_number)
		{
			if (m_table->isRowHidden(row_)) {
				m_kind->setCurrentIndex(0);
			}
			m_table->selectRow(row_);
			m_table->scrollToItem(m_table->item(row_, ColOrder), QAbstractItemView::PositionAtCenter);
			break;
		}
	}
}

Article TerminalArticleDialog::selectedArticle() const
{
	const auto rows_ = m_table->selectionModel()->selectedRows();
	if (rows_.isEmpty() || m_table->isRowHidden(rows_.first().row())) {
		return Article();
	}
	auto *first_ = m_table->item(rows_.first().row(), ColManufacturer);
	if (!first_) {
		return Article();
	}
	return m_articles.value(first_->data(Qt::UserRole).toInt());
}

void TerminalArticleDialog::selectionChanged()
{
	const Article a_ = selectedArticle();
	if (!a_.isValid())
	{
		m_preview->setStrip(QString(), {});
		m_details->clear();
		m_picture->clear();
		return;
	}
	m_picture->setText(tr("Loading picture…"));
	m_picture_url.clear();
	m_picture_timer->start();

	QVector<TerminalPreviewWidget::Terminal> terminals_;
	for (int i = 1; i <= 5; ++i)
	{
		TerminalPreviewWidget::Terminal t_;
		t_.label = QString::number(i);
		t_.article = a_;
		t_.pe = a_.kind == Kind::ProtectiveEarth;
		terminals_ << t_;
	}
	m_preview->setStrip(QStringLiteral("X1"), terminals_);

	m_details->setText(QStringLiteral("<b>%1 %2</b><br>%3<br><br>%4: %5 mm²<br>%6: %7 mm<br>%8: %9<br>%10: %11")
					   .arg(a_.manufacturer.toHtmlEscaped(), a_.order_number.toHtmlEscaped(),
							a_.designation.toHtmlEscaped(),
							tr("Cross-section"), a_.section > 0 ? mm(a_.section) : tr("not given"),
							tr("Width"), a_.width > 0 ? mm(a_.width) : tr("not given (preview with 5.2)"),
							tr("Clamp"), clampName(a_.clamp))
					   .arg(tr("Connection points"), QString::number(a_.conductors)));
}

void TerminalArticleDialog::openCatalogFolder()
{
	const QString dir_ = TerminalArticleCatalog::catalogDirectory(true);
	QDesktopServices::openUrl(QUrl::fromLocalFile(dir_));
}
