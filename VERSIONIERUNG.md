# Versionierung des Forks

Es gibt **einen** Stand: den Branch `main`. Er liegt identisch in beiden Repositories:

| Remote | Repository | Branch | Tags |
|---|---|---|---|
| `meko` | MeKo-Tech/Eplan-Software-mit-MCP (privat) | `main` | `v1.0.0`, `v1.1.0`, … |
| `origin` | Birger90/qelectrotech-eplan (öffentlich) | `main` | dieselben |
| `upstream` | qelectrotech/qelectrotech-source-mirror | `master` | wird nur geholt |

Jeder Push geht in beide Repos: `git push meko main` und `git push origin main`. Tags werden ebenfalls in beide
gepusht, jeder **einzeln** (`git push meko vX.Y.Z` und `git push origin vX.Y.Z`).

## Was nicht in den Fork gehört

Kundennamen, Auftrags- und Plannummern, Kundenpläne und Herstellerdateien gehören nicht in den Fork. Sie bleiben
im privaten Daten-Repository (`QElectroTech\Daten`, nur auf dem NAS):

- Der EPLAN-Import wird in `Daten\vorlagen` entwickelt. Im Fork steht unter `tools/eplan_import` nur die
  anonymisierte Kopie (Projekt A, B, … statt Plannummern; ohne `groundtruth.py`). Sie entsteht mit
  `python Daten\fork_export.py <fork> public` (das Skript liegt nur im Daten-Repo, weil es die Plannummern kennt).
- Vor jedem Push prüfen:

```bash
git grep -I -n -E "COMA|Fliegl|Andritz|(2[0-9])[ _]37[ _][0-9]{3}|192\.168\." -- . ":!elements" ":!lang"
```

Kein Treffer, dann pushen. Die Historie vor v1.0.0 enthält solche Daten. Sie liegt nur auf meko, als Tag
`archiv/privat-vor-1.0.0`, und kommt nie nach `origin`.

## Versionsnummer

`vMAJOR.MINOR.PATCH` nach [SemVer](https://semver.org/lang/de/):

- **MINOR** steigt mit jeder neuen Funktion, z. B. einem neuen Menü, Import oder Werkzeug.
- **PATCH** steigt mit Fehlerbehebungen ohne neue Funktion.
- **MAJOR** steigt, wenn ältere Fork-Projekte oder -Bauteile nicht mehr ohne Anpassung gehen.

Die Nummer steht an zwei Stellen. Beide werden zusammen mit dem Tag geändert:

1. `sources/qetversion.cpp`, Funktion `forkVersion()`. Sie erscheint im Info-Dialog und bei `--version` als
   „0.200.1-dev (Fork 1.0.0)“. Projektdateien speichern weiter nur die QET-Version.
2. `CHANGELOG-FORK.md`, ein neuer Abschnitt oben.

## Ablauf

1. **Neue Arbeit:** `git switch -c feature/<thema> main`. Für einen eigenen Ordner:
   `git worktree add ../qelectrotech-eplan-<thema> -b feature/<thema> main`. Feature-Branches bleiben lokal
   oder liegen auf meko.
2. **Fertig:** gebaut und getestet → `git switch main` → `git merge --no-ff feature/<thema>` → in beide Repos
   pushen → den Feature-Branch und den Worktree löschen.
3. **Release:** Nummer in `forkVersion()` und `CHANGELOG-FORK.md` erhöhen, committen,
   `git tag -a vX.Y.Z -m "…"`, Branch und Tag in beide Repos pushen.
4. **Original nachziehen** (montags): `git fetch upstream` → in `main` `git merge upstream/master` → bauen →
   in beide Repos pushen.
