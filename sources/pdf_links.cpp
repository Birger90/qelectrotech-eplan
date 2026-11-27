/*
	Copyright 2006-2025 The QElectroTech Team
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
#include "pdf_links.h"

#include "cable/cable.h"
#include "cable/cablepart.h"
#include "diagram.h"
#include "qetgraphicsitem/crossrefitem.h"
#include "qetgraphicsitem/dynamicelementtextitem.h"
#include "qetgraphicsitem/element.h"
#include "qetgraphicsitem/elementtextitemgroup.h"
#include "qetgraphicsitem/independenttextitem.h"
#include "qetproject.h"
#include "TerminalStrip/GraphicsItem/terminalstripitem.h"

// Private Qt PDF engine for drawHyperlink() — not public API.
// Availability of Qt::GuiPrivate is verified at configure time in CMakeLists.txt.
#include <private/qpdf_p.h>

#include <QByteArray>
#include <QCryptographicHash>
#include <QFile>
#include <QGraphicsTextItem>
#include <QHash>
#include <QList>
#include <QRegularExpression>
#include <QUrl>
#include <QVector>

#include <algorithm>

namespace PdfLinks {

void injectCrossRefLinks(QPdfEngine *engine, Diagram *diagram,
						 const PageGeometry &geom,
						 const QMap<Diagram *, int> &pageMap,
						 const QString &outputFileName)
{
	if (!engine || !diagram)
		return;

	const QTransform &fit       = geom.sceneToDevice;
	const QRectF      &target   = geom.target;
	const QRectF      &pageBounds = geom.pageBounds;

	// Compute, in PDF points on its OWN page, the rectangle to frame for a
	// target (used as a /FitR destination so the link zooms onto it).
	// The target is given as a rectangle in the scene of @p dg, since what
	// is pointed at is not always an element: a cable section is a plain
	// drawn line somewhere on another folio.
	// A rectangle in the scene of @p dg, in PDF points on dg's own page.
	auto sceneToPdf = [&](Diagram *dg, const QRectF &sceneRect) -> QRectF {
		if (!dg || sceneRect.width() <= 0.0 || sceneRect.height() <= 0.0)
			return QRectF();
		const QRectF srcT = geom.sourceRectOf(dg);
		if (srcT.width() <= 0.0 || srcT.height() <= 0.0) return QRectF();
		const qreal sT = qMin(target.width()  / srcT.width(),
							  target.height() / srcT.height());
		QTransform fitT;
		fitT.translate(target.x(), target.y());
		fitT.scale(sT, sT);
		fitT.translate(-srcT.x(), -srcT.y());

		const QRectF devT = fitT.mapRect(sceneRect);
		const QPointF a = geom.devToPdf(devT.topLeft());
		const QPointF b = geom.devToPdf(devT.bottomRight());
		return QRectF(QPointF(qMin(a.x(), b.x()), qMin(a.y(), b.y())),
					  QPointF(qMax(a.x(), b.x()), qMax(a.y(), b.y())));
	};

	auto frameRectPdf = [&](Diagram *dg, const QRectF &targetScene) -> QRectF {
		if (targetScene.width() <= 0.0 || targetScene.height() <= 0.0)
			return QRectF();
		// Frame the target with a little context, and enforce a minimum
		// framed size so tiny contacts don't zoom in extremely.
		QRectF framed = targetScene;
		const qreal pad = 25.0;
		framed.adjust(-pad, -pad, pad, pad);
		const qreal minSide = 160.0;
		if (framed.width()  < minSide)
			framed.adjust(-(minSide - framed.width())  / 2.0, 0,
						  (minSide - framed.width())  / 2.0, 0);
		if (framed.height() < minSide)
			framed.adjust(0, -(minSide - framed.height()) / 2.0,
						  0,  (minSide - framed.height()) / 2.0);
		return sceneToPdf(dg, framed);
	};

	// The red frame which marks the target after the jump (EPLAN 2.9 and
	// later): the target itself, with a few pixels to spare.
	auto markRectPdf = [&](Diagram *dg, const QRectF &targetScene) -> QRectF {
		return sceneToPdf(dg, targetScene.adjusted(-4.0, -4.0, 4.0, 4.0));
	};

	// The links are collected first and drawn sorted, not in the order the
	// loop below finds them: a cross-reference's contacts are keyed by
	// Element pointer and the scene's items come in stacking order, so that
	// order changes from run to run and the same project gave a different
	// PDF each time it was exported.
	struct Link { QRectF rect; QUrl url; };
	QList<Link> links;

	// One clickable link: @p sceneRect on THIS folio leads to
	// @p targetDiagram, framed around @p targetScene over there. With
	// @p mark, the target also gets the red frame of the jump (not when
	// the target is a whole folio).
	auto injectLinkTo = [&](const QRectF &sceneRect, Diagram *targetDiagram,
							const QRectF &targetScene, bool mark = true) {
		if (!targetDiagram) return;
		const int targetPage = pageMap.value(targetDiagram, -1);
		if (targetPage < 1) return;
		const QRectF devRect = fit.mapRect(sceneRect);
		if (!devRect.isValid() || !pageBounds.intersects(devRect)) return;

		QString frag = QString("page=%1").arg(targetPage);
		const QRectF d = frameRectPdf(targetDiagram, targetScene);   // /FitR L_B_R_T
		if (d.isValid())
			frag += QString("&fitr=%1_%2_%3_%4")
				.arg(qRound(d.left())).arg(qRound(d.top()))
				.arg(qRound(d.right())).arg(qRound(d.bottom()));
		const QRectF m = mark ? markRectPdf(targetDiagram, targetScene) : QRectF();
		if (m.isValid())                                             // frame L_B_R_T
			frag += QString("&mark=%1_%2_%3_%4")
				.arg(qRound(m.left())).arg(qRound(m.top()))
				.arg(qRound(m.right())).arg(qRound(m.bottom()));

		QUrl url = QUrl::fromLocalFile(outputFileName);
		url.setFragment(frag);
		links.append({devRect, url});
	};

	auto injectLink = [&](const QRectF &sceneRect, Element *targetElmt) {
		if (!targetElmt || !targetElmt->diagram()) return;
		injectLinkTo(sceneRect, targetElmt->diagram(),
					 targetElmt->mapRectToScene(targetElmt->boundingRect()));
	};

	for (auto *item : diagram->items()) {

		// --- An element with a destination (EPLAN: device <-> cabinet
		// layout): the element itself becomes the link ---
		if (auto *elmt = dynamic_cast<Element*>(item)) {
				// a linked folio report: the arrow itself leads to its other
				// end too, not only its label (as in EPLAN)
			if (Element *other = elmt->jumpTarget()) {
				injectLink(elmt->mapRectToScene(elmt->boundingRect()), other);
				continue;
			}
				// EPLAN's three-way jump: left third forward in the jump
				// sequence of the device, middle to the main function, right
				// third backward. Without a sequence the main target takes
				// the whole element.
			const QRectF r = elmt->boundingRect();
			const QUuid next = elmt->jumpUuid(QStringLiteral("next"));
			const QUuid prev = elmt->jumpUuid(QStringLiteral("prev"));
			const QUuid main = elmt->jumpUuid(QStringLiteral("main"));
			auto link = [&](const QRectF &local, const QUuid &u) {
				if (u.isNull()) return;
				const Element::JumpFound f = Element::findJump(diagram->project(), u);
				if (f.diagram && f.element != elmt)
					injectLinkTo(elmt->mapRectToScene(local), f.diagram, f.sceneRect,
								 f.element || f.text);
			};
			if (next.isNull() && prev.isNull()) {
				link(r, main);
			} else {
				const qreal w = r.width() / 3.0;
				link(QRectF(r.left(), r.top(), w, r.height()), next);
				link(QRectF(r.left() + w, r.top(), w, r.height()), main.isNull() ? next : main);
				link(QRectF(r.left() + 2 * w, r.top(), w, r.height()), prev);
			}
			continue;
		}

		// --- CrossRefItem links ---
		if (auto *xref = dynamic_cast<CrossRefItem*>(item)) {
			for (auto it = xref->hoveredContactsMap().begin();
				 it != xref->hoveredContactsMap().end(); ++it)
			{
				Element *targetElmt = it.key();
				if (!targetElmt || !targetElmt->diagram()) continue;
				// it.value() is in the CrossRefItem's LOCAL coords -> scene
				injectLink(xref->mapRectToScene(it.value()), targetElmt);
			}
			continue;
		}

		// --- Cable cross-reference links (the lines under a cable which
		// name the folio it goes on to) ---
		if (auto *cable = dynamic_cast<CablePart*>(item)) {
			for (const CablePart::PdfRef &ref : cable->pdfRefs()) {
				injectLinkTo(ref.here, ref.diagram, ref.there);
			}
				// the label of a cable with a destination leads to that folio
				// (EPLAN: cable -> its row of the cable overview)
			if (cable->cable() && !cable->cable()->destination().isNull() && diagram->project()) {
				const auto folios = diagram->project()->diagrams();
				for (Diagram *folio : folios) {
					if (folio->uuid() == cable->cable()->destination()) {
						injectLinkTo(cable->labelSceneRect(), folio,
									 folio->border_and_titleblock.insideBorderRect(), false);
						break;
					}
				}
			}
			continue;
		}

		// --- Terminal strip plan: the xref of each terminal leads to its element ---
		if (auto *strip_item = dynamic_cast<TerminalStripItem*>(item)) {
			for (const TerminalStripItem::PdfRef &ref : strip_item->pdfRefs()) {
				injectLinkTo(ref.here, ref.diagram, ref.there);
			}
			continue;
		}

		// --- A free text which leads to a folio (a reference such as /12.3) ---
		if (auto *text = dynamic_cast<IndependentTextItem*>(item)) {
			if (text->destination().isNull() || !diagram->project()) continue;
			bool found = false;
			const auto folios = diagram->project()->diagrams();
			for (Diagram *folio : folios) {
				if (folio->uuid() != text->destination()) continue;
				injectLinkTo(text->mapRectToScene(text->boundingRect()), folio,
							 folio->border_and_titleblock.insideBorderRect(), false);
				found = true;
				break;
			}
				// ...or to an element: the link frames that element
			if (!found)
				injectLink(text->mapRectToScene(text->boundingRect()), text->destinationElement());
			continue;
		}

		// --- Folio report links (DynamicElementTextItem) ---
		if (auto *deti = dynamic_cast<DynamicElementTextItem*>(item)) {
			Element *parent = deti->parentElement();
			if (!parent) continue;

			// (a) Report element : label -> linked report on another folio
			if (parent->linkType() & Element::AllReport) {
				if (parent->linkedElements().isEmpty()) continue;

				bool showsLabel =
					(deti->textFrom() == DynamicElementTextItem::ElementInfo
					 && deti->infoName() == QLatin1String("label")) ||
					(deti->textFrom() == DynamicElementTextItem::CompositeText
					 && deti->compositeText().contains(QStringLiteral("%{label}")));
				if (!showsLabel) continue;

				Element *targetElmt = parent->linkedElements().first();
				if (!targetElmt || !targetElmt->diagram()) continue;

				injectLink(deti->mapRectToScene(deti->boundingRect()), targetElmt);
				continue;
			}

			// (b) Slave element : the "(folio-pos)" text -> master element
			if (parent->linkType() == Element::Slave) {
				QGraphicsTextItem *sx = deti->slaveXrefItem();
				Element *master = deti->masterElement();
				if (sx && master && master->diagram()) {
					injectLink(sx->mapRectToScene(sx->boundingRect()), master);
				}
				continue;
			}
			continue;
		}

		// --- Slave cross-reference carried by a grouped text ---
		if (auto *grp = dynamic_cast<ElementTextItemGroup*>(item)) {
			Element *parent = grp->parentElement();
			if (!parent || parent->linkType() != Element::Slave) continue;
			if (parent->linkedElements().isEmpty()) continue;
			QGraphicsTextItem *sx = grp->slaveXrefItem();
			if (!sx) continue;
			Element *master = parent->linkedElements().first();
			if (!master || !master->diagram()) continue;
			injectLink(sx->mapRectToScene(sx->boundingRect()), master);
			continue;
		}
	}

	// Top to bottom, then left to right; the target breaks a tie between
	// two links on the same spot.
	std::sort(links.begin(), links.end(), [](const Link &a, const Link &b) {
		if (a.rect.top() != b.rect.top())
			return a.rect.top() < b.rect.top();
		if (a.rect.left() != b.rect.left())
			return a.rect.left() < b.rect.left();
		if (a.rect.bottom() != b.rect.bottom())
			return a.rect.bottom() < b.rect.bottom();
		if (a.rect.right() != b.rect.right())
			return a.rect.right() < b.rect.right();
		return a.url.fragment() < b.url.fragment();
	});
	for (const Link &link : std::as_const(links))
		engine->drawHyperlink(link.rect, link.url);
}

/// The trailer dictionary of the xref table at \a xrefStart, from
/// "trailer" up to its startxref, which the caller writes again with the
/// new offset. Copying the old startxref too left a second one in the
/// file, with the size the file had before its links were rewritten, so
/// the bytes depended on the output path the links carry (#1178).
static QByteArray trailerDictionary(const QByteArray &data, int xrefStart)
{
	const int start = data.indexOf("trailer", xrefStart);
	if (start == -1)
		return "trailer\n<<>>";
	const int end = data.indexOf("startxref", start);
	if (end == -1)
		return "trailer\n<<>>";
	return data.mid(start, end - start).trimmed();
}

/// The highest object number "N 0 obj" in \a data.
static int highestObjectNumber(const QByteArray &data)
{
	int highest = 0;
	const QByteArray objMarker(" 0 obj");
	int p = 0;
	while ((p = data.indexOf(objMarker, p)) != -1) {
		int numStart = p - 1;
		while (numStart > 0 && data[numStart - 1] != '\n' && data[numStart - 1] != '\r')
			--numStart;
		bool ok = false;
		const int num = data.mid(numStart, p - numStart).trimmed().toInt(&ok);
		if (ok && num > highest)
			highest = num;
		++p;
	}
	return highest;
}

/// Where the object "\a num 0 obj" starts in \a data, -1 if absent.
static int objectStart(const QByteArray &data, int num)
{
	const QByteArray head = QByteArray::number(num) + " 0 obj";
	if (data.startsWith(head)) return 0;
	const int p = data.indexOf("\n" + head);
	return p == -1 ? -1 : p + 1;
}

/// \a text as a PDF literal string, with its brackets and backslashes
/// escaped.
static QByteArray pdfLiteral(const QByteArray &text)
{
	QByteArray out("(");
	for (const char c : text) {
		if (c == '(' || c == ')' || c == '\\') out += '\\';
		out += c;
	}
	return out + ')';
}

/// The four integers of "\a key=A_B_C_D" in \a uri, or an empty list.
static QList<int> fragmentNumbers(const QByteArray &uri, const QByteArray &key)
{
	const int at = uri.indexOf(key + '=');
	if (at == -1) return {};
	const QByteArray rest = uri.mid(at + key.size() + 1);
	int end = 0;
	while (end < rest.size()
		   && ((rest[end] >= '0' && rest[end] <= '9')
			   || rest[end] == '_' || rest[end] == '-'))
		++end;
	const QList<QByteArray> parts = rest.left(end).split('_');
	if (parts.size() != 4) return {};
	QList<int> numbers;
	for (const QByteArray &part : parts) {
		bool ok = false;
		numbers << part.toInt(&ok);
		if (!ok) return {};
	}
	return numbers;
}

/// Where the /Annots array of the page object \a pageObj opens ('[') and
/// closes (']') in \a data: Qt writes "/Annots 17 0 R", an array object of
/// its own, but an inline array is read as well. False if it has none.
static bool annotsArray(const QByteArray &data, int pageObj, int *open, int *close)
{
	const int start = objectStart(data, pageObj);
	const int end = start == -1 ? -1 : data.indexOf("endobj", start);
	if (end == -1) return false;
	const int annots = data.indexOf("/Annots ", start);
	if (annots == -1 || annots > end) return false;
	int value = annots + 8;
	if (data.at(value) != '[') {
		const int arrayObj = data.mid(value, 12).split(' ').value(0).toInt();
		const int arrayStart = objectStart(data, arrayObj);
		value = arrayStart == -1 ? -1 : data.indexOf('[', arrayStart);
	}
	if (value == -1) return false;
	const int closing = data.indexOf(']', value);
	if (closing == -1) return false;
	*open = value;
	*close = closing;
	return true;
}

/**
	The JavaScript after a jump to the frame named \a name: show it, let it
	blink every 500 ms and hide it after 5 s, as EPLAN's frame does. The
	timers of the jump before are cleared. It only uses what Chrome's and
	Edge's viewer (PDFium) know as well as Acrobat: getField().display,
	app.setInterval() and app.setTimeOut(). EPLAN's own PDFs add the frame
	with addField() from a link's /Next instead; PDFium runs neither, tried
	in Chrome (25.11.2026).
*/
static QByteArray blinkScript(const QByteArray &name)
{
	const QByteArray field = "this.getField('" + name + "')";
	return
		"var q = this; q.qetN = 0;"
		" if (q.qetI) { app.clearInterval(q.qetI); app.clearTimeOut(q.qetT); }"
		" " + field + ".display = display.visible;"
		" q.qetI = app.setInterval(\"" + field
			+ ".display = (this.qetN++ % 2) ? display.visible : display.hidden;\", 500);"
		" q.qetT = app.setTimeOut(\"app.clearInterval(this.qetI); " + field
			+ ".display = display.hidden;\", 5000);";
}

void convertUriToGoTo(const QString &pdfPath)
{
	// --- 1. Read raw bytes ---
	QFile f(pdfPath);
	if (!f.open(QIODevice::ReadOnly)) return;
	QByteArray data = f.readAll();
	f.close();

	// --- 2. Collect page object numbers in document order ---
	// Read them from the page tree (/Type /Pages -> /Kids [ N 0 R ... ]).
	// This is reliable; scanning raw bytes for "/Type /Page" is NOT: that
	// marker also occurs inside content streams, and a forward lookahead
	// wrongly tags neighbouring objects (it found 280 "pages" for a 137-page
	// document). Qt writes a single, flat /Kids array listing every page.
	QVector<int> pageObjs;
	{
		int pagesPos = data.indexOf("/Type /Pages");
		int kidsPos  = (pagesPos == -1) ? -1 : data.indexOf("/Kids", pagesPos);
		int lb       = (kidsPos  == -1) ? -1 : data.indexOf('[', kidsPos);
		int rb       = (lb       == -1) ? -1 : data.indexOf(']', lb);
		if (lb != -1 && rb != -1 && rb > lb) {
			const QString kids =
				QString::fromLatin1(data.mid(lb + 1, rb - lb - 1));
			QRegularExpression re(QStringLiteral("(\\d+)\\s+\\d+\\s+R"));
			auto it = re.globalMatch(kids);
			while (it.hasNext()) {
				int objNum = it.next().captured(1).toInt();
				if (objNum > 0) pageObjs.append(objNum);
			}
		}
	}

	if (pageObjs.isEmpty()) return;  // nothing to do

	// The red frame of the jump target (EPLAN 2.9 and later): one hidden
	// read-only button per target, the frame, and over each link an
	// invisible push button with the jump, which shows that frame. Chrome
	// and Edge (PDFium) run only the GoTo of a link annotation, not its
	// /Next, but every action of a button, and their Hide action finds a
	// field by its name only; Acrobat runs the button too, it lies on top.
	// Not in PDF/A, as in EPLAN: PDF/A allows no JavaScript.
	struct Frame {
		int page;          // 1-based
		QList<int> rect;   // L B R T in points
		int widget;        // object numbers
		int appearance;
		QByteArray name;
	};
	QList<Frame> frames;
	QMap<QByteArray, int> frameOfTarget;   // "page:L_B_R_T" -> index
	struct Button {
		int page;          // 1-based
		QByteArray rect;   // as the link's /Rect
		QByteArray action; // the inside of the action dictionary
	};
	QList<Button> buttons;
	const bool marks = !data.contains("pdfaid:part");
	const int hideAll = highestObjectNumber(data) + 1;   // first new object
	QHash<int, int> pageOfAnnot;           // annotation object -> 1-based page
	if (marks) {
		static const QRegularExpression ref(QStringLiteral("(\\d+)\\s+0\\s+R"));
		for (int i = 0; i < pageObjs.size(); ++i) {
			int open = -1, close = -1;
			if (!annotsArray(data, pageObjs.at(i), &open, &close)) continue;
			auto it = ref.globalMatch(QString::fromLatin1(data.mid(open + 1, close - open - 1)));
			while (it.hasNext())
				pageOfAnnot.insert(it.next().captured(1).toInt(), i + 1);
		}
	}

	// --- 3. Replace URI annotations with GoTo ---
	// Pattern (Qt always writes exactly this):
	//   /S /URI\n/URI (file:///...<anything>#page=N)\n
	// or (older patches without file://):
	//   /S /URI\n/URI (page=N)\n
	bool changed = false;
	{
		// We do a manual scan to handle variable-length replacements.
		QByteArray out;
		out.reserve(data.size());

		const QByteArray sUri  = "/S /URI\n/URI (";
		const QByteArray sGoTo = "/S /GoTo\n/D [";
		int pos = 0;

		while (pos < data.size()) {
			int found = data.indexOf(sUri, pos);
			if (found == -1) {
				out.append(data.mid(pos));
				break;
			}

			// Copy everything up to the match
			out.append(data.mid(pos, found - pos));

			// Find closing ')' of the URI value
			int uriStart = found + sUri.size();
			int closeParen = data.indexOf(")\n", uriStart);
			if (closeParen == -1) {
				// Malformed — copy rest verbatim
				out.append(data.mid(found));
				pos = data.size();
				break;
			}

			QByteArray uriVal = data.mid(uriStart, closeParen - uriStart);

			// Skip component-info annotations — handled by
			// convertComponentInfoAnnotations() in a separate pass.
			if (uriVal.startsWith("componentinfo://")
				|| uriVal.startsWith("http://componentinfo.local/")) {
				out.append(data.mid(found, closeParen + 1 - found));
				pos = closeParen + 1;
				continue;
			}

			// Extract page number: look for #page=N or bare page=N
			int pageNum = -1;
			int hashPos = uriVal.lastIndexOf("#page=");
			int digitStart = -1;
			if (hashPos != -1) {
				digitStart = hashPos + 6;
			} else if (uriVal.startsWith("page=")) {
				digitStart = 5;
			}
			if (digitStart != -1) {
				// Take only the leading digits: the fragment may carry extra
				// parameters after the page number (e.g. "22&fitr=15_489_..."),
				// and QByteArray::toInt() would fail on the whole remainder.
				int e = digitStart;
				while (e < uriVal.size()
					   && uriVal[e] >= '0' && uriVal[e] <= '9')
					++e;
				if (e > digitStart)
					pageNum = uriVal.mid(digitStart, e - digitStart).toInt();
			}

			if (pageNum >= 1 && pageNum <= pageObjs.size()) {
				// Valid page reference — emit GoTo action.
				int pageObjNum = pageObjs[pageNum - 1];

				// Optional precise destination: &fitr=Left_Bottom_Right_Top
				// (integer PDF points). If present -> /FitR (frame the element);
				// otherwise -> /Fit (whole page, top).
				QByteArray dest = " /Fit]";
				const QList<int> fitr = fragmentNumbers(uriVal, "fitr");
				if (fitr.size() == 4) {
					dest = " /FitR " + QByteArray::number(fitr[0]) + " "
						   + QByteArray::number(fitr[1]) + " "
						   + QByteArray::number(fitr[2]) + " "
						   + QByteArray::number(fitr[3]) + "]";
				}

				// The jump, then: hide every frame, show the target's, let
				// it blink.
				QByteArray next;
				const QList<int> mark = marks ? fragmentNumbers(uriVal, "mark")
											  : QList<int>();
				if (mark.size() == 4 && mark[2] > mark[0] && mark[3] > mark[1]) {
					QByteArray key = QByteArray::number(pageNum) + ':';
					for (int v : mark) key += QByteArray::number(v) + '_';
					int index = frameOfTarget.value(key, -1);
					if (index == -1) {
						index = frames.size();
						const int widget = hideAll + 1 + 2 * index;
						frames.append({pageNum, mark, widget, widget + 1,
									   "qetmark" + QByteArray::number(index + 1)});
						frameOfTarget.insert(key, index);
					}
					const Frame &frame = frames.at(index);
					next = "\n/Next [" + QByteArray::number(hideAll) + " 0 R"
						+ " << /S /Hide /T " + pdfLiteral(frame.name) + " /H false >>"
						+ " << /S /JavaScript /JS " + pdfLiteral(blinkScript(frame.name))
						+ " >>]";
				}

				QByteArray goTo = sGoTo
					+ QByteArray::number(pageObjNum)
					+ " 0 R" + dest + next;
				out.append(goTo);

				// The button over the link: the link's rectangle and page
				if (!next.isEmpty()) {
					const int head = data.lastIndexOf(" 0 obj", found);
					const int line = head == -1 ? -1 : data.lastIndexOf('\n', head) + 1;
					const int linkObj = line == -1 ? 0 : data.mid(line, head - line).trimmed().toInt();
					const int rectAt = line == -1 ? -1 : data.indexOf("/Rect [", line);
					const int rectEnd = rectAt == -1 ? -1 : data.indexOf(']', rectAt);
					const int page = pageOfAnnot.value(linkObj, 0);
					if (page > 0 && rectAt != -1 && rectEnd != -1 && rectEnd < found)
						buttons.append({page, data.mid(rectAt + 7, rectEnd - rectAt - 7).simplified(),
										goTo});
				}
				changed = true;
			} else {
				// Unknown page — keep original URI
				out.append(sUri);
				out.append(uriVal);
				out.append(')');
			}

			pos = closeParen + 1;  // skip past ')'
		}

		if (!changed) return;  // nothing was replaced
		data = out;
	}

	// --- 4. Rebuild xref table ---
	// Find start of existing xref (last occurrence)
	int xrefStart = data.lastIndexOf("\nxref\n");
	if (xrefStart == -1) xrefStart = data.lastIndexOf("\nxref ");
	if (xrefStart == -1) return;  // malformed PDF
	++xrefStart;  // skip the leading '\n'

	QByteArray body = data.left(xrefStart);

	// --- 3b. The frames and the buttons: their objects, on their pages'
	// /Annots, and in the form, where the Hide actions and getField() find
	// them by name ---
	if (!frames.isEmpty()) {
		// After the frames: one empty appearance, then the buttons.
		const int emptyAppearance = hideAll + 1 + 2 * int(frames.size());
		auto buttonObj = [&](int i) { return emptyAppearance + 1 + i; };

		QByteArray fields, names;
		for (const Frame &frame : std::as_const(frames)) {
			fields += QByteArray::number(frame.widget) + " 0 R ";
			names += pdfLiteral(frame.name) + ' ';
		}
		for (int i = 0; i < buttons.size(); ++i)
			fields += QByteArray::number(buttonObj(i)) + " 0 R ";
		fields = fields.trimmed();
		names = names.trimmed();

		// Per page, the frames go first on /Annots: below the links, so a
		// frame that is showing never catches a click. The buttons go last,
		// on top of the links they stand for.
		QMap<int, QByteArray> firstOfPage, lastOfPage;
		for (const Frame &frame : std::as_const(frames))
			firstOfPage[frame.page] += QByteArray::number(frame.widget) + " 0 R ";
		for (int i = 0; i < buttons.size(); ++i)
			lastOfPage[buttons.at(i).page] += " " + QByteArray::number(buttonObj(i)) + " 0 R";
		QList<int> pages = firstOfPage.keys() + lastOfPage.keys();
		std::sort(pages.begin(), pages.end());
		pages.erase(std::unique(pages.begin(), pages.end()), pages.end());
		for (int page : std::as_const(pages)) {
			const int pageObj = pageObjs.at(page - 1);
			int open = -1, close = -1;
			if (annotsArray(body, pageObj, &open, &close)) {
				body.insert(close, lastOfPage.value(page) + ' ');
				body.insert(open + 1, " " + firstOfPage.value(page));
			} else {
				const int start = objectStart(body, pageObj);
				const int dict = start == -1 ? -1 : body.indexOf("<<", start);
				if (dict == -1) continue;
				body.insert(dict + 2, "\n/Annots [" + firstOfPage.value(page)
										  + lastOfPage.value(page) + "]");
			}
		}

		const int catalog = body.indexOf("/Type /Catalog");
		if (catalog != -1) {
			const int catalogEnd = body.indexOf(">>", catalog);
			const int form = body.indexOf("/AcroForm", catalog);
			if (form == -1 || form > catalogEnd)
				body.insert(catalog + 14, "\n/AcroForm << /Fields [" + fields + "] >>");
		}

		QByteArray objects;
		objects += QByteArray::number(hideAll) + " 0 obj\n<<\n/Type /Action\n/S /Hide\n/T ["
			+ names + "]\n/H true\n>>\nendobj\n";
		for (const Frame &frame : std::as_const(frames)) {
			const int w = frame.rect[2] - frame.rect[0];
			const int h = frame.rect[3] - frame.rect[1];
			const QByteArray rect = QByteArray::number(frame.rect[0]) + " "
				+ QByteArray::number(frame.rect[1]) + " "
				+ QByteArray::number(frame.rect[2]) + " "
				+ QByteArray::number(frame.rect[3]);
			objects += QByteArray::number(frame.widget) + " 0 obj\n<<\n"
				"/Type /Annot\n/Subtype /Widget\n/FT /Btn\n/Ff 65537\n"
				"/T " + pdfLiteral(frame.name) + "\n"
				"/Rect [" + rect + "]\n"
				"/F 2\n"
				"/P " + QByteArray::number(pageObjs.at(frame.page - 1)) + " 0 R\n"
				"/MK << /BC [1 0 0] >>\n"
				"/BS << /W 3 /S /S >>\n"
				"/AP << /N " + QByteArray::number(frame.appearance) + " 0 R >>\n"
				">>\nendobj\n";
			const QByteArray stroke = "1 0 0 RG 3 w 1.5 1.5 "
				+ QByteArray::number(w - 3) + " " + QByteArray::number(h - 3)
				+ " re S\n";
			objects += QByteArray::number(frame.appearance) + " 0 obj\n<<\n"
				"/Type /XObject\n/Subtype /Form\n"
				"/BBox [0 0 " + QByteArray::number(w) + " " + QByteArray::number(h) + "]\n"
				"/Length " + QByteArray::number(stroke.size()) + "\n"
				">>\nstream\n" + stroke + "endstream\nendobj\n";
		}
		objects += QByteArray::number(emptyAppearance) + " 0 obj\n<<\n"
			"/Type /XObject\n/Subtype /Form\n/BBox [0 0 1 1]\n/Length 0\n"
			">>\nstream\n\nendstream\nendobj\n";
		// The button jumps when the mouse button goes down (/AA /D), not
		// when it is released (/A): PDFium keeps the focus on the button
		// clicked last and hands it the next release first, which then
		// swallowed every other click on a new arrow in Chrome (Birger,
		// 25.11.2026). A press goes to the button under the pointer.
		for (int i = 0; i < buttons.size(); ++i) {
			const Button &button = buttons.at(i);
			objects += QByteArray::number(buttonObj(i)) + " 0 obj\n<<\n"
				"/Type /Annot\n/Subtype /Widget\n/FT /Btn\n/Ff 65536\n"
				"/T " + pdfLiteral("qetjump" + QByteArray::number(i + 1)) + "\n"
				"/Rect [" + button.rect + "]\n"
				"/F 4\n/H /N\n"
				"/P " + QByteArray::number(pageObjs.at(button.page - 1)) + " 0 R\n"
				"/AP << /N " + QByteArray::number(emptyAppearance) + " 0 R >>\n"
				"/AA << /D <<\n/Type /Action\n" + button.action + "\n>> >>\n"
				">>\nendobj\n";
		}
		body += objects;
	}

	// Collect all object offsets from the body
	QMap<int, int> offsets;  // objNum -> byte offset
	{
		const QByteArray objMarker = " 0 obj";
		int pos = 0;
		while ((pos = body.indexOf(objMarker, pos)) != -1) {
			int numStart = pos - 1;
			while (numStart > 0 && body[numStart-1] != '\n' && body[numStart-1] != '\r')
				--numStart;
			QByteArray numStr = body.mid(numStart, pos - numStart).trimmed();
			bool ok = false;
			int objNum = numStr.toInt(&ok);
			if (ok && objNum > 0)
				offsets[objNum] = numStart;
			++pos;
		}
	}

	if (offsets.isEmpty()) return;

	int maxObj = offsets.lastKey();

	// Build xref table
	QByteArray xref;
	xref += "xref\n";
	xref += "0 " + QByteArray::number(maxObj + 1) + "\n";
	xref += "0000000000 65535 f \n";
	for (int i = 1; i <= maxObj; ++i) {
		if (offsets.contains(i)) {
			xref += QByteArray::number(offsets[i]).rightJustified(10, '0')
				+ " 00000 n \n";
		} else {
			xref += "0000000000 65535 f \n";
		}
	}

	QByteArray trailer = trailerDictionary(data, xrefStart);
	{
		// /Size: one more than the highest object, the frames included
		const int sizePos = trailer.indexOf("/Size ");
		if (sizePos != -1) {
			const int numStart = sizePos + 6;
			int numEnd = numStart;
			while (numEnd < trailer.size() && trailer[numEnd] >= '0' && trailer[numEnd] <= '9')
				++numEnd;
			if (numEnd > numStart)
				trailer.replace(numStart, numEnd - numStart, QByteArray::number(maxObj + 1));
		}
	}

	int newXrefOffset = body.size();

	QByteArray result;
	result.reserve(body.size() + xref.size() + trailer.size() + 30);
	result += body;
	result += xref;
	result += trailer;
	result += "\nstartxref\n";
	result += QByteArray::number(newXrefOffset);
	result += "\n%%EOF\n";

	// --- 5. Write back ---
	QFile out(pdfPath);
	if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate)) return;
	out.write(result);
	out.close();
}

void removeUnusedPdfxNamespace(const QString &pdfPath)
{
	QFile f(pdfPath);
	if (!f.open(QIODevice::ReadOnly)) return;
	QByteArray data = f.readAll();
	f.close();

	// Qt only fills the pdfxid namespace in when it writes PDF/X-4.
	if (data.contains("pdfxid:GTS_PDFXVersion")) return;

	static const QByteArray decl =
		" xmlns:pdfxid=\"http://www.npes.org/pdfx/ns/id/\"";
	const int pos = data.indexOf(decl);
	if (pos == -1) return;

	// Same length, so the stream /Length and the xref offsets stay valid.
	data.replace(pos, decl.size(), QByteArray(decl.size(), ' '));

	if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) return;
	f.write(data);
	f.close();
}

QUuid placeholderDocumentId()
{
	// Any fixed uuid: it is also the namespace of the ids made from it.
	return QUuid(QStringLiteral("{6f1c2d4e-9a3b-4c5d-8e7f-0a1b2c3d4e5f}"));
}

void setDocumentIdFromContent(const QString &pdfPath)
{
	QFile f(pdfPath);
	if (!f.open(QIODevice::ReadOnly)) return;
	QByteArray data = f.readAll();
	f.close();

	// Qt writes the id as text in the XMP ("uuid:6f1c...") and as the hex
	// of that same text in the trailer ("/ID [ <3666...> <3666...> ]").
	const QUuid placeholder = placeholderDocumentId();
	const QByteArray text = placeholder.toByteArray(QUuid::WithoutBraces);
	if (!data.contains(text) && !data.contains(text.toHex())) return;

	const QUuid id = QUuid::createUuidV5(
		placeholder, QCryptographicHash::hash(data, QCryptographicHash::Sha256));
	const QByteArray idText = id.toByteArray(QUuid::WithoutBraces);
	data.replace(text, idText);
	data.replace(text.toHex(), idText.toHex());

	if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) return;
	f.write(data);
}

void setDocumentDate(const QString &pdfPath, const QDateTime &when)
{
	QFile f(pdfPath);
	if (!f.open(QIODevice::ReadOnly)) return;
	const QByteArray data = f.readAll();
	f.close();

	const QDateTime utc = when.toUTC();
	const QByteArray pdfDate =
		"(D:" + utc.toString(QStringLiteral("yyyyMMddHHmmss")).toLatin1() + "Z)";
	const QByteArray xmpDate =
		utc.toString(QStringLiteral("yyyy-MM-ddTHH:mm:ss")).toLatin1() + "Z";

	// Each edit replaces [start, end) of the original file.
	struct Edit { int start; int end; QByteArray text; };
	QList<Edit> edits;

	// The document information: "/CreationDate (D:...)" and, from Qt 6.8,
	// "/ModDate (D:...)". Its other strings are UTF-16, so they cannot hold
	// these keys.
	for (const QByteArray &key : {QByteArray("/CreationDate "), QByteArray("/ModDate ")}) {
		const int k = data.indexOf(key);
		if (k == -1) {
			if (key.startsWith("/ModDate")) continue;
			return;
		}
		const int start = k + key.size();
		const int end = data.indexOf(')', start);
		if (end == -1 || data.at(start) != '(') return;
		edits.append({start, end + 1, pdfDate});
	}

	// The XMP metadata stream, which Qt writes uncompressed from Qt 6.8
	// (before, only for PDF/A): its three dates, and its /Length, which
	// changes with them.
	const QByteArray metadata("/Type /Metadata /Subtype /XML");
	const int m = data.indexOf(metadata);
	if (m != -1) {
		const QByteArray lengthKey("/Length ");
		const int l = data.indexOf(lengthKey, m);
		const int streamStart = data.indexOf("stream\n", m);
		if (l == -1 || streamStart == -1 || l > streamStart) return;
		const int lengthStart = l + lengthKey.size();
		int lengthEnd = lengthStart;
		while (lengthEnd < data.size() && QChar(data.at(lengthEnd)).isDigit())
			++lengthEnd;
		const int streamEnd = data.indexOf("endstream", streamStart);
		if (lengthEnd == lengthStart || streamEnd == -1) return;

		// The last of each in the stream: the title and author come before
		// the dates, and are text a project could fill with anything.
		int xmpDelta = 0;
		QList<Edit> xmpEdits;
		for (const QByteArray &attr : {QByteArray("xmp:CreateDate=\""),
									   QByteArray("xmp:ModifyDate=\""),
									   QByteArray("xmp:MetadataDate=\"")}) {
			const int a = data.lastIndexOf(attr, streamEnd);
			if (a < streamStart) return;
			const int start = a + attr.size();
			const int end = data.indexOf('"', start);
			if (end == -1 || end > streamEnd) return;
			xmpEdits.append({start, end, xmpDate});
			xmpDelta += xmpDate.size() - (end - start);
		}
		const int length = data.mid(lengthStart, lengthEnd - lengthStart).toInt();
		edits.append({lengthStart, lengthEnd, QByteArray::number(length + xmpDelta)});
		edits.append(xmpEdits);
	}

	std::sort(edits.begin(), edits.end(),
			  [](const Edit &a, const Edit &b) { return a.start < b.start; });

	// Where an offset in the original file is in the new one.
	auto moved = [&edits](int offset) {
		int shift = 0;
		for (const Edit &e : std::as_const(edits))
			if (e.end <= offset)
				shift += e.text.size() - (e.end - e.start);
		return offset + shift;
	};

	// The xref table the last startxref points at, which is the one a
	// reader uses.
	const int sx = data.lastIndexOf("startxref");
	if (sx == -1) return;
	int numStart = sx + 9;
	while (numStart < data.size() && QChar(data.at(numStart)).isSpace())
		++numStart;
	int numEnd = numStart;
	while (numEnd < data.size() && QChar(data.at(numEnd)).isDigit())
		++numEnd;
	const int xref = data.mid(numStart, numEnd - numStart).toInt();
	if (numEnd == numStart || !data.mid(xref).startsWith("xref")) return;
	for (const Edit &e : std::as_const(edits))
		if (e.end > xref) return;    // every edit is in the objects before it

	QByteArray out;
	out.reserve(data.size() + 64);
	int pos = 0;
	for (const Edit &e : std::as_const(edits)) {
		if (e.start < pos) return;   // overlapping: not a file Qt wrote
		out += data.mid(pos, e.start - pos);
		out += e.text;
		pos = e.end;
	}

	// The xref entries: "0000012345 00000 n \n", 20 bytes each, after the
	// "xref" line and one "first count" line.
	int line = data.indexOf('\n', xref) + 1;
	line = data.indexOf('\n', line) + 1;
	if (line <= 0) return;
	out += data.mid(pos, line - pos);
	pos = line;
	static const QRegularExpression entry(QStringLiteral(R"(^(\d{10}) (\d{5}) ([nf])\s*$)"));
	while (pos + 18 <= data.size()) {
		const QByteArray row = data.mid(pos, 20);
		const auto match = entry.match(QString::fromLatin1(row));
		if (!match.hasMatch()) break;
		if (match.captured(3) == QLatin1String("n")) {
			const int offset = match.captured(1).toInt();
			out += QByteArray::number(moved(offset)).rightJustified(10, '0');
			out += row.mid(10);
		} else {
			out += row;
		}
		pos += 20;
	}
	out += data.mid(pos, numStart - pos);
	out += QByteArray::number(moved(xref));
	out += data.mid(numEnd);

	if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) return;
	f.write(out);
	f.close();
}

void convertComponentInfoAnnotations(const QString &pdfPath,
									const QList<ComponentInfo> &annotations)
{
	if (annotations.isEmpty()) return;

	QFile f(pdfPath);
	if (!f.open(QIODevice::ReadOnly)) return;
	QByteArray data = f.readAll();
	f.close();

	const QByteArray markerPrefix("http://componentinfo.local/");
	if (!data.contains(markerPrefix)) return;

	int xrefStart = data.lastIndexOf("\nxref\n");
	if (xrefStart == -1) xrefStart = data.lastIndexOf("\nxref ");
	if (xrefStart == -1) return;
	++xrefStart;
	QByteArray body = data.left(xrefStart);

	// Pre-scan body for highest object number
	int maxObjNum = 0;
	{
		const QByteArray objMarker(" 0 obj");
		int p = 0;
		while ((p = body.indexOf(objMarker, p)) != -1) {
			int numStart = p - 1;
			while (numStart > 0 && body[numStart - 1] != '\n' && body[numStart - 1] != '\r')
				--numStart;
			QByteArray numStr = body.mid(numStart, p - numStart).trimmed();
			bool ok = false;
			int num = numStr.toInt(&ok);
			if (ok && num > maxObjNum)
				maxObjNum = num;
			++p;
		}
	}
	// Empty Form XObject used as invisible appearance for all annotations.
	// All annotations reference this single object.
	int emptyXObjNum = maxObjNum + 1;

	QByteArray out;
	out.reserve(data.size());

	int pos = 0;
	bool anyConverted = false;

	while (pos < body.size()) {
		// Find next indexed marker: http://componentinfo.local/<N>
		int markerPos = body.indexOf(markerPrefix, pos);
		if (markerPos == -1) {
			out.append(body.mid(pos));
			break;
		}

		// Extract index from marker URL
		int idxStart = markerPos + markerPrefix.size();
		int idxEnd = idxStart;
		while (idxEnd < body.size() && body[idxEnd] >= '0' && body[idxEnd] <= '9')
			++idxEnd;
		if (idxEnd == idxStart) {
			// No index — skip malformed marker
			out.append(body.mid(pos, markerPos + markerPrefix.size() - pos));
			pos = markerPos + markerPrefix.size();
			continue;
		}
		int annotIndex = body.mid(idxStart, idxEnd - idxStart).toInt();

		int uriOpen = body.lastIndexOf("/URI (", markerPos);
		int closeParen = (uriOpen != -1) ? body.indexOf(')', markerPos) : -1;
		if (uriOpen == -1 || closeParen == -1 || uriOpen < pos) {
			out.append(body.mid(pos, idxEnd - pos));
			pos = idxEnd;
			continue;
		}

		// Find /S /URI before /URI (
		int sUriPos = body.lastIndexOf("/S /URI", uriOpen);

		// Find the /A << that opens the action dict containing /S /URI.
		// Qt always writes: /A <<\n/S /URI\n/URI (...)\n>>\n>>
		int aDictOpen = (sUriPos != -1)
			? body.lastIndexOf("/A <<", sUriPos)
			: -1;

		// If /A << is not found, leave this annotation untouched
		if (aDictOpen == -1 || aDictOpen < pos) {
			out.append(body.mid(pos, closeParen + 1 - pos));
			pos = closeParen + 1;
			continue;
		}

		// Validate index
		if (annotIndex < 0 || annotIndex >= annotations.size()) {
			// Index out of range — keep original annotation intact
			out.append(body.mid(pos, closeParen + 1 - pos));
			pos = closeParen + 1;
			continue;
		}

		// Copy annotation dict header up to /A <<
		out.append(body.mid(pos, aDictOpen - pos));

		QByteArray contents = annotations[annotIndex].contents.toUtf8();

		// Replace /Subtype /Link with /Subtype /Text, bounded to the enclosing
		// PDF object.  Anchoring to " 0 obj" prevents hitting /Subtype /Link
		// from a cross-ref annotation that sits between the previous marker
		// and the current annotation on the same page.
		int objStart = body.lastIndexOf(" 0 obj", aDictOpen);
		int subTypeInBody = (objStart != -1)
			? body.indexOf("/Subtype /Link", objStart) : -1;
		int subTypePos = (subTypeInBody != -1 && subTypeInBody < aDictOpen)
			? out.size() - (aDictOpen - subTypeInBody) : -1;
		if (subTypePos != -1)
			// Acrobat draws its own sticky-note icon for /Subtype /Text
			// whatever /AP says (verified on Reader 5.0). /Square with no
			// /C and no /IC draws nothing anywhere, and is still a markup
			// annotation, so the /Contents popup is unaffected.
			out.replace(subTypePos, 14, "/Subtype /Square");

		// Encode as UTF-16BE hex with BOM for proper Unicode support (Umlauten etc.).
		// PDF spec: hex strings starting with FE FF are interpreted as UTF-16BE.
		QByteArray utf16be;
		utf16be.append('\xfe');
		utf16be.append('\xff');
		{
			for (int i = 0; i < contents.size(); ) {
				ushort cp = 0;
				uchar c = static_cast<uchar>(contents.at(i));
				if (c < 0x80) {
					cp = c;
					++i;
				} else if ((c & 0xE0) == 0xC0 && i + 1 < contents.size()) {
					cp = ((c & 0x1F) << 6)
					   | (static_cast<uchar>(contents.at(i + 1)) & 0x3F);
					i += 2;
				} else if ((c & 0xF0) == 0xE0 && i + 2 < contents.size()) {
					cp = ((c & 0x0F) << 12)
					   | ((static_cast<uchar>(contents.at(i + 1)) & 0x3F) << 6)
					   | (static_cast<uchar>(contents.at(i + 2)) & 0x3F);
					i += 3;
				} else {
					cp = 0xFFFD; // replacement character
					++i;
				}
				utf16be.append(static_cast<char>((cp >> 8) & 0xFF));
				utf16be.append(static_cast<char>(cp & 0xFF));
			}
		}

		out += "/Contents <";
		out += utf16be.toHex();
		out += ">\n/AP << /N " + QByteArray::number(emptyXObjNum) + " 0 R >>\n";

		// Skip the entire /A << ... >> action dict.
		// After closeParen ')' we have: \n>>  (closes /A dict)  \n>>  (closes annot dict)
		int actionDictClose = body.indexOf(">>", closeParen + 1);
		if (actionDictClose != -1) {
			pos = actionDictClose + 2;  // skip past >> that closes /A dict
		} else {
			pos = closeParen + 1;
		}
		anyConverted = true;
	}

	if (!anyConverted) return;

	// Append empty Form XObject (shared by all annotations for invisible appearance)
	QByteArray emptyXObj;
	emptyXObj += QByteArray::number(emptyXObjNum) + " 0 obj\n";
	emptyXObj += "<< /Type /XObject /Subtype /Form /BBox [0 0 0 0] /Length 0 >>\n";
	emptyXObj += "stream\nendstream\n";
	emptyXObj += "endobj\n";
	out += emptyXObj;

	// Rebuild xref table
	QMap<int, int> offsets;
	{
		const QByteArray objMarker(" 0 obj");
		int p = 0;
		while ((p = out.indexOf(objMarker, p)) != -1) {
			int numStart = p - 1;
			while (numStart > 0 && out[numStart - 1] != '\n' && out[numStart - 1] != '\r')
				--numStart;
			QByteArray numStr = out.mid(numStart, p - numStart).trimmed();
			bool ok = false;
			int objNum = numStr.toInt(&ok);
			if (ok && objNum > 0)
				offsets[objNum] = numStart;
			++p;
		}
	}

	if (offsets.isEmpty()) return;

	int maxObj = offsets.lastKey();

	QByteArray xref;
	xref += "xref\n";
	xref += "0 " + QByteArray::number(maxObj + 1) + "\n";
	xref += "0000000000 65535 f \n";
	for (int i = 1; i <= maxObj; ++i) {
		if (offsets.contains(i)) {
			xref += QByteArray::number(offsets[i]).rightJustified(10, '0')
				+ " 00000 n \n";
		} else {
			xref += "0000000000 65535 f \n";
		}
	}

	// Copy trailer and bump /Size to account for the new XObject
	QByteArray trailer = trailerDictionary(data, xrefStart);

	// Bump /Size: original was maxObjNum+1, now it's emptyXObjNum+1
	{
		int sizePos = trailer.indexOf("/Size ");
		if (sizePos != -1) {
			int numStart = sizePos + 6;
			int numEnd = numStart;
			while (numEnd < trailer.size() && trailer[numEnd] >= '0' && trailer[numEnd] <= '9')
				++numEnd;
			if (numEnd > numStart) {
				trailer.replace(numStart, numEnd - numStart,
								QByteArray::number(emptyXObjNum + 1));
			}
		}
	}

	QByteArray result;
	result.reserve(out.size() + xref.size() + trailer.size() + 64);
	result += out;
	int newXrefOffset = out.size();
	result += xref;
	result += trailer;
	result += "\nstartxref\n";
	result += QByteArray::number(newXrefOffset);
	result += "\n%%EOF\n";

	QFile outF(pdfPath);
	if (!outF.open(QIODevice::WriteOnly | QIODevice::Truncate)) return;
	outF.write(result);
	outF.close();
}

/// \a text as a PDF text string: a literal for plain ASCII, else UTF-16BE
/// with its byte order mark, as hex
static QByteArray pdfTextString(const QString &text)
{
	bool ascii = true;
	for (const QChar c : text) if (c.unicode() < 32 || c.unicode() > 126) {ascii = false; break;}
	if (ascii) return pdfLiteral(text.toLatin1());
	QByteArray hex("<FEFF");
	for (const QChar c : text) hex += QByteArray::number(c.unicode(), 16).rightJustified(4, '0').toUpper();
	return hex + '>';
}

QString outlineTitle(Diagram *diagram)
{
	if (!diagram) return QString();
	QString folio = diagram->border_and_titleblock.finalfolio().trimmed();
	if (folio.isEmpty()) folio = QString::number(diagram->folioIndex() + 1);
	const QString title = diagram->title().trimmed();
	return title.isEmpty() ? folio : folio + QLatin1Char(' ') + title;
}

void addOutline(const QString &pdfPath, const QList<QPair<int, QString>> &entries)
{
	if (entries.isEmpty()) return;
	QFile f(pdfPath);
	if (!f.open(QIODevice::ReadOnly)) return;
	QByteArray data = f.readAll();
	f.close();
	if (data.contains("/Outlines")) return;

		// the pages in order, from the page tree as convertUriToGoTo() reads it
	QVector<int> pages;
	{
		const int pagesPos = data.indexOf("/Type /Pages");
		const int kidsPos = pagesPos == -1 ? -1 : data.indexOf("/Kids", pagesPos);
		const int lb = kidsPos == -1 ? -1 : data.indexOf('[', kidsPos);
		const int rb = lb == -1 ? -1 : data.indexOf(']', lb);
		if (lb == -1 || rb <= lb) return;
		static const QRegularExpression re(QStringLiteral("(\\d+)\\s+\\d+\\s+R"));
		auto it = re.globalMatch(QString::fromLatin1(data.mid(lb + 1, rb - lb - 1)));
		while (it.hasNext()) pages << it.next().captured(1).toInt();
	}
	if (pages.isEmpty()) return;

		// the last trailer: its /Root (the catalog), /Info, /ID and where
		// its xref starts
	const int sx = data.lastIndexOf("startxref");
	if (sx == -1) return;
	bool ok = false;
	const int prev = data.mid(sx + 9, 24).trimmed().split('\n').value(0).trimmed().toInt(&ok);
	if (!ok) return;
	const int tr = data.lastIndexOf("trailer", sx);
	if (tr == -1) return;
	const QByteArray trailer = data.mid(tr, sx - tr);
	static const QRegularExpression root_re(QStringLiteral("/Root\\s+(\\d+)\\s+0\\s+R"));
	const QRegularExpressionMatch rm = root_re.match(QString::fromLatin1(trailer));
	if (!rm.hasMatch()) return;
	const int catalog = rm.captured(1).toInt();
	const int cs = objectStart(data, catalog);
	if (cs == -1) return;
	const int dict_open = data.indexOf("<<", cs);
	const int obj_end = data.indexOf("endobj", cs);
	const int dict_close = data.lastIndexOf(">>", obj_end);
	if (dict_open == -1 || obj_end == -1 || dict_close <= dict_open) return;
	QByteArray catalog_dict = data.mid(dict_open + 2, dict_close - dict_open - 2);
	static const QRegularExpression mode_re(QStringLiteral("/PageMode\\s*/\\w+"));
	catalog_dict = QString::fromLatin1(catalog_dict).remove(mode_re).toLatin1();

		// the new objects: the outline root, then one item per entry
	QList<QPair<int, QString>> items;
	for (const auto &e : entries)
		if (e.first >= 0 && e.first < pages.size()) items << e;
	if (items.isEmpty()) return;
	const int first = highestObjectNumber(data) + 1;
	const int outlines = first;
	QByteArray update("\n");
	QMap<int, int> offsets;
	auto object = [&](int num, const QByteArray &body) {
		offsets.insert(num, data.size() + update.size());
		update += QByteArray::number(num) + " 0 obj\n" + body + "\nendobj\n";
	};
	object(outlines, "<< /Type /Outlines /First " + QByteArray::number(first + 1) + " 0 R /Last "
					 + QByteArray::number(first + int(items.size())) + " 0 R /Count "
					 + QByteArray::number(items.size()) + " >>");
	for (int i = 0; i < items.size(); ++i) {
		const int num = first + 1 + i;
		QByteArray body = "<< /Title " + pdfTextString(items.at(i).second) + " /Parent "
						  + QByteArray::number(outlines) + " 0 R";
		if (i > 0) body += " /Prev " + QByteArray::number(num - 1) + " 0 R";
		if (i + 1 < items.size()) body += " /Next " + QByteArray::number(num + 1) + " 0 R";
		body += " /Dest [" + QByteArray::number(pages.at(items.at(i).first)) + " 0 R /Fit] >>";
		object(num, body);
	}
	object(catalog, "<<" + catalog_dict + " /Outlines " + QByteArray::number(outlines)
					+ " 0 R /PageMode /UseOutlines >>");

		// the xref section of the update, one subsection per run of numbers
	QByteArray xref("xref\n");
	for (auto it = offsets.constBegin(); it != offsets.constEnd();) {
		auto end = it;
		int count = 0;
		while (end != offsets.constEnd() && end.key() == it.key() + count) {++end; ++count;}
		xref += QByteArray::number(it.key()) + ' ' + QByteArray::number(count) + '\n';
		for (; it != end; ++it) xref += QByteArray::number(it.value()).rightJustified(10, '0') + " 00000 n \n";
	}
	const int size = qMax(first + int(items.size()) + 1, catalog + 1);
	QByteArray new_trailer = "trailer\n<< /Size " + QByteArray::number(size) + " /Root "
							 + QByteArray::number(catalog) + " 0 R";
	static const QRegularExpression info_re(QStringLiteral("/Info\\s+\\d+\\s+0\\s+R"));
	static const QRegularExpression id_re(QStringLiteral("/ID\\s*\\[[^\\]]*\\]"));
	const QRegularExpressionMatch im = info_re.match(QString::fromLatin1(trailer));
	if (im.hasMatch()) new_trailer += ' ' + im.captured(0).toLatin1();
	const QRegularExpressionMatch idm = id_re.match(QString::fromLatin1(trailer));
	if (idm.hasMatch()) new_trailer += ' ' + idm.captured(0).toLatin1();
	new_trailer += " /Prev " + QByteArray::number(prev) + " >>\n";
	const int xref_at = data.size() + update.size();
	data += update + xref + new_trailer + "startxref\n" + QByteArray::number(xref_at) + "\n%%EOF\n";

	QFile out(pdfPath);
	if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate)) return;
	out.write(data);
	out.close();
}

} // namespace PdfLinks
