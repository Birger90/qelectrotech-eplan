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
#include "drawnterminalsdialog.h"

#include "terminalarticledialog.h"
#include "terminalpreviewwidget.h"
#include "../terminalarticlecatalog.h"

#include <ElementsCollection/elementslocation.h>
#include <elementdialog.h>
#include <qetgraphicsitem/conductor.h>
#include <qetgraphicsitem/element.h>
#include <qetgraphicsitem/terminal.h>
#include <qetproject.h>

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMap>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QStyledItemDelegate>
#include <QTableWidget>
#include <QVBoxLayout>

#include <algorithm>
#include <climits>
#include <functional>

namespace
{
	const char *key_element    = "drawnterminals/element";
	const char *key_article    = "drawnterminals/article";
	const char *key_pe_article = "drawnterminals/pe_article";

		//Rail mounted push-in terminals as the house standard: a
		//feed-through terminal and its protective earth twin
	const char *default_article    = "WAGO 2002-1201";
	const char *default_pe_article = "WAGO 2002-1207";

	enum Column { ColWire = 0, ColPe, ColStrip, ColNumber, ColLevel, ColArticle, ColCount };

		///The strip cell: a list of the strips in use, and free typing
	class StripDelegate : public QStyledItemDelegate
	{
		public:
			StripDelegate(std::function<QStringList()> strips, QObject *parent) :
				QStyledItemDelegate(parent), m_strips(std::move(strips)) {}

			QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &,
								  const QModelIndex &) const override
			{
				auto *combo_ = new QComboBox(parent);
				combo_->setEditable(true);
				combo_->addItems(m_strips());
				return combo_;
			}
			void setEditorData(QWidget *editor, const QModelIndex &index) const override {
				static_cast<QComboBox *>(editor)->setCurrentText(index.data().toString());
			}
			void setModelData(QWidget *editor, QAbstractItemModel *model,
							  const QModelIndex &index) const override {
				model->setData(index, static_cast<QComboBox *>(editor)->currentText().trimmed());
			}

		private:
			std::function<QStringList()> m_strips;
	};

		///"-K1:13 – -M1:U1" for a wire which carries no number of its own
	QString wireText(const Conductor *conductor)
	{
		const QString own_ = DrawnTerminals::conductorText(conductor);
		if (!own_.isEmpty() || !conductor) {
			return own_;
		}
		auto end_ = [](const Terminal *terminal) {
			if (!terminal || !terminal->parentElement()) {
				return QString();
			}
			QString label_ = terminal->parentElement()->actualLabel();
			if (label_.isEmpty()) {
				label_ = terminal->parentElement()->name();
			}
			const QString pin_ = terminal->name();
			return pin_.isEmpty() ? label_ : label_ + QLatin1Char(':') + pin_;
		};
		const QString a_ = end_(conductor->terminal1);
		const QString b_ = end_(conductor->terminal2);
		if (a_.isEmpty() && b_.isEmpty()) {
			return QString();
		}
		return a_ + QStringLiteral(" – ") + b_;
	}

	int numberOf(const QString &text)
	{
		bool ok_ = false;
		const int n_ = text.trimmed().toInt(&ok_);
		return ok_ ? n_ : INT_MAX;
	}
}

DrawnTerminalsDialog::DrawnTerminalsDialog(QETProject *project,
										   const QList<DrawnTerminals::Row> &rows,
										   QWidget *parent) :
	QDialog(parent),
	m_project(project),
	m_rows(rows),
	m_own_strip(rows.size(), false),
	m_own_article(rows.size(), false),
	m_own_number(rows.size(), false)
{
	setWindowTitle(tr("Draw terminal strip"));

	QSettings settings_;

	m_strip = new QComboBox(this);
	m_strip->setEditable(true);
	const QString next_ = DrawnTerminals::nextFreeStrip(project);
	m_strip->addItem(next_);
	for (const auto &name_ : DrawnTerminals::stripNames(project)) {
		m_strip->addItem(name_);
	}
	m_strip->setCurrentText(next_);
	m_strip->setToolTip(tr("The strip of every wire you did not move elsewhere; a new strip is "
						   "suggested, pick an existing one to add the terminals to it"));

	m_first = new QSpinBox(this);
	m_first->setRange(1, 9999);

	m_element_path = settings_.value(QLatin1String(key_element),
									 DrawnTerminals::defaultElementPath()).toString();
	m_element_label = new QLabel(this);
	auto *choose_ = new QPushButton(tr("Choose…"), this);
	auto *element_row_ = new QHBoxLayout;
	element_row_->addWidget(m_element_label, 1);
	element_row_->addWidget(choose_);

	m_article = new QLineEdit(settings_.value(QLatin1String(key_article),
											  QLatin1String(default_article)).toString(), this);
	m_article->setPlaceholderText(tr("Manufacturer and order number, e.g. WAGO 2002-1201"));
	m_pe_article = new QLineEdit(settings_.value(QLatin1String(key_pe_article),
												 QLatin1String(default_pe_article)).toString(), this);

	auto *catalog_ = new QPushButton(tr("Catalogue…"), this);
	catalog_->setToolTip(tr("Choose the terminal from the catalogue, with filters and a preview"));
	auto *pe_catalog_ = new QPushButton(tr("Catalogue…"), this);
	pe_catalog_->setToolTip(catalog_->toolTip());
	m_pe_catalog = pe_catalog_;
	m_levels_hint = new QLabel(this);
	m_levels_hint->setWordWrap(true);
	m_levels_hint->setStyleSheet(QStringLiteral("color: #00706e;"));
	auto *article_row_ = new QHBoxLayout;
	article_row_->addWidget(m_article, 1);
	article_row_->addWidget(catalog_);
	auto *pe_article_row_ = new QHBoxLayout;
	pe_article_row_->addWidget(m_pe_article, 1);
	pe_article_row_->addWidget(pe_catalog_);

	auto *form_ = new QFormLayout;
	form_->addRow(tr("Terminal strip:"), m_strip);
	form_->addRow(tr("First number:"), m_first);
	form_->addRow(tr("Symbol:"), element_row_);
	form_->addRow(tr("Article:"), article_row_);
	form_->addRow(tr("PE article:"), pe_article_row_);
	form_->addRow(QString(), m_levels_hint);

	m_table = new QTableWidget(int(m_rows.size()), ColCount, this);
	m_table->setHorizontalHeaderLabels({tr("Wire"), tr("PE"), tr("Strip"), tr("Terminal"),
										tr("Level"), tr("Article")});
	m_table->verticalHeader()->setVisible(false);
	m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_table->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed
							 | QAbstractItemView::AnyKeyPressed);
	auto *header_ = m_table->horizontalHeader();
	header_->setSectionResizeMode(ColWire, QHeaderView::Stretch);
	header_->setSectionResizeMode(ColArticle, QHeaderView::Stretch);
	for (int c : {ColPe, ColStrip, ColNumber, ColLevel}) {
		header_->setSectionResizeMode(c, QHeaderView::ResizeToContents);
	}
	m_table->setItemDelegateForColumn(ColStrip, new StripDelegate([this]() { return knownStrips(); }, m_table));
	for (int i = 0; i < m_rows.size(); ++i)
	{
		auto *wire_ = new QTableWidgetItem(wireText(m_rows.at(i).conductor));
		if (wire_->text().isEmpty()) {
			wire_->setText(tr("Wire %1").arg(i + 1));
		}
		wire_->setFlags(wire_->flags() & ~Qt::ItemIsEditable);
		m_table->setItem(i, ColWire, wire_);
		auto *pe_ = new QTableWidgetItem;
		pe_->setFlags((pe_->flags() | Qt::ItemIsUserCheckable) & ~Qt::ItemIsEditable);
		pe_->setCheckState(m_rows.at(i).pe ? Qt::Checked : Qt::Unchecked);
		m_table->setItem(i, ColPe, pe_);
		m_table->setItem(i, ColStrip, new QTableWidgetItem);
		m_table->setItem(i, ColNumber, new QTableWidgetItem);
		auto *level_ = new QTableWidgetItem;
		level_->setFlags(level_->flags() & ~Qt::ItemIsEditable);
		m_table->setItem(i, ColLevel, level_);
		auto *article_ = new QTableWidgetItem;
		article_->setFlags(article_->flags() & ~Qt::ItemIsEditable);
		m_table->setItem(i, ColArticle, article_);
	}
	m_table->setMinimumHeight(170);

		//What the selected wires can be given in one go
	auto *own_strip_ = new QPushButton(tr("Own strip"), this);
	own_strip_->setToolTip(tr("Put the selected wires on a strip of their own (N on X2, PE on X3, …); "
							  "they are numbered there from 1"));
	auto *row_article_ = new QPushButton(tr("Article…"), this);
	row_article_->setToolTip(tr("Choose the article of the selected wires from the catalogue"));
	auto *reset_ = new QPushButton(tr("Reset"), this);
	reset_->setToolTip(tr("Every wire back to the strip and the article at the top"));
	auto *selection_row_ = new QHBoxLayout;
	selection_row_->addWidget(new QLabel(tr("Selected wires:"), this));
	selection_row_->addWidget(own_strip_);
	selection_row_->addWidget(row_article_);
	selection_row_->addStretch(1);
	selection_row_->addWidget(reset_);

	auto *hint_ = new QLabel(tr("%n wire(s) crossed. Every wire gets a terminal at the crossing and runs "
								"through it. Strip, number and article can be changed for each wire "
								"(double-click); wires with the same strip and number share one terminal "
								"of several levels.", nullptr, m_rows.size()), this);
	hint_->setWordWrap(true);

	auto *buttons_ = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	buttons_->button(QDialogButtonBox::Ok)->setText(tr("Place terminals"));
	buttons_->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));

	m_preview = new TerminalPreviewWidget(this);
	m_preview->setMinimumHeight(150);
	m_preview_label = new QLabel(this);
	m_preview_label->setWordWrap(true);

	auto *layout_ = new QVBoxLayout(this);
	layout_->addWidget(hint_);
	layout_->addLayout(form_);
	layout_->addWidget(m_table, 1);
	layout_->addLayout(selection_row_);
	layout_->addWidget(m_preview_label);
	layout_->addWidget(m_preview);
	layout_->addWidget(buttons_);

	connect(buttons_, &QDialogButtonBox::accepted, this, &DrawnTerminalsDialog::accept);
	connect(buttons_, &QDialogButtonBox::rejected, this, &DrawnTerminalsDialog::reject);
	connect(choose_, &QPushButton::clicked, this, &DrawnTerminalsDialog::chooseElement);
	connect(m_strip, &QComboBox::currentTextChanged, this, &DrawnTerminalsDialog::stripChanged);
	connect(m_first, qOverload<int>(&QSpinBox::valueChanged), this, &DrawnTerminalsDialog::rebuild);
	connect(catalog_, &QPushButton::clicked, this, [this]() { chooseArticle(false); });
	connect(pe_catalog_, &QPushButton::clicked, this, [this]() { chooseArticle(true); });
	connect(m_article, &QLineEdit::textChanged, this, &DrawnTerminalsDialog::articleChanged);
	connect(m_pe_article, &QLineEdit::textChanged, this, &DrawnTerminalsDialog::rebuild);
	connect(m_table, &QTableWidget::cellChanged, this, &DrawnTerminalsDialog::itemChanged);
	connect(m_table, &QTableWidget::cellDoubleClicked, this, [this](int, int column) {
		if (column == ColArticle) {
			chooseRowArticle();
		}
	});
	connect(m_table, &QTableWidget::itemSelectionChanged, this, &DrawnTerminalsDialog::currentRowChanged);
	connect(own_strip_, &QPushButton::clicked, this, &DrawnTerminalsDialog::ownStripForSelection);
	connect(row_article_, &QPushButton::clicked, this, &DrawnTerminalsDialog::chooseRowArticle);
	connect(reset_, &QPushButton::clicked, this, &DrawnTerminalsDialog::resetRows);

	showElement();
	{
		const QSignalBlocker blocker_(m_first);
		m_first->setValue(DrawnTerminals::nextNumber(m_project, m_strip->currentText()));
	}
	articleChanged();
	if (m_table->rowCount()) {
		m_table->selectRow(0);
	}
	resize(760, qMin(880, 540 + 26 * int(m_rows.size())));
}

DrawnTerminalsDialog::~DrawnTerminalsDialog()
{
		//The wires shown on the folio go back to unselected
	for (const auto &row_ : std::as_const(m_rows)) {
		if (row_.conductor) {
			row_.conductor->setSelected(false);
		}
	}
}

QList<DrawnTerminals::Row> DrawnTerminalsDialog::rows() const
{
	return m_rows;
}

DrawnTerminals::Settings DrawnTerminalsDialog::settings() const
{
	DrawnTerminals::Settings s_;
	s_.strip = m_strip->currentText().trimmed();
	s_.element_path = m_element_path;
	s_.article = m_article->text().trimmed();
	s_.pe_article = m_pe_article->text().trimmed();
	return s_;
}

void DrawnTerminalsDialog::articleChanged()
{
	const QString article_ = m_article->text();
	const int levels_ = DrawnTerminals::levelsOf(article_);
	const bool pe_inside_ = DrawnTerminals::peLevelOf(article_) >= 0;
		//Only a terminal with a PE level takes the PE wire itself: the
		//PE article is put aside then and comes back with any other one
	if (pe_inside_ && m_pe_article->isEnabled())
	{
		m_saved_pe_article = m_pe_article->text();
		const QSignalBlocker blocker_(m_pe_article);
		m_pe_article->clear();
		m_pe_article->setPlaceholderText(tr("PE is a level of the terminal"));
	}
	else if (!pe_inside_ && !m_pe_article->isEnabled())
	{
		const QSignalBlocker blocker_(m_pe_article);
		m_pe_article->setText(m_saved_pe_article);
		m_pe_article->setPlaceholderText(QString());
	}
	m_pe_article->setEnabled(!pe_inside_);
	if (m_pe_catalog) {
		m_pe_catalog->setEnabled(!pe_inside_);
	}

	m_levels_hint->setVisible(levels_ > 1);
	if (levels_ > 1)
	{
		QStringList names_;
		for (int l = 0; l < levels_; ++l) {
			names_ << DrawnTerminals::levelName(article_, l);
		}
		QString text_ = tr("Terminal with %1 levels (%2): the wires of one strip fill its levels "
						   "in turn.").arg(levels_).arg(names_.join(QStringLiteral(" / ")));
		text_ += QLatin1Char(' ');
		text_ += pe_inside_ ? tr("PE wires go on its PE level.")
							: tr("It has no PE level: PE wires get a terminal of their own with the PE article.");
		text_ += QLatin1Char(' ');
		text_ += tr("Put N or PE on a strip of their own with \"Own strip\".");
		m_levels_hint->setText(text_);
	}
	rebuild();
}

void DrawnTerminalsDialog::stripChanged()
{
	const QSignalBlocker blocker_(m_first);
	m_first->setValue(DrawnTerminals::nextNumber(m_project, m_strip->currentText()));
	rebuild();
}

/**
	@brief DrawnTerminalsDialog::rebuild
	Work every row out again from the settings at the top, keeping what
	the user set for a row himself, and show it.
*/
void DrawnTerminalsDialog::rebuild()
{
	if (!m_table) {
		return;
	}
	const auto settings_ = settings();
	QStringList kept_numbers_;
	for (int i = 0; i < m_rows.size(); ++i)
	{
		auto &row_ = m_rows[i];
		if (!m_own_strip.at(i)) {
			row_.strip = settings_.strip;
		}
		if (!m_own_article.at(i)) {
			row_.article = DrawnTerminals::defaultArticle(row_, settings_);
		}
		kept_numbers_ << row_.number;
	}
	QHash<QString, int> first_;
	first_.insert(settings_.strip, m_first->value());
	DrawnTerminals::numberRows(m_project, m_rows, first_);
	bool own_numbers_ = false;
	for (int i = 0; i < m_rows.size(); ++i)
	{
		if (m_own_number.at(i)) {
			m_rows[i].number = kept_numbers_.at(i);
			own_numbers_ = true;
		}
	}
	if (own_numbers_) {
		assignLevels();
	}
	fillTable();
	updatePreview();
}

/**
	@brief DrawnTerminalsDialog::assignLevels
	The levels once numbers were typed by hand: the rows of one strip and
	one number share a terminal, PE on its PE level, the others on the
	free levels in the order of the rows.
*/
void DrawnTerminalsDialog::assignLevels()
{
	QHash<QString, QVector<bool>> taken_;
	for (auto &row_ : m_rows)
	{
		const int levels_ = DrawnTerminals::levelsOf(row_.article);
		if (levels_ < 2) {
			row_.level = 0;
			continue;
		}
		const QString key_ = row_.strip + QLatin1Char(':') + row_.number.trimmed();
		auto &used_ = taken_[key_];
		if (used_.size() < levels_) {
			used_.resize(levels_);
		}
		const int pe_level_ = DrawnTerminals::peLevelOf(row_.article);
		int level_ = -1;
		if (row_.pe && pe_level_ >= 0 && !used_.at(pe_level_)) {
			level_ = pe_level_;
		}
		for (int l = 0; level_ < 0 && l < used_.size(); ++l) {
			if (!used_.at(l) && (l != pe_level_ || row_.pe)) {
				level_ = l;
			}
		}
		if (level_ < 0) {
				//More wires than levels: shown, refused on accept
			level_ = int(used_.size());
			used_.resize(level_ + 1);
		}
		used_[level_] = true;
		row_.level = level_;
	}
}

void DrawnTerminalsDialog::fillTable()
{
	m_filling = true;
	const QSignalBlocker blocker_(m_table);
	const QString default_strip_ = m_strip->currentText().trimmed();
	for (int i = 0; i < m_rows.size(); ++i)
	{
		const auto &row_ = m_rows.at(i);
		m_table->item(i, ColStrip)->setText(row_.strip);
		m_table->item(i, ColNumber)->setText(row_.number);
		m_table->item(i, ColLevel)->setText(DrawnTerminals::levelName(row_.article, row_.level));
		const auto catalog_ = TerminalArticleCatalog::find(row_.article);
		QString article_text_ = row_.article;
		if (catalog_.isValid() && !catalog_.type_number.isEmpty()) {
			article_text_ += QStringLiteral(" (") + catalog_.type_number + QLatin1Char(')');
		}
		auto *article_item_ = m_table->item(i, ColArticle);
		article_item_->setText(article_text_);
		article_item_->setToolTip((catalog_.isValid() ? catalog_.designation + QChar(0x0A) : QString())
								  + tr("Double-click to choose another article for this wire"));
			//What differs from the top is bold: a wire moved elsewhere
		QFont strip_font_ = m_table->item(i, ColStrip)->font();
		strip_font_.setBold(row_.strip != default_strip_);
		m_table->item(i, ColStrip)->setFont(strip_font_);
		QFont article_font_ = article_item_->font();
		article_font_.setBold(m_own_article.at(i));
		article_item_->setFont(article_font_);
	}
	m_filling = false;
}

void DrawnTerminalsDialog::itemChanged(int row, int column)
{
	if (m_filling || row < 0 || row >= m_rows.size()) {
		return;
	}
	QList<int> targets_ = selectedRows();
	if (!targets_.contains(row)) {
		targets_ = {row};
	}

	if (column == ColStrip)
	{
		const QString strip_ = m_table->item(row, ColStrip)->text().trimmed();
		if (strip_.isEmpty()) {
			fillTable();
			return;
		}
			//A strip typed into one of several selected rows is for all of them
		for (int i : std::as_const(targets_))
		{
			m_rows[i].strip = strip_;
			m_own_strip[i] = strip_ != m_strip->currentText().trimmed();
			m_own_number[i] = false;
		}
		rebuild();
	}
	else if (column == ColNumber)
	{
		m_rows[row].number = m_table->item(row, ColNumber)->text().trimmed();
		m_own_number[row] = true;
		rebuild();
	}
	else if (column == ColPe)
	{
		m_rows[row].pe = m_table->item(row, ColPe)->checkState() == Qt::Checked;
		m_own_number[row] = false;
		rebuild();
	}
}

QList<int> DrawnTerminalsDialog::selectedRows() const
{
	QList<int> rows_;
	for (const auto &range_ : m_table->selectedRanges()) {
		for (int r = range_.topRow(); r <= range_.bottomRow(); ++r) {
			if (!rows_.contains(r)) {
				rows_ << r;
			}
		}
	}
	std::sort(rows_.begin(), rows_.end());
	if (rows_.isEmpty() && m_table->currentRow() >= 0) {
		rows_ << m_table->currentRow();
	}
	return rows_;
}

QStringList DrawnTerminalsDialog::knownStrips() const
{
	QStringList strips_ = DrawnTerminals::stripNames(m_project);
	for (const auto &row_ : m_rows) {
		if (!row_.strip.isEmpty() && !strips_.contains(row_.strip)) {
			strips_ << row_.strip;
		}
	}
	const QString top_ = m_strip->currentText().trimmed();
	if (!top_.isEmpty() && !strips_.contains(top_)) {
		strips_.prepend(top_);
	}
	return strips_;
}

/**
	@brief DrawnTerminalsDialog::ownStripForSelection
	The selected wires on the next strip nobody uses yet, neither in the
	project nor in this dialog: select N, click, it is X2; select PE,
	click, it is X3.
*/
void DrawnTerminalsDialog::ownStripForSelection()
{
	const auto targets_ = selectedRows();
	if (targets_.isEmpty()) {
		return;
	}
	const QStringList used_ = knownStrips();
		//The project's way of writing it: "-X" where it uses the prefix
	const bool dash_ = std::any_of(used_.begin(), used_.end(), [](const QString &name) {
		return name.startsWith(QLatin1Char('-'));
	});
	const QString prefix_ = dash_ ? QStringLiteral("-X") : QStringLiteral("X");
	QString name_;
	for (int n = 1; n < 10000 && name_.isEmpty(); ++n)
	{
		const QString candidate_ = prefix_ + QString::number(n);
		if (!used_.contains(candidate_)) {
			name_ = candidate_;
		}
	}
	for (int i : targets_)
	{
		m_rows[i].strip = name_;
		m_own_strip[i] = true;
		m_own_number[i] = false;
	}
	rebuild();
}

void DrawnTerminalsDialog::chooseRowArticle()
{
	const auto targets_ = selectedRows();
	if (targets_.isEmpty()) {
		return;
	}
	const auto &first_ = m_rows.at(targets_.first());
	const auto article_ = TerminalArticleDialog::choose(this, first_.article, first_.pe);
	if (!article_.isValid()) {
		return;
	}
	for (int i : targets_)
	{
		m_rows[i].article = article_.articleText();
		m_own_article[i] = true;
		m_own_number[i] = false;
	}
	rebuild();
}

void DrawnTerminalsDialog::resetRows()
{
	m_own_strip.fill(false);
	m_own_article.fill(false);
	m_own_number.fill(false);
	rebuild();
}

void DrawnTerminalsDialog::accept()
{
	if (m_strip->currentText().trimmed().isEmpty())
	{
		QMessageBox::warning(this, windowTitle(), tr("The terminal strip needs a name."));
		return;
	}
	QHash<QString, int> wires_;
	for (int i = 0; i < m_rows.size(); ++i)
	{
		const auto &row_ = m_rows.at(i);
		if (row_.strip.trimmed().isEmpty() || row_.number.trimmed().isEmpty())
		{
			QMessageBox::warning(this, windowTitle(), tr("Every terminal needs a strip and a number."));
			m_table->selectRow(i);
			return;
		}
		const QString key_ = row_.strip + QLatin1Char(':') + row_.number.trimmed();
		const int count_ = ++wires_[key_];
		if (count_ > DrawnTerminals::levelsOf(row_.article))
		{
			QMessageBox::warning(this, windowTitle(),
								 tr("Terminal %1 gets more wires than it has levels. Give the wires "
									"different numbers or a terminal with more levels.").arg(key_));
			m_table->selectRow(i);
			return;
		}
	}

		//The next strip starts from the same choice
	QSettings settings_;
	settings_.setValue(QLatin1String(key_element), m_element_path);
	settings_.setValue(QLatin1String(key_article), m_article->text().trimmed());
	settings_.setValue(QLatin1String(key_pe_article), m_pe_article->isEnabled()
					   ? m_pe_article->text().trimmed() : m_saved_pe_article.trimmed());
	QDialog::accept();
}

void DrawnTerminalsDialog::chooseElement()
{
	const ElementsLocation location_ = ElementDialog::getOpenElementLocation(this);
	if (!location_.isElement()) {
		return;
	}
	m_element_path = location_.collectionPath(true);
	showElement();
}

void DrawnTerminalsDialog::showElement()
{
	const ElementsLocation location_(m_element_path);
	const QString name_ = location_.exist() ? location_.name() : QString();
	m_element_label->setText(name_.isEmpty() ? m_element_path : name_);
	m_element_label->setToolTip(m_element_path);
}

void DrawnTerminalsDialog::chooseArticle(bool pe)
{
	QLineEdit *field_ = pe ? m_pe_article : m_article;
	const auto article_ = TerminalArticleDialog::choose(this, field_->text(), pe);
	if (article_.isValid()) {
		field_->setText(article_.articleText());
	}
}

/**
	@brief DrawnTerminalsDialog::currentRowChanged
	Show the wires of the selected rows on the folio, and the strip the
	current one goes into.
*/
void DrawnTerminalsDialog::currentRowChanged()
{
	const auto selected_ = selectedRows();
	for (int i = 0; i < m_rows.size(); ++i) {
		if (auto *conductor_ = m_rows.at(i).conductor) {
			conductor_->setSelected(selected_.contains(i));
		}
	}
	updatePreview();
}

void DrawnTerminalsDialog::updatePreview()
{
	if (!m_preview || !m_table) {
		return;
	}
	const int current_ = m_table->currentRow();
	const QString strip_ = current_ >= 0 && current_ < m_rows.size()
						   ? m_rows.at(current_).strip : m_strip->currentText().trimmed();
	QStringList strips_;
	for (const auto &row_ : std::as_const(m_rows)) {
		if (!strips_.contains(row_.strip)) {
			strips_ << row_.strip;
		}
	}
	m_preview_label->setText(strips_.size() > 1
							 ? tr("Preview of strip %1 (from above, at scale) – this line makes %2 strips: %3")
								   .arg(strip_).arg(strips_.size()).arg(strips_.join(QStringLiteral(", ")))
							 : tr("Preview of strip %1 (from above, at scale):").arg(strip_));

		//The terminals of this strip, by number, each with all its levels:
		//the levels no wire of this line uses stay empty
	QMap<int, QList<int>> terminals_by_number_;
	for (int i = 0; i < m_rows.size(); ++i) {
		if (m_rows.at(i).strip == strip_) {
			terminals_by_number_[numberOf(m_rows.at(i).number)] << i;
		}
	}

	QVector<TerminalPreviewWidget::Terminal> terminals_;
	int group_ = 0;
	for (auto it_ = terminals_by_number_.cbegin(); it_ != terminals_by_number_.cend(); ++it_, ++group_)
	{
		const auto &first_ = m_rows.at(it_.value().first());
		const auto article_ = TerminalArticleCatalog::find(first_.article);
		const int levels_ = qMax(DrawnTerminals::levelsOf(first_.article), int(it_.value().size()));
		const int pe_level_ = DrawnTerminals::peLevelOf(first_.article);
		for (int l = 0; l < levels_; ++l)
		{
			TerminalPreviewWidget::Terminal t_;
			t_.label = first_.number;
			t_.article = article_;
			t_.group = group_;
			t_.pe = levels_ == 1 ? first_.pe : l == pe_level_;
			terminals_ << t_;
		}
	}
	m_preview->setStrip(strip_, terminals_);
}
