# REEL 15 – Vintage Tape Machine

Bandmaschinen-Emulation, Teil der *G Audio Vintage Series*
(gleiches Design wie DUSTBOX 12 und SPRINGTANK).
Formate: **VST3** (Windows/macOS), **AU** (macOS), **Standalone**.

## Signalweg

```
Input -> Pre-Emphasis -> Bandsättigung -> De-Emphasis
      -> Bandtransport (Wow, Flutter, Tape-Stop)
      -> Head Bump + Bassverlust -> Rauschen -> Höhenverlust -> Dropouts -> Output
```

## Bedienung

**Bandgeschwindigkeit (7.5 / 15 / 30 IPS):** Bestimmt Head-Bump-Frequenz
(45 / 75 / 120 Hz) und Höhengrenze (9 / 15 / 19,5 kHz). 7.5 klingt wie
Kassette, 30 wie eine Studiomaschine.

| Regler | Funktion |
|---|---|
| INPUT | Aufnahmepegel, mehr Pegel = mehr Sättigung |
| SATURATE | Bandsättigung (asymmetrisch, dadurch geradzahlige Obertöne). Die Höhen sättigen durch Pre-/De-Emphasis stärker, wie auf echtem Band |
| HEAD BUMP | Bassanhebung durch den Tonkopf |
| TONE | Höhen dunkler/heller |
| HISS | Bandrauschen |
| WOW | Langsame Tonhöhenschwankung (Leiern) |
| FLUTTER | Schnelle Tonhöhenschwankung (Flattern) |
| WEAR | Abgenutztes Band: zufällige Pegel-Dropouts und Höhenverlust |
| MIX / OUTPUT | Dry/Wet (latenzkompensiert) und Ausgangspegel |
| **STOP** | Tape-Stop: Das Band bremst in ca. 0,9 s bis zum Stillstand ab und läuft beim Lösen wieder an. Als Parameter **automatisierbar**, z. B. am Ende eines Drops |

Die Spulen im Kopfbereich drehen sich je nach Bandgeschwindigkeit und werden
beim Tape-Stop langsamer. Das VU-Meter zeigt den Ausgangspegel.

Das Plugin meldet 6 ms Latenz an den Host. Die Latenz wird automatisch ausgeglichen.

## Werkspresets

01 Studio 15 · 02 Hot 30 IPS · 03 Cassette Deck · 04 Lo-Fi Memory ·
05 Warped VHS · 06 Mastering Glue · 07 Broken Walkman · 08 Init (Clean)

## Bauen

**GitHub Actions:** Projekt in ein Repo pushen, dann unter *Actions → Build REEL 15*
die Artefakte für Windows und macOS herunterladen.

**Lokal:**
```
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## Installation

- **Windows:** `REEL 15.vst3` nach `C:\Program Files\Common Files\VST3\`
- **macOS:** `REEL 15.vst3` nach `~/Library/Audio/Plug-Ins/VST3/`,
  `REEL 15.component` nach `~/Library/Audio/Plug-Ins/Components/`.
  Nicht signiert: `xattr -cr ~/Library/Audio/Plug-Ins/VST3/"REEL 15.vst3"`

## Lizenz-Hinweis

JUCE 8 ist dual lizenziert (AGPLv3 oder kommerziell). Vor einem Closed-Source-Verkauf
die Bedingungen der JUCE-Lizenzen auf juce.com prüfen.
