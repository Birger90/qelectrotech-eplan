"""Planvorlagen: Schaltpläne aus wiederverwendbaren Funktionsbausteinen erzeugen (QElectroTech-Fork).

    from planvorlagen import Plan, bausteine as B
    plan = Plan("Presse", ordner, "KUNDE_Projekt_G")
    b = plan.blatt("Ölfilterung Filtermotor", "DA", "CA", 31)
    b.schiene("L1"); b.schiene("L2"); b.schiene("L3")
    B.motorabgang_direkt(b, spalte=0, ...)
    plan.erzeuge("Projekt_G_Vorlagen.qet")
"""
from .blatt import Plan, Blatt, spalte_links, spalte_mitte, BAND
from . import symbole, bausteine
