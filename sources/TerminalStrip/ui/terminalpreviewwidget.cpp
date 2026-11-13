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
#include "terminalpreviewwidget.h"

#include "../GraphicsItem/properties/terminalstriplayoutpattern.h"
#include "../GraphicsItem/terminalstripdrawer.h"
#include "../terminalcolor.h"

#include <QPainter>
#include <algorithm>

namespace
{
	using namespace TerminalStripDrawer;

	class PreviewRealTerminal : public AbstractRealTerminalInterface
	{
		public:
			PreviewRealTerminal(const TerminalPreviewWidget::Terminal &t) :
				m_t(t)
			{
				QString spec_ = TerminalArticleCatalog::colorSpec(t.article);
				if (spec_.isEmpty()) {
					spec_ = t.pe ? QStringLiteral("Green/Yellow") : QStringLiteral("Gray");
				}
				TerminalColor::parse(spec_, &m_color, &m_stripe);
				m_entry = TerminalArticleCatalog::drawingCatalogEntry(t.article);
			}
			QString label() const override { return m_t.label; }
			bool isBridged() const override { return false; }
			AbstractBridgeInterface *bridge() const override { return nullptr; }
			QString xref() const override { return QString(); }
			ElementData::TerminalType type() const override {
				return m_t.pe ? ElementData::TTGround : ElementData::TTGeneric;
			}
			QColor color() const override { return m_color; }
			QColor stripeColor() const override { return m_stripe; }
			qreal widthMm() const override {
				return m_t.article.width > 0 ? m_t.article.width : 5.2;
			}
			QString catalogEntry() const override { return m_entry; }

		private:
			TerminalPreviewWidget::Terminal m_t;
			QColor m_color, m_stripe;
			QString m_entry;
	};

	class PreviewPhysicalTerminal : public AbstractPhysicalTerminalInterface
	{
		public:
				///The levels of one terminal, one entry each
			explicit PreviewPhysicalTerminal(const QVector<TerminalPreviewWidget::Terminal> &levels)
			{
				for (const auto &t_ : levels) {
					m_real << QSharedPointer<AbstractRealTerminalInterface>(new PreviewRealTerminal(t_));
				}
			}
			QVector<QSharedPointer<AbstractRealTerminalInterface>> realTerminals() const override {
				return m_real;
			}
		private:
			QVector<QSharedPointer<AbstractRealTerminalInterface>> m_real;
	};

	class PreviewStrip : public AbstractTerminalStripInterface
	{
		public:
			PreviewStrip(const QString &name, const QVector<TerminalPreviewWidget::Terminal> &terminals) :
				m_name(name)
			{
				for (int i = 0; i < terminals.size(); )
				{
					const auto &t_ = terminals.at(i);
					QVector<TerminalPreviewWidget::Terminal> levels_;
					if (t_.group < 0)
					{
						for (int l = 0; l < qMax(1, t_.article.levels); ++l) {
							levels_ << t_;
						}
						++i;
					}
					else
					{
						while (i < terminals.size() && terminals.at(i).group == t_.group) {
							levels_ << terminals.at(i++);
						}
					}
					m_physical << QSharedPointer<AbstractPhysicalTerminalInterface>(new PreviewPhysicalTerminal(levels_));
				}
			}
			QString installation() const override { return QString(); }
			QString location() const override { return QString(); }
			QString name() const override { return m_name; }
			QVector<QSharedPointer<AbstractPhysicalTerminalInterface>> physicalTerminal() const override {
				return m_physical;
			}
		private:
			QString m_name;
			QVector<QSharedPointer<AbstractPhysicalTerminalInterface>> m_physical;
	};
}

TerminalPreviewWidget::TerminalPreviewWidget(QWidget *parent) :
	QWidget(parent)
{
	setAutoFillBackground(false);
}

void TerminalPreviewWidget::setStrip(const QString &name, const QVector<Terminal> &terminals)
{
	m_name = name;
	m_terminals = terminals;
	update();
}

QSize TerminalPreviewWidget::sizeHint() const
{
	return QSize(420, 180);
}

QSize TerminalPreviewWidget::minimumSizeHint() const
{
	return QSize(200, 120);
}

void TerminalPreviewWidget::paintEvent(QPaintEvent *)
{
	QPainter painter_(this);
	painter_.setRenderHint(QPainter::Antialiasing);
		//Printed output: black on a white page, whatever the theme
	painter_.fillRect(rect(), Qt::white);
	painter_.setPen(QColor(0xc8, 0xc8, 0xc8));
	painter_.drawRect(rect().adjusted(0, 0, -1, -1));

	if (m_terminals.isEmpty())
	{
		painter_.setPen(Qt::darkGray);
		painter_.drawText(rect(), Qt::AlignCenter, tr("No terminal"));
		return;
	}

	const bool all_screw_ = std::all_of(m_terminals.begin(), m_terminals.end(), [](const Terminal &t) {
		return t.article.isValid() && t.article.clamp == TerminalArticleCatalog::Clamp::Screw;
	});

	auto pattern_ = QSharedPointer<TerminalStripLayoutPattern>::create();
	TerminalStripDrawer::TerminalStripDrawer drawer_(
		QSharedPointer<AbstractTerminalStripInterface>(new PreviewStrip(m_name, m_terminals)),
		pattern_);
	drawer_.setDrawStyle(all_screw_ ? TerminalStripLayoutPattern::DrawStyle::RealisticScrew
									: TerminalStripLayoutPattern::DrawStyle::RealisticPushIn);

	const QRectF bounds_ = drawer_.boundingRect();
	if (bounds_.isEmpty()) {
		return;
	}
	const QRectF target_ = QRectF(rect()).adjusted(10, 10, -10, -10);
		//Never bigger than twice the normal scale: three terminals must
		//not fill the window like a photograph
	const qreal scale_ = qMin(2.0, qMin(target_.width() / bounds_.width(),
										target_.height() / bounds_.height()));
	painter_.translate(target_.center());
	painter_.scale(scale_, scale_);
	painter_.translate(-bounds_.center());
	drawer_.paint(&painter_);
}
