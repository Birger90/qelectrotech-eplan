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
#include "purchaseexport.h"

#include "purchasefetch.h"
#include "purchaselist.h"
#include "../qetapp.h"
#include "../qetproject.h"

#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QLocale>
#include <QMessageBox>
#include <QProgressDialog>
#include <QPushButton>
#include <QSettings>
#include <QUrl>

void PurchaseExport::run(QETProject *project, QWidget *parent)
{
	if (!project) return;

	QSettings settings;
	const QString last_suffix = settings.value(QStringLiteral("purchase/suffix"),
											   QStringLiteral("xlsx")).toString();
	QString dir = project->currentDir();
	if (dir.isEmpty()) dir = QETApp::documentDir();
	QString title = project->title().trimmed();
	if (title.isEmpty()) title = QFileInfo(project->filePath()).completeBaseName();
	const QString proposed = QDir(dir).filePath(
		tr("purchase list") + QLatin1Char('_') + title + QLatin1Char('.') + last_suffix);
	const QString excel = tr("Excel workbook (*.xlsx)");
	const QString csv = tr("CSV file (*.csv)");
	QString filter = last_suffix == QLatin1String("csv") ? csv : excel;
	QString path = QFileDialog::getSaveFileName(parent, tr("Export purchase list"), proposed,
												excel + QStringLiteral(";;") + csv, &filter);
	if (path.isEmpty()) return;
	if (QFileInfo(path).suffix().isEmpty())
		path += filter == csv ? QStringLiteral(".csv") : QStringLiteral(".xlsx");
	settings.setValue(QStringLiteral("purchase/suffix"), QFileInfo(path).suffix().toLower());

		//The price list the company fills in: there from the first export
		//on, so it can be found
	QString error;
	PurchaseList::ensurePriceList(&error);

	QList<PurchaseList::Item> items = PurchaseList::collect(project, &error);
	if (!error.isEmpty()) {
		QMessageBox::critical(parent, tr("Export purchase list"),
							  tr("The parts of the project cannot be read.\n%1").arg(error));
		return;
	}

	QProgressDialog progress(tr("Looking up article data..."), tr("Skip"), 0, 0, parent);
	progress.setWindowModality(Qt::WindowModal);
	progress.setMinimumDuration(500);
	const PurchaseFetch::Result fetched = PurchaseFetch::fetch(items, true,
		[&](int done, int total, const QString &part) {
			progress.setMaximum(total);
			progress.setValue(done);
			if (!part.isEmpty())
				progress.setLabelText(tr("Looking up article data: %1").arg(part));
			QCoreApplication::processEvents();
			return !progress.wasCanceled();
		});
	progress.close();
	if (fetched.from_portal + fetched.from_edz + fetched.not_found > 0)
		items = PurchaseList::collect(project, &error);

	QStringList written;
	if (!PurchaseList::write(items, path, true, &written, &error)) {
		QMessageBox::critical(parent, tr("Export purchase list"),
							  tr("Unable to save the purchase list to %1.\n%2")
								  .arg(QDir::toNativeSeparators(path), error));
		return;
	}

	const PurchaseList::Summary s = PurchaseList::summarize(items);
	QString text = tr("%n article(s) written, grouped by supplier.", "", s.articles);
	text += QLatin1Char('\n') + tr("With price: %1, with EAN/GTIN: %2.").arg(s.priced).arg(s.with_gtin);
	if (s.priced > 0)
		text += QLatin1Char('\n') + tr("Total of the known prices: %1 %2")
				.arg(QLocale().toString(s.total, 'f', 2), s.currencies.join(QLatin1Char('/')));
	if (s.open > 0)
		text += QLatin1Char('\n') + tr("%n device(s) without article (\"Article open\").", "", s.open);
	if (!fetched.error.isEmpty())
		text += QStringLiteral("\n\n") + tr("The Data Portal could not be asked: %1").arg(fetched.error);
	QStringList files;
	for (const QString &f : std::as_const(written)) files << QDir::toNativeSeparators(f);
	text += QStringLiteral("\n\n") + files.join(QLatin1Char('\n'));
	const QString price_list = PurchaseList::priceListPath();
	if (!price_list.isEmpty())
		text += QStringLiteral("\n\n") + tr("Own prices, delivery times and suppliers: %1")
				.arg(QDir::toNativeSeparators(price_list));

	QMessageBox box(QMessageBox::Information, tr("Export purchase list"), text,
					QMessageBox::Close, parent);
	QPushButton *open = box.addButton(tr("Open list"), QMessageBox::AcceptRole);
	QPushButton *prices = price_list.isEmpty()
			? nullptr : box.addButton(tr("Open price list"), QMessageBox::ActionRole);
	box.setDefaultButton(open);
	box.exec();
	if (box.clickedButton() == open)
		QDesktopServices::openUrl(QUrl::fromLocalFile(path));
	else if (prices && box.clickedButton() == prices)
		QDesktopServices::openUrl(QUrl::fromLocalFile(price_list));
}
