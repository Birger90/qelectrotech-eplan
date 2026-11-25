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
#ifndef THMMATERIALDIALOG_H
#define THMMATERIALDIALOG_H

#include "thmmaterials.h"

#include <QDialog>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QTableWidget;

/**
	@brief Choose a material as in M-Print PRO: the article variants in a
	table, filtered by text, category, size, colour and sides, only the
	printable ones or only those loaded on a printer. A double click or OK
	takes the chosen one.
*/
class ThmMaterialDialog : public QDialog
{
	Q_OBJECT
	public:
		ThmMaterialDialog(const QList<ThmMaterials::Entry> &entries,
						  const QString &current, QWidget *parent = nullptr);
			/// The article number chosen, empty if none
		QString article() const;
		QString name() const;

	private:
		void fill();
		const ThmMaterials::Entry *chosen() const;

		QList<ThmMaterials::Entry> m_entries;
		QLineEdit *m_text = nullptr;
		QComboBox *m_category = nullptr;
		QComboBox *m_size = nullptr;
		QComboBox *m_colour = nullptr;
		QComboBox *m_sides = nullptr;
		QCheckBox *m_printable = nullptr;
		QCheckBox *m_loaded = nullptr;
		QTableWidget *m_table = nullptr;
		QLabel *m_count = nullptr;
};

#endif // THMMATERIALDIALOG_H
