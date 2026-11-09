/*
	SiePortal-Links für QElectroTech (Herstellerportale)

	SiePortal lässt keine Programme zu (Bot-Schutz) und hat keine Schnittstelle
	für die CAx-Daten. Die Dateien selbst (EPLAN-Makro, DXF, STEP) liegen aber
	öffentlich auf static.siemens.com; QElectroTech lädt sie, sobald es ihre
	Links kennt. Dieses Skript sammelt die Links im eigenen, normal angemeldeten
	Browser – so, wie man sie im Dialog „Download von Produktbildern und Daten
	für M-CAD und E-CAD“ von Hand anklicken würde – und gibt sie als Zeilen für
	Daten\herstellerdaten\Siemens\sieportal_links.txt aus.

	Benutzung:
	1. https://sieportal.siemens.com im Browser öffnen (angemeldet), F12 → Konsole.
	2. Dieses Skript einfügen, dann z. B.
	     copy(await sieportalLinks(["3RV2011-0FA10", "3RT2015-1BB41"]))
	   Ergebnis: Zeilen "<Bestellnummer>\t<Link>"; in die Linkdatei anhängen
	   oder in QElectroTech unter Bauteil-Import > Herstellerportale > Siemens
	   > „Links hinzufügen…“ einfügen. Danach --fetch-parts bzw. „Fehlende
	   Bauteile holen“.

	Es arbeitet langsam und nacheinander (höchstens 2 Seiten zugleich, Pausen),
	wie beim Klicken von Hand – keine Umgehung von Schutzmaßnahmen, keine CAPTCHAs.
*/
async function sieportalLinks(orders, {parallel = 2, timeoutMs = 30000} = {}) {
	const sleep = ms => new Promise(r => setTimeout(r, ms));
	const wanted = /static\.siemens\.com\/mimes\/imagedb_(edz|dxf|stp)\//;

	async function waitFor(doc, find, ms) {
		const end = Date.now() + ms;
		while (Date.now() < end) {
			const found = find(doc);
			if (found) return found;
			await sleep(300);
		}
		return null;
	}

	async function one(order) {
		const id = order.replace(/\s+/g, "");
		const frame = document.createElement("iframe");
		frame.style.cssText = "position:fixed;left:-3000px;top:0;width:1400px;height:1000px;visibility:hidden";
		frame.src = "/de-de/products-services/detail/" + encodeURIComponent(id);
		const loaded = new Promise(r => frame.addEventListener("load", r, {once: true}));
		document.body.appendChild(frame);
		const links = new Set();
		try {
			await loaded;
			const doc = frame.contentDocument;
			const button = await waitFor(doc, d => [...d.querySelectorAll("button, a")]
				.find(b => /M-CAD|E-CAD|Produktbildern/i.test(b.textContent || "")), timeoutMs);
			if (!button) return {order: id, links: [], note: "kein CAx-Knopf"};
			button.click();
			const dialog = await waitFor(doc, d => d.querySelector("[role=dialog]"), 10000);
			if (!dialog) return {order: id, links: [], note: "kein Dialog"};
			await sleep(800);
			const thumbs = [...dialog.querySelectorAll("img")].filter(i => i.getBoundingClientRect().width < 80);
			for (const t of thumbs) {
				t.click();
				await sleep(600);
				for (const a of doc.querySelectorAll("[role=dialog] a[href]"))
					if (wanted.test(a.href)) links.add(a.href.split("?")[0]);
			}
			return {order: id, links: [...links]};
		} catch (e) {
			return {order: id, links: [...links], note: String(e)};
		} finally {
			frame.remove();
		}
	}

	const results = [];
	const queue = [...orders];
	async function worker() {
		while (queue.length) {
			results.push(await one(queue.shift()));
			await sleep(1500);
		}
	}
	await Promise.all(Array.from({length: Math.max(1, Math.min(parallel, 2))}, worker));
	const lines = results.flatMap(r => r.links.map(l => r.order + "\t" + l));
	const missing = results.filter(r => !r.links.length).map(r => r.order + (r.note ? " (" + r.note + ")" : ""));
	if (missing.length) console.warn("Ohne Links:", missing.join(", "));
	return lines.join("\n");
}
