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
#include "plclinkwidget.h"

#include <qetgraphicsitem/element.h>
#include <qetgraphicsitem/terminal.h>
#include <elementprovider.h>
#include <undocommand/linkelementcommand.h>
#include <diagram.h>
#include <qetproject.h>
#include <properties/elementdata.h>

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardItemModel>
#include <QVBoxLayout>

#include <algorithm>

namespace {
	/// 1 input, 2 output
	int ioDirection(ElementData::PlcIOType type)
	{
		switch (type) {
			case ElementData::SortieDigitale:
			case ElementData::SortieAnalogique:
			case ElementData::SortieUniverselle:
				return 2;
			default:
				return 1;
		}
	}

	bool isPlcModule(const Element *e)
	{
		return e && e->elementData().m_type == ElementData::Master
			&& e->elementData().m_master_type == ElementData::PLC
			&& !e->elementData().plcMasterData().ios.isEmpty();
	}

	/// the master @p slave is linked to and the channel it sits on
	QPair<Element *, int> linkOf(Element *slave)
	{
		if (!slave) return {nullptr, -1};
		for (Element *e : slave->linkedElements())
			if (e->linkType() == Element::Master)
				return {e, e->groupIndexForElement(slave)};
		return {nullptr, -1};
	}

	QString folioOf(const Element *e)
	{
		return e && e->diagram() ? QString::number(e->diagram()->folioIndex() + 1) : QString();
	}
}

PlcLinkWidget::PlcLinkWidget(Element *elmt, QWidget *parent)
	: AbstractElementPropertiesEditorWidget(parent)
{
	m_state = new QLabel(this);
	m_state->setWordWrap(true);
	m_unlink_pb = new QPushButton(tr("Unlink"), this);
	m_show_module_pb = new QPushButton(tr("Show the module"), this);
	auto *state_row = new QHBoxLayout;
	state_row->addWidget(m_state, 1);
	state_row->addWidget(m_show_module_pb);
	state_row->addWidget(m_unlink_pb);

	m_module_cb = new QComboBox(this);
	m_module_cb->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
	m_channel_cb = new QComboBox(this);
	m_channel_cb->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
	m_channel_cb->setModel(new QStandardItemModel(m_channel_cb));
	m_all_cb = new QCheckBox(tr("Also offer channels of the other direction"), this);
	m_all_cb->setChecked(QSettings().value(QStringLiteral("plclinkwidget/allChannels"), false).toBool());

	auto *form = new QFormLayout;
	form->addRow(tr("Module:"), m_module_cb);
	form->addRow(tr("Channel:"), m_channel_cb);
	form->addRow(QString(), m_all_cb);

	m_hint = new QLabel(this);
	m_hint->setWordWrap(true);
	QFont italic = m_hint->font();
	italic.setItalic(true);
	m_hint->setFont(italic);

	auto *layout = new QVBoxLayout(this);
	layout->addLayout(state_row);
	layout->addLayout(form);
	layout->addWidget(m_hint);
	layout->addStretch(1);

	connect(m_unlink_pb, &QPushButton::clicked, this, &PlcLinkWidget::unlink);
	connect(m_show_module_pb, &QPushButton::clicked, this, &PlcLinkWidget::showModule);
	connect(m_module_cb, &QComboBox::currentIndexChanged, this, [this]() {
		if (!m_filling) fillChannels();
	});
	connect(m_channel_cb, &QComboBox::activated, this, &PlcLinkWidget::channelChosen);
	connect(m_all_cb, &QCheckBox::toggled, this, [this](bool on) {
		QSettings().setValue(QStringLiteral("plclinkwidget/allChannels"), on);
		updateUi();
	});

	if (elmt)
		setElement(elmt);
}

void PlcLinkWidget::setElement(Element *element)
{
	if (m_element == element)
		return;
	m_element = element;
	updateUi();
}

void PlcLinkWidget::apply()
{
	QUndoCommand *undo = associatedUndo();
	if (undo)
		m_element->diagram()->undoStack().push(undo);

	m_element_to_link = nullptr;
	m_pending_io_index = -1;
	updateUi();
}

QUndoCommand *PlcLinkWidget::associatedUndo() const
{
	if (!m_element || !m_element_to_link || m_pending_io_index < 0)
		return nullptr;

	auto *undo = new LinkElementCommand(m_element);
	undo->setLink(m_element_to_link);
	undo->setGroupIndex(m_pending_io_index);
	return undo;
}

QString PlcLinkWidget::title() const
{
	return tr("Programmable Logic Controller (PLC)");
}

int PlcLinkWidget::channelDirection(const Element *slave)
{
	if (!slave) return 0;
	static const QRegularExpression out(QStringLiteral("^(DQ|DO|AQ|AO|Q|O|A)\\d*$"),
					    QRegularExpression::CaseInsensitiveOption);
	static const QRegularExpression in(QStringLiteral("^(DI|AI|I|E)\\d*$"),
					   QRegularExpression::CaseInsensitiveOption);
	for (const Terminal *t : slave->terminals()) {
		const QString n = t->baseName().trimmed();
		if (out.match(n).hasMatch()) return 2;
		if (in.match(n).hasMatch()) return 1;
	}
	return 0;
}

void PlcLinkWidget::updateUi()
{
	if (!m_element)
		return;
	const auto link = linkOf(m_element);
	if (link.first) {
		const auto &ios = link.first->elementData().plcMasterData().ios;
		const QString channel = link.second >= 0 && link.second < ios.size()
			? ios.at(link.second).address : QString();
		m_state->setText(tr("On %1, channel %2 (folio %3)")
				 .arg(link.first->actualLabel(), channel, folioOf(link.first)));
	} else {
		m_state->setText(tr("Not linked to a module yet: choose the module, then the channel."));
	}
	m_unlink_pb->setVisible(link.first != nullptr);
	m_show_module_pb->setVisible(link.first != nullptr);
	fillModules();
}

/**
	Every PLC module of the project with channels of the slave's
	direction, the one it is linked to selected.
*/
void PlcLinkWidget::fillModules()
{
	m_filling = true;
	Element *keep = currentMaster();
	const auto link = linkOf(m_element);
	if (link.first) keep = link.first;

	m_module_cb->clear();
	m_modules.clear();
	if (m_element && m_element->diagram() && m_element->diagram()->project()) {
		const int dir = m_all_cb->isChecked() ? 0 : channelDirection(m_element);
		ElementProvider ep(m_element->diagram()->project());
		QVector<QPointer<Element>> masters = ep.find(ElementData::Master);
		std::sort(masters.begin(), masters.end(), [](const QPointer<Element> &a, const QPointer<Element> &b) {
			const int fa = a && a->diagram() ? a->diagram()->folioIndex() : 0;
			const int fb = b && b->diagram() ? b->diagram()->folioIndex() : 0;
			if (fa != fb) return fa < fb;
			return QString::localeAwareCompare(a ? a->actualLabel() : QString(),
							   b ? b->actualLabel() : QString()) < 0;
		});
		for (Element *e : masters) {
			if (!isPlcModule(e)) continue;
			bool fits = dir == 0 || e == link.first;
			for (const auto &io : e->elementData().plcMasterData().ios)
				if (ioDirection(io.type) == dir) fits = true;
			if (!fits) continue;
			QString label = e->actualLabel();
			if (label.isEmpty()) label = e->name();
			const QString part = e->elementInformations()
				.value(QStringLiteral("manufacturer_reference")).toString();
			m_module_cb->addItem(part.isEmpty()
				? tr("%1 (folio %2)").arg(label, folioOf(e))
				: tr("%1  %2 (folio %3)").arg(label, part, folioOf(e)));
			m_modules << e;
		}
	}
	m_module_cb->setEnabled(!m_modules.isEmpty());
	m_hint->setText(m_modules.isEmpty()
		? tr("The project has no PLC module with channels of this kind. Place the module "
		     "(e.g. its overview or device picture from the company collection) first.")
		: QString());
	const int idx = keep ? m_modules.indexOf(keep) : -1;
	m_module_cb->setCurrentIndex(idx >= 0 ? idx : (m_modules.isEmpty() ? -1 : 0));
	m_filling = false;
	fillChannels();
}

/**
	The channels of the chosen module; those in use show who uses them and
	cannot be chosen, except the slave's own.
*/
void PlcLinkWidget::fillChannels()
{
	m_filling = true;
	auto *model = static_cast<QStandardItemModel *>(m_channel_cb->model());
	model->clear();
	Element *master = currentMaster();
	const auto link = linkOf(m_element);
	int select = 0;
	if (master) {
		auto *placeholder = new QStandardItem(tr("— choose the channel —"));
		placeholder->setData(-1, Qt::UserRole);
		model->appendRow(placeholder);

		QHash<int, Element *> used;
		for (Element *slave : master->linkedElements()) {
			const int i = master->groupIndexForElement(slave);
			if (i >= 0) used.insert(i, slave);
		}
		const int dir = m_all_cb->isChecked() ? 0 : channelDirection(m_element);
		const auto &ios = master->elementData().plcMasterData().ios;
		for (int i = 0; i < ios.size(); ++i) {
			const auto &io = ios.at(i);
			const bool own = master == link.first && i == link.second;
			if (dir != 0 && ioDirection(io.type) != dir && !own) continue;
			QString text = io.address.isEmpty() ? tr("Channel %1").arg(i + 1) : io.address;
			if (!io.terminals.isEmpty())
				text += tr("  terminals %1").arg(io.terminals.join(QStringLiteral(", ")));
			if (!io.functionText.isEmpty())
				text += QStringLiteral("  ") + io.functionText;
			auto *item = new QStandardItem(text);
			item->setData(i, Qt::UserRole);
			Element *other = used.value(i);
			if (other && other != m_element) {
				item->setText(text + tr("  (used by %1, folio %2)")
					      .arg(other->actualLabel(), folioOf(other)));
				item->setFlags(item->flags() & ~(Qt::ItemIsEnabled | Qt::ItemIsSelectable));
			}
			model->appendRow(item);
			if (own) select = model->rowCount() - 1;
		}
	}
	m_channel_cb->setEnabled(master != nullptr);
	m_channel_cb->setCurrentIndex(master ? select : -1);
	m_filling = false;
}

void PlcLinkWidget::channelChosen(int combo_index)
{
	if (m_filling || !m_element) return;
	Element *master = currentMaster();
	const int io = m_channel_cb->itemData(combo_index, Qt::UserRole).toInt();
	if (!master || io < 0) return;
	const auto link = linkOf(m_element);
	if (link.first == master && link.second == io) return;
	m_element_to_link = master;
	m_pending_io_index = io;
	apply();
}

void PlcLinkWidget::unlink()
{
	if (!m_element || m_element->isFree())
		return;

	auto *undo = new LinkElementCommand(m_element);
	// the current group index, so that makeLink() can restore the PLC
	// variables and terminal labels on undo
	const auto link = linkOf(m_element);
	if (link.second >= 0)
		undo->setGroupIndex(link.second);
	undo->unlinkAll();
	m_element->diagram()->undoStack().push(undo);
	updateUi();
}

void PlcLinkWidget::showModule()
{
	Element *master = linkOf(m_element).first;
	if (!master || !master->diagram())
		return;
	master->diagram()->showMe();
	master->setHighlighted(true);
}

Element *PlcLinkWidget::currentMaster() const
{
	const int i = m_module_cb ? m_module_cb->currentIndex() : -1;
	return i >= 0 && i < m_modules.size() ? m_modules.at(i).data() : nullptr;
}

int PlcLinkWidget::currentChannel() const
{
	return m_channel_cb ? m_channel_cb->currentData(Qt::UserRole).toInt() : -1;
}
