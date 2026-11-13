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
#ifndef DRAWNTERMINALSDIALOG_H
#define DRAWNTERMINALSDIALOG_H

#include "../drawnterminals.h"

#include <QDialog>

class QComboBox;
class QLabel;
class QLineEdit;
class QSpinBox;
class QTableWidget;
class TerminalPreviewWidget;
class QETProject;

/**
	@brief The proposal shown once a terminal line has been drawn: which
	strip, which numbers, which symbol and article -- all filled in
	already, so a plain Enter is enough, and every value can still be
	changed before anything is put on the folio.
*/
class DrawnTerminalsDialog : public QDialog
{
		Q_OBJECT

	public:
		DrawnTerminalsDialog(QETProject *project,
							 const QList<DrawnTerminals::Row> &rows,
							 QWidget *parent = nullptr);

		QList<DrawnTerminals::Row> rows() const;
		DrawnTerminals::Settings settings() const;

	public slots:
		void accept() override;

	private:
		void stripChanged();
		void renumber();
		void chooseElement();
		void showElement();
		void chooseArticle(bool pe);
		void updatePreview();
		void articleChanged();
		int levels() const;

	private:
		QETProject *m_project = nullptr;
		QList<DrawnTerminals::Row> m_rows;
		QComboBox *m_strip = nullptr;
		QSpinBox *m_first = nullptr;
		QLabel *m_element_label = nullptr;
		QString m_element_path;
		QLineEdit *m_article = nullptr;
		QLineEdit *m_pe_article = nullptr;
		QTableWidget *m_table = nullptr;
		TerminalPreviewWidget *m_preview = nullptr;
		QWidget *m_pe_catalog = nullptr;
		QLabel *m_levels_hint = nullptr;
		QString m_saved_pe_article;	///< while a terminal of several levels is chosen
};

#endif // DRAWNTERMINALSDIALOG_H
