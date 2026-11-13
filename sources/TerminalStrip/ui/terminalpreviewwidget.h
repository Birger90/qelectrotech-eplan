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
#ifndef TERMINALPREVIEWWIDGET_H
#define TERMINALPREVIEWWIDGET_H

#include "../terminalarticlecatalog.h"

#include <QWidget>

/**
	@brief A terminal strip seen from above, at scale -- the realistic
	drawing of the terminal strip items -- for terminals not placed yet:
	the catalogue shows the article it offers, the "draw terminal strip"
	dialog the strip it is about to build.
*/
class TerminalPreviewWidget : public QWidget
{
		Q_OBJECT

	public:
		struct Terminal
		{
			QString label;
			TerminalArticleCatalog::Article article;
			bool pe = false;
				///Terminals next to each other with the same group (>= 0)
				///are the levels of one terminal; -1: a terminal with
				///as many levels as its article has
			int group = -1;
		};

		explicit TerminalPreviewWidget(QWidget *parent = nullptr);

		void setStrip(const QString &name, const QVector<Terminal> &terminals);
		QSize sizeHint() const override;
		QSize minimumSizeHint() const override;

	protected:
		void paintEvent(QPaintEvent *event) override;

	private:
		QString m_name;
		QVector<Terminal> m_terminals;
};

#endif // TERMINALPREVIEWWIDGET_H
