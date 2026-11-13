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

#include <ElementsCollection/elementslocation.h>
#include <elementdialog.h>
#include <qetproject.h>

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QTableWidget>
#include <QVBoxLayout>

namespace
{
	const char *key_element    = "drawnterminals/element";
	const char *key_article    = "drawnterminals/article";
	const char *key_pe_article = "drawnterminals/pe_article";

		//Rail mounted push-in terminals as the house standard: a
		//feed-through terminal and its protective earth twin
	const char *default_article    = "WAGO 2002-1201";
	const char *default_pe_article = "WAGO 2002-1207";

	enum Column { ColNumber = 0, ColLevel, ColWire, ColPe, ColCount };
}

DrawnTerminalsDialog::DrawnTerminalsDialog(QETProject *project,
										   const QList<DrawnTerminals::Row> &rows,
										   QWidget *parent) :
	QDialog(parent),
	m_project(project),
	m_rows(rows)
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
	m_strip->setToolTip(tr("A new strip is suggested; pick an existing one to add the terminals to it"));

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
	m_table->setHorizontalHeaderLabels({tr("Terminal"), tr("Level"), tr("Wire"), tr("PE")});
	m_table->verticalHeader()->setVisible(false);
	m_table->horizontalHeader()->setSectionResizeMode(ColWire, QHeaderView::Stretch);
	m_table->horizontalHeader()->setSectionResizeMode(ColNumber, QHeaderView::ResizeToContents);
	m_table->horizontalHeader()->setSectionResizeMode(ColLevel, QHeaderView::ResizeToContents);
	m_table->horizontalHeader()->setSectionResizeMode(ColPe, QHeaderView::ResizeToContents);
	for (int i = 0; i < m_rows.size(); ++i)
	{
		m_table->setItem(i, ColNumber, new QTableWidgetItem);
		auto *level_ = new QTableWidgetItem;
		level_->setFlags(level_->flags() & ~Qt::ItemIsEditable);
		m_table->setItem(i, ColLevel, level_);
		auto *wire_ = new QTableWidgetItem(DrawnTerminals::conductorText(m_rows.at(i).conductor));
		wire_->setFlags(wire_->flags() & ~Qt::ItemIsEditable);
		m_table->setItem(i, ColWire, wire_);
		auto *pe_ = new QTableWidgetItem;
		pe_->setFlags((pe_->flags() | Qt::ItemIsUserCheckable) & ~Qt::ItemIsEditable);
		pe_->setCheckState(m_rows.at(i).pe ? Qt::Checked : Qt::Unchecked);
		m_table->setItem(i, ColPe, pe_);
	}

	auto *hint_ = new QLabel(tr("%n wire(s) crossed. Every wire gets a terminal at the crossing "
								"and runs through it; the terminal strip, the terminal plan and "
								"the bill of materials follow from it.", nullptr, m_rows.size()), this);
	hint_->setWordWrap(true);

	auto *buttons_ = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	buttons_->button(QDialogButtonBox::Ok)->setText(tr("Place terminals"));
	buttons_->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
	m_table->setMinimumHeight(150);

	m_preview = new TerminalPreviewWidget(this);
	m_preview->setMinimumHeight(150);

	auto *layout_ = new QVBoxLayout(this);
	layout_->addWidget(hint_);
	layout_->addLayout(form_);
	layout_->addWidget(m_table, 1);
	layout_->addWidget(new QLabel(tr("Preview of the strip (from above, at scale):"), this));
	layout_->addWidget(m_preview);
	layout_->addWidget(buttons_);

	connect(buttons_, &QDialogButtonBox::accepted, this, &DrawnTerminalsDialog::accept);
	connect(buttons_, &QDialogButtonBox::rejected, this, &DrawnTerminalsDialog::reject);
	connect(choose_, &QPushButton::clicked, this, &DrawnTerminalsDialog::chooseElement);
	connect(m_strip, &QComboBox::currentTextChanged, this, &DrawnTerminalsDialog::stripChanged);
	connect(m_first, qOverload<int>(&QSpinBox::valueChanged), this, &DrawnTerminalsDialog::renumber);
	connect(catalog_, &QPushButton::clicked, this, [this]() { chooseArticle(false); });
	connect(pe_catalog_, &QPushButton::clicked, this, [this]() { chooseArticle(true); });
	connect(m_article, &QLineEdit::textChanged, this, &DrawnTerminalsDialog::articleChanged);
	connect(m_pe_article, &QLineEdit::textChanged, this, &DrawnTerminalsDialog::updatePreview);
	connect(m_table, &QTableWidget::itemChanged, this, &DrawnTerminalsDialog::updatePreview);

	showElement();
	stripChanged();
	articleChanged();
	resize(620, qMin(820, 470 + 26 * int(m_rows.size())));
}

QList<DrawnTerminals::Row> DrawnTerminalsDialog::rows() const
{
	QList<DrawnTerminals::Row> rows_ = m_rows;
	for (int i = 0; i < rows_.size(); ++i)
	{
		rows_[i].number = m_table->item(i, ColNumber)->text().trimmed();
		rows_[i].pe = m_table->item(i, ColPe)->checkState() == Qt::Checked;
	}
	return rows_;
}

DrawnTerminals::Settings DrawnTerminalsDialog::settings() const
{
	DrawnTerminals::Settings s_;
	s_.strip = m_strip->currentText().trimmed();
	s_.element_path = m_element_path;
	s_.article = m_article->text().trimmed();
	s_.pe_article = m_pe_article->text().trimmed();
	s_.levels = levels();
	return s_;
}

int DrawnTerminalsDialog::levels() const
{
	return DrawnTerminals::levelsOf(m_article->text());
}

void DrawnTerminalsDialog::articleChanged()
{
	const int levels_ = levels();
	const bool several_ = levels_ > 1;
		//The PE of a terminal of several levels is one of its levels: the
		//PE article is put aside (and comes back with a one-level terminal)
	if (several_ && m_pe_article->isEnabled())
	{
		m_saved_pe_article = m_pe_article->text();
		const QSignalBlocker blocker_(m_pe_article);
		m_pe_article->clear();
		m_pe_article->setPlaceholderText(tr("PE is a level of the terminal"));
	}
	else if (!several_ && !m_pe_article->isEnabled())
	{
		const QSignalBlocker blocker_(m_pe_article);
		m_pe_article->setText(m_saved_pe_article);
		m_pe_article->setPlaceholderText(QString());
	}
	m_pe_article->setEnabled(!several_);
	if (m_pe_catalog) {
		m_pe_catalog->setEnabled(!several_);
	}
	m_levels_hint->setVisible(several_);
	if (several_)
	{
		const int wires_ = int(m_rows.size());
		const int terminals_ = (wires_ + levels_ - 1) / levels_;
		m_levels_hint->setText(tr("Terminal with %1 levels: every %1 wires share one terminal "
								  "(%2 terminal(s) for %3 wires), one number and one article; "
								  "the PE article is not needed.")
							   .arg(levels_).arg(terminals_).arg(wires_));
	}
	renumber();
	updatePreview();
}

void DrawnTerminalsDialog::accept()
{
	if (m_strip->currentText().trimmed().isEmpty())
	{
		QMessageBox::warning(this, windowTitle(), tr("The terminal strip needs a name."));
		return;
	}
	for (int i = 0; i < m_table->rowCount(); ++i)
	{
		if (m_table->item(i, ColNumber)->text().trimmed().isEmpty())
		{
			QMessageBox::warning(this, windowTitle(), tr("Every terminal needs a number."));
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

void DrawnTerminalsDialog::stripChanged()
{
	const QSignalBlocker blocker_(m_first);
	m_first->setValue(DrawnTerminals::nextNumber(m_project, m_strip->currentText()));
	renumber();
	updatePreview();
}

void DrawnTerminalsDialog::renumber()
{
	const int levels_ = levels();
	const QSignalBlocker blocker_(m_table);
	for (int i = 0; i < m_table->rowCount(); ++i)
	{
		m_table->item(i, ColNumber)->setText(QString::number(m_first->value() + i / levels_));
		m_table->item(i, ColLevel)->setText(levels_ > 1 ? QString::number(i % levels_ + 1) : QString());
	}
	updatePreview();
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

void DrawnTerminalsDialog::updatePreview()
{
	if (!m_preview || !m_table) {
		return;
	}
	const auto normal_ = TerminalArticleCatalog::find(m_article->text());
	const int levels_ = levels();
	auto pe_ = TerminalArticleCatalog::find(m_pe_article->text());
	if (levels_ > 1 || (!pe_.isValid() && m_pe_article->text().trimmed().isEmpty())) {
		pe_ = normal_;
	}

	QVector<TerminalPreviewWidget::Terminal> terminals_;
	for (int i = 0; i < m_table->rowCount(); ++i)
	{
		auto *number_ = m_table->item(i, ColNumber);
		auto *pe_item_ = m_table->item(i, ColPe);
		if (!number_ || !pe_item_) {
			continue;
		}
		TerminalPreviewWidget::Terminal t_;
		t_.label = number_->text();
		t_.pe = pe_item_->checkState() == Qt::Checked;
		t_.article = t_.pe ? pe_ : normal_;
		t_.group = i / levels_;
		terminals_ << t_;
	}
	m_preview->setStrip(m_strip->currentText().trimmed(), terminals_);
}
