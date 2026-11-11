"""Symbole der Sammlungen: Pfad -> Datei, Anschlüsse mit Lage, Name und UUID.

Bausteine setzen Symbole über einen Anker-Anschluss („Anschluss 3 dieses Symbols liegt bei x/y“), damit
Leitungen ohne Knick laufen. Leitungen hängen an der Anschluss-UUID (eindeutig, auch bei gleichem Namen).
"""
import functools
import os
import xml.etree.ElementTree as ET

DATEN = os.environ.get("QET_DATEN", r"C:\Users\test\Documents\Code Projekte\QElectroTech\Daten")
COMPANY = os.environ.get("QET_COMPANY_ELEMENTS_DIR", os.path.join(DATEN, "elements-company"))
COMMON = os.environ.get("QET_COMMON_ELEMENTS_DIR",
                        r"C:\Users\test\Documents\Code Projekte\qelectrotech-eplan\build\deploy\elements")


def datei(pfad):
    if pfad.startswith("company://"):
        return os.path.join(COMPANY, pfad[len("company://"):])
    if pfad.startswith("common://"):
        return os.path.join(COMMON, pfad[len("common://"):])
    return pfad


class Anschluss:
    __slots__ = ("name", "x", "y", "richtung", "uuid", "index")

    def __init__(self, name, x, y, richtung, uuid, index):
        self.name, self.x, self.y, self.richtung, self.uuid, self.index = name, x, y, richtung, uuid, index


class Symbol:
    def __init__(self, pfad):
        self.pfad = pfad
        root = ET.parse(datei(pfad)).getroot()
        self.link_type = root.get("link_type") or "simple"
        # Mitte des Symbols relativ zum Einfügepunkt (für Texte mittig unter Kästen)
        self.mitte_x = float(root.get("width", 0)) / 2 - float(root.get("hotspot_x", 0))
        roh = [(a.get("name") or "", float(a.get("x")), float(a.get("y")), a.get("orientation"), a.get("uuid"))
               for a in root.iter("terminal") if a.get("x") is not None]   # nicht <plcIO><terminal> der SPS-Karte
        # QETs Index-Reihenfolge: oben -> unten, dann links -> rechts
        roh.sort(key=lambda a: (round(a[2], 1), round(a[1], 1)))
        self.anschluesse = [Anschluss(n, x, y, o, u, i) for i, (n, x, y, o, u) in enumerate(roh)]
        # Index der Texte in Dateireihenfolge (so zählt QETs set_element_text): Benutzerfelder und Kennzeichen
        dyn = list(root.iter("dynamic_text"))
        self.feld_index = [i for i, t in enumerate(dyn) if t.get("text_from") == "UserText"]
        self.feld_vorgabe = [(t.findtext("text") or "") for t in dyn if t.get("text_from") == "UserText"]
        self.felder = len(self.feld_index)
        self.label_index = next((i for i, t in enumerate(dyn) if t.get("text_from") == "ElementInfo"
                                 and (t.findtext("info_name") or "") == "label"), 0)
        # Lage des Kennzeichens (für das automatische Freistellen in Blatt.setze): x, y, Schriftgröße, Ausrichtung
        lab = dyn[self.label_index] if dyn and self.label_index < len(dyn) else None
        if lab is not None and lab.get("text_from") == "ElementInfo":
            import re as _re
            g = _re.search(r",([\d.]+),", lab.get("font") or "")
            self.label_lage = (float(lab.get("x", 0)), float(lab.get("y", 0)), float(g.group(1)) if g else 9.0,
                               lab.get("Halignment") or "AlignLeft")
        else:
            self.label_lage = None

    def anschluss(self, wer):
        """wer: Name ('A1', '3'), Index (int) oder ('name', n) für den n-ten gleichnamigen."""
        if isinstance(wer, int):
            return self.anschluesse[wer]
        if isinstance(wer, tuple):
            name, n = wer
            return [a for a in self.anschluesse if a.name == name][n]
        treffer = [a for a in self.anschluesse if a.name == wer]
        if not treffer:
            raise KeyError(f"{self.pfad}: kein Anschluss {wer!r} (hat {[a.name for a in self.anschluesse]})")
        return treffer[0]


@functools.lru_cache(maxsize=None)
def symbol(pfad):
    return Symbol(pfad)


C = "common://10_electric/10_allpole/"
U = "company://"
# Normsymbole des Rasters (Polabstand 30 px)
MOTORSCHUTZ = U + "Allgemein/IEC_Polabstand_30/dis_mag_term_3f-2.elmt"        # 1/3/5 oben, 2/4/6 unten, Master
HAUPTKONTAKT = U + "Allgemein/IEC_Polabstand_30/com_puiss4.elmt"              # 3 Schließer, Slave der Spule
SPULE = C + "310_relays_contactors_contacts/01_coils/bobine3.elmt"            # A1 oben, A2 unten, Master
SCHLIESSER = C + "310_relays_contactors_contacts/02_contacts_cross_referencing/01_auxiliary_contacts/con_simple.elmt"
OEFFNER = C + "310_relays_contactors_contacts/02_contacts_cross_referencing/01_auxiliary_contacts/con_simple_nf.elmt"
MOTOR3 = U + "Allgemein/Motoren/Drehstrommotor_3Anschluss/Drehstrommotor_3_Anschluesse_Plan.elmt"
MOTOR6 = U + "Allgemein/Motoren/Drehstrommotor_6Anschluss/Drehstrommotor_6_Anschluesse_Plan.elmt"
KLEMME = C + "130_terminals_terminal_strips/borne_continuite2.elmt"           # oben/unten, nur Nummer
KLEMME_PE = U + "Allgemein/Klemmen/PE_Durchgangsklemme.elmt"                   # PE-Reihenklemme (Typ ground)
ERDE = C + "110_network_supplies/terre.elmt"                                         # PE-Anschluss am Gerät, Anschluss oben
KOMMEND = U + "Allgemein/Verweise/01coming_arrow_gefuellt.elmt"               # previous_report, Anschluss rechts
GEHEND = U + "Allgemein/Verweise/02going_arrow_gefuellt.elmt"                 # next_report, Anschluss links
KOMMEND_S = U + "Allgemein/Verweise/03coming_arrow_senkrecht.elmt"   # von oben, Verweis links neben der Spitze
GEHEND_S = U + "Allgemein/Verweise/04going_arrow_senkrecht.elmt"     # nach unten, Verweis senkrecht unter der Spitze
ABZWEIG = U + "Allgemein/Verbindungen/Abzweig_Plan/Abzweig_unten_links_Plan.elmt"   # W, E, S
ABZWEIG_OBEN = U + "Allgemein/Verbindungen/Abzweig_Plan/Abzweig_oben_rechts_Plan.elmt"  # W, E, N (Abgang oben)
ECKE = U + "Allgemein/Verbindungen/Abzweig_Plan/Ecke_Plan.elmt"               # N, W, E, S am selben Punkt
DI_KANAL = "company://Siemens/SPS/6ES7131-6BH01-0BA0/lesbar/6ES7131-6BH01-0BA0_DI_B1.elmt"          # DI oben
DQ_KANAL = "company://Siemens/Steuerungen/6ES7132-6BH00-0AA0/lesbar/6ES7132-6BH00-0AA0_DQ_C1.elmt"  # DQ unten
SCHALTER_FELD = U + "Allgemein/Sensoren/Druckschalter_Plan/Druckschalter_13-14_Plan.elmt"   # 13 oben, 14 unten
SANFTSTARTER_3RW40 = U + "Siemens/Schaltgeräte/3RW4047-1BB04/3RW4047-1BB04.elmt"   # Master; 1/L1..5/L3, A1, 1 oben
LASTTRENNSCHALTER = U + "Allgemein/IEC_Polabstand_30/lasttrennschalter_3p.elmt"  # 1/3/5 oben, 2/4/6 unten, Master
STROMWANDLER = C + "340_converters_inverters/20_current_tansformers/transformateur_courant1.elmt.elmt"  # primär oben/unten, sekundär rechts
PAC3220 = U + "Siemens/Steuerungen/7KM3220-1BA01-1EA0/7KM3220-1BA01-1EA0_+-l_A1.elmt"   # V1..VN, L, N oben; IL unten
DI_AUX_KANAL = "company://Siemens/SPS/6ES7131-6BH01-0BA0/lesbar/6ES7131-6BH01-0BA0_DI-AUX_C1.elmt"  # DI, AUX oben
NETZTEIL_3P_40A = U + "Siemens/Spannungsquelle und Generator/6EP3437-7SB00-3AX0/6EP3437-7SB00-3AX0.elmt"
SEL1200 = {8: U + "Siemens/Spannungsquelle und Generator/6EP4438-7FB00-3DX0/6EP4438-7FB00-3DX0.elmt",
           4: U + "Siemens/Spannungsquelle und Generator/6EP4437-7FB00-3CX0/6EP4437-7FB00-3CX0.elmt"}
F_DI_VS_KANAL = "company://Siemens/SPS/6ES7136-6BA01-0CA0/lesbar/6ES7136-6BA01-0CA0_DI-VS_B1.elmt"   # DI, VS oben (F-DI)
F_DI_KANAL = "company://Siemens/SPS/6ES7136-6BA01-0CA0/lesbar/6ES7136-6BA01-0CA0_DI_C1.elmt"         # DI oben (OSSD)
NOTHALT_2NC = U + "Allgemein/Befehlsgeräte/Not-Halt_2NC/Not-Halt-Taster_2_Oeffner_Plan.elmt"     # 11/12, 21/22
SCHUTZTUER_2NC = U + "Allgemein/Befehlsgeräte/Schutztuerschalter_2NC/Schutztuerschalter_2_Oeffner_Plan.elmt"
FI_LS = U + "Allgemein/IEC_Polabstand_30/fi_ls_1p_n.elmt"                                 # 1/N oben, 2/N. unten
LS_1P = U + "Allgemein/Schutzeinrichtungen/LS_1P_Plan/Leitungsschutzschalter_1P_Plan.elmt"   # 1 oben, 2 unten
STECKDOSE = U + "Allgemein/Steckdosen/Schuko_1P_N_PE/Steckdose_1P+N+PE.elmt"                # L, N, PE oben
LUEFTER = U + "Allgemein/Lüfter/Filterlüfter_230VAC/Filterluefter_230V_AC_235m_h.elmt"      # L, N oben, PE
LEUCHTE = C + "380_signaling_operating/11_optical_signaling/lampe2.elmt"                    # A1 oben, A2 unten
THERMOSTAT = U + "Finder/Thermostate/7T.81.0.000.2303/Thermostat_7T.81_Schliesser.elmt"     # 13 oben, 14 unten
SCHALTER_1NO = U + "Allgemein/Befehlsgeräte/Drucktaster_1NO/Drucktaster_1_Schliesser_Plan.elmt"  # 13 oben, 14 unten

# Kanalmakros in lesbarer Größe (symbolbau.lesbar, Faktor 1,5; Original-Makros sind mit 5-pt-Schrift zu klein)
