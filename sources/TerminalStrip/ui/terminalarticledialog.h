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
#ifndef TERMINALARTICLEDIALOG_H
#define TERMINALARTICLEDIALOG_H

#include "../terminalarticlecatalog.h"
#include <import/eplandxf/dataportalclient.h>

#include <QDialog>

class QComboBox;
class QPushButton;
class QTimer;
class QLabel;
class QLineEdit;
class QTableWidget;
class TerminalPreviewWidget;

/**
	@brief The terminal catalogue: every terminal block one can order,
	filtered by manufacturer, kind, cross-section and clamp or by any
	text, with a drawing at scale of the one selected.
*/
class TerminalArticleDialog : public QDialog
{
		Q_OBJECT

	public:
			///Let the user pick an article; invalid when he cancels
		static TerminalArticleCatalog::Article choose(QWidget *parent,
													  const QString &current_article,
													  bool protective_earth);

		explicit TerminalArticleDialog(QWidget *parent = nullptr);

		void preselect(const QString &article_text, bool protective_earth);
		TerminalArticleCatalog::Article selectedArticle() const;

	private:
		void fillFilters();
		void fillTable();
		void applyFilter();
		void selectionChanged();
		void openCatalogFolder();
		void searchOnline();
		void onlineResults(const QVector<DataPortalPart> &parts);
		void lookUpPicture();
		void showPicture(const QString &url, const QByteArray &data);

	private:
		QVector<TerminalArticleCatalog::Article> m_articles;
		DataPortalClient *m_client = nullptr;			///< online search
		DataPortalClient *m_picture_client = nullptr;	///< pictures of the selected terminal
		QTimer *m_picture_timer = nullptr;
		QLabel *m_picture = nullptr;
		QPushButton *m_online = nullptr;
		QString m_picture_url;	///< the picture the label should show
		QLineEdit *m_search = nullptr;
		QComboBox *m_manufacturer = nullptr;
		QComboBox *m_kind = nullptr;
		QComboBox *m_section = nullptr;
		QComboBox *m_clamp = nullptr;
		QTableWidget *m_table = nullptr;
		TerminalPreviewWidget *m_preview = nullptr;
		QLabel *m_details = nullptr;
		QLabel *m_count = nullptr;
};

#endif // TERMINALARTICLEDIALOG_H
