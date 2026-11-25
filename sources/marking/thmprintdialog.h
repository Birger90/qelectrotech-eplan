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
#ifndef THMPRINTDIALOG_H
#define THMPRINTDIALOG_H

#include "markingexport.h"
#include "thmcatalog.h"
#include "thmclient.h"
#include "thmjob.h"
#include "thmlayouts.h"
#include "thmprinters.h"

#include <QDialog>

class QETProject;
class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QSpinBox;
class QTableWidget;

/**
	@brief Project > Print THM labels...: the markers of the open project
	printed straight on a THM printer, without the maker's software.

	The markers are chosen as in the marking export, equal texts merged
	and copies per marker; the article chosen picks the printer loaded
	with it. Status and material can be asked before; nothing is printed
	before a summary of target, article and quantity has been confirmed.
*/
class ThmPrintDialog : public QDialog
{
	Q_OBJECT
	public:
		explicit ThmPrintDialog(QETProject *project, QWidget *parent = nullptr);

		MarkingExport::Options options() const;
		QList<ThmJob::Label> labels() const;

	private:
		void loadCatalog();
		void chooseCatalog();
		void loadLayouts();
		void chooseLayouts();
			/// The profile of a job on the article chosen, for @p printer
		bool jobProfile(const ThmPrinter *printer, ThmJob::Profile *profile,
						QString *source = nullptr, QString *error = nullptr) const;
		QString currentArticle() const;
			/// Material change: the user loads the article chosen on a
			/// printer and confirms it
		void changeMaterial();
		void fillArticles();
		void updatePreview();
		void updateTarget();
		void managePrinters();
		const ThmPrinter *currentPrinter() const;
		void query(const QList<ThmClient::Request> &requests);
		void onReply(const ThmClient::Reply &reply);
		void cancelJob();
		void saveJob();
		void print();
		void remember() const;
		void setBusy(bool busy);

		QETProject *m_project = nullptr;
		QList<ThmPrinter> m_printers;
		QList<ThmCatalog::Variant> m_catalog;
		QString m_catalog_error;
		QList<ThmLayouts::Variant> m_layouts;
		QString m_layouts_error;
		ThmClient *m_client = nullptr;
		QList<ThmClient::Request> m_queue;
		QStringList m_lines;           ///< the replies of the last query
		ThmJob::Material m_material;   ///< last ESC l reply
		QString m_material_printer;    ///< id of the printer it came from

		QCheckBox *m_wires = nullptr;
		QCheckBox *m_cables = nullptr;
		QCheckBox *m_terminals = nullptr;
		QCheckBox *m_devices = nullptr;
		QSpinBox *m_folio_from = nullptr;
		QSpinBox *m_folio_to = nullptr;
		QCheckBox *m_full = nullptr;
		QCheckBox *m_both_ends = nullptr;
		QCheckBox *m_merge = nullptr;
		QSpinBox *m_copies = nullptr;
		QTableWidget *m_preview = nullptr;
		QLabel *m_count = nullptr;

		QComboBox *m_article = nullptr;
		QLabel *m_catalog_label = nullptr;
		QLabel *m_layouts_label = nullptr;
		QComboBox *m_printer = nullptr;
		QLabel *m_target_message = nullptr;
		QLabel *m_profile_source = nullptr;
		QLabel *m_device_state = nullptr;
		QList<QPushButton *> m_device_buttons;
		QPushButton *m_print = nullptr;
		QPushButton *m_save = nullptr;
};

/**
	@brief The THM printers: address, the article variant loaded and the
	print profile of that material, kept in the settings
	(ThmPrinters). Nothing is filled in for the user: no address and no
	label size is built in.
*/
class ThmPrintersDialog : public QDialog
{
	Q_OBJECT
	public:
		ThmPrintersDialog(const QList<ThmPrinter> &printers,
						  const QList<ThmCatalog::Variant> &catalog,
						  QWidget *parent = nullptr,
						  const QList<ThmLayouts::Variant> &layouts = {});
		QList<ThmPrinter> printers() const { return m_printers; }

	private:
		void showPrinter(int row);
		void store();
		void add();
		void removeCurrent();
		void accept() override;

		QString nameOf(const QString &material_no) const;
		QList<ThmPrinter> m_printers;
		QList<ThmCatalog::Variant> m_catalog;
		QList<ThmLayouts::Variant> m_layouts;
		int m_row = -1;
		bool m_showing = false;

		QListWidget *m_list = nullptr;
		QWidget *m_form = nullptr;
		QLabel *m_id = nullptr;
		QLineEdit *m_name = nullptr;
		QLineEdit *m_model = nullptr;
		QLineEdit *m_host = nullptr;
		QSpinBox *m_port = nullptr;
		QComboBox *m_transport = nullptr;
		QComboBox *m_material = nullptr;
		QComboBox *m_usual = nullptr;
		QLineEdit *m_roller = nullptr;
		QLabel *m_loaded_at = nullptr;
		QDoubleSpinBox *m_width = nullptr;
		QDoubleSpinBox *m_height = nullptr;
		QDoubleSpinBox *m_gap = nullptr;
		QComboBox *m_sensor = nullptr;
		QSpinBox *m_columns = nullptr;
		QDoubleSpinBox *m_pitch = nullptr;
		QDoubleSpinBox *m_margin_left = nullptr;
		QDoubleSpinBox *m_margin_right = nullptr;
		QDoubleSpinBox *m_offset_x = nullptr;
		QDoubleSpinBox *m_offset_y = nullptr;
		QSpinBox *m_font = nullptr;
		QDoubleSpinBox *m_text_size = nullptr;
		QDoubleSpinBox *m_baseline = nullptr;
		QCheckBox *m_set_heat = nullptr;
		QSpinBox *m_speed = nullptr;
		QSpinBox *m_heat = nullptr;
		QCheckBox *m_transfer = nullptr;
		QCheckBox *m_rotate = nullptr;
};

#endif // THMPRINTDIALOG_H
