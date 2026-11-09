/*
	Copyright 2006-2026 The QElectroTech Team
	This file is part of QElectroTech.

	QElectroTech is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	(at your option) any later version.

	QElectroTech is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with QElectroTech. If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef PORTALSDIALOG_H
#define PORTALSDIALOG_H

#include <QDialog>
#include <QHash>
#include <QPointer>

class QCheckBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QProgressBar;
class QPushButton;
class PartFetcher;

/**
	@brief The manufacturer portals: per portal the account (log in, log
	out, status), the 3Dfindit request limit, the SiePortal links, and
	the processing of a list of missing parts.
*/
class PortalsDialog : public QDialog
{
	Q_OBJECT

	public:
		explicit PortalsDialog(QWidget *parent = nullptr);
		~PortalsDialog() override;

		/// elements written by the processing of a list
		QStringList writtenPaths() const { return m_written; }

	private:
		struct Row {
			QLabel *status = nullptr;
			QLineEdit *user = nullptr;
			QLineEdit *secret = nullptr;
			QPushButton *login = nullptr;
			QPushButton *logout = nullptr;
		};
		QWidget *portalPage(const QString &id);
		void login(const QString &id);
		void logout(const QString &id);
		void updateStatus();
		void countRequest();
		void addLinks();
		void chooseList();
		void runList();

		QHash<QString, Row> m_rows;
		QLabel *m_quota = nullptr;
		QLineEdit *m_request_order = nullptr;
		QLabel *m_links_info = nullptr;
		QLineEdit *m_list = nullptr;
		QLineEdit *m_makers = nullptr;
		QCheckBox *m_dry_run = nullptr;
		QPushButton *m_run = nullptr;
		QPushButton *m_cancel = nullptr;
		QProgressBar *m_progress = nullptr;
		QPlainTextEdit *m_log = nullptr;
		QPointer<PartFetcher> m_fetcher;
		QStringList m_written;
};

#endif // PORTALSDIALOG_H
