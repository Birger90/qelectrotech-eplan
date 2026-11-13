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
	@brief The proposal shown once a terminal line has been drawn: one row
	per wire, with the strip, the number, the level and the article its
	terminal gets -- all filled in already, so a plain Enter is enough.

	Every row can go its own way: a wire moved to another strip ("N on
	X2, PE on X3") is numbered there, a terminal of several levels takes
	the wires of one strip in turn, PE on its PE level when it has one.
	Rows of one strip with the same number are one terminal.
*/
class DrawnTerminalsDialog : public QDialog
{
		Q_OBJECT

	public:
		DrawnTerminalsDialog(QETProject *project,
							 const QList<DrawnTerminals::Row> &rows,
							 QWidget *parent = nullptr);
		~DrawnTerminalsDialog() override;

		QList<DrawnTerminals::Row> rows() const;
		DrawnTerminals::Settings settings() const;

	public slots:
		void accept() override;

	private:
		void stripChanged();
		void rebuild();
		void assignLevels();
		void fillTable();
		void itemChanged(int row, int column);
		void chooseElement();
		void showElement();
		void chooseArticle(bool pe);
		void chooseRowArticle();
		void ownStripForSelection();
		void resetRows();
		void updatePreview();
		void articleChanged();
		void currentRowChanged();
		QList<int> selectedRows() const;
		QStringList knownStrips() const;

	private:
		QETProject *m_project = nullptr;
		QList<DrawnTerminals::Row> m_rows;
			///What the user set for a row himself; the rest follows the
			///settings at the top
		QVector<bool> m_own_strip, m_own_article, m_own_number;
		QComboBox *m_strip = nullptr;
		QSpinBox *m_first = nullptr;
		QLabel *m_element_label = nullptr;
		QString m_element_path;
		QLineEdit *m_article = nullptr;
		QLineEdit *m_pe_article = nullptr;
		QTableWidget *m_table = nullptr;
		TerminalPreviewWidget *m_preview = nullptr;
		QLabel *m_preview_label = nullptr;
		QWidget *m_pe_catalog = nullptr;
		QLabel *m_levels_hint = nullptr;
		QString m_saved_pe_article;	///< while a terminal with a PE level is chosen
		bool m_filling = false;
};

#endif // DRAWNTERMINALSDIALOG_H
