# ESP32 Bluetooth-Lautsprecher mit MAX98357 (ESP32-A2DP)

Projekt: Ein einfacher Bluetooth‑Lautsprecher auf Basis eines **ESP32** (Classic, WROOM) und eines I2S‑Verstärkers **MAX98357A**.  
Audio wird über **Bluetooth A2DP** (Smartphone / PC) empfangen und per I2S an den MAX98357 ausgegeben.

Verwendete Bibliothek:  
https://github.com/pschatzmann/ESP32-A2DP

---

## Hardware

**Board:**

- Beliebiges **ESP32‑Board mit Bluetooth Classic**  
  (NICHT ESP32‑C3/C2/C6, NICHT ESP32‑S3‑Varianten, da diese nur BLE haben).

**Audio‑Verstärker:**

- Modul **MAX98357A** (I2S‑Digitalverstärker).

**Verkabelung MAX98357A ↔ ESP32**

Pins im Sketch `BT_transiver.ino`:

- I2S:
  - `GPIO 26` → `BCLK` (`BCK`) des MAX98357A  
  - `GPIO 25` → `LRCLK` (`LRC`, `WSEL`)  
  - `GPIO 22` → `DIN` (`SDATA`)  

- Versorgung:
  - `5V` des ESP32 → `VIN` des MAX98357A (falls das Modul für 5 V ausgelegt ist)  
  - `GND` ESP32 → `GND` des MAX98357A  

- Weitere Pins (falls auf dem Modul vorhanden):
  - `SD` (Shutdown / Mode) → typischerweise auf HIGH (3,3 V oder 5 V) für Betrieb  
  - `GAIN` nach Datenblatt beschalten (gewünschte Verstärkung).

- Lautsprecher:
  - `SPK+` / `OUT+` und `SPK-` / `OUT-` an die Lautsprecherklemmen (nicht auf GND legen).

---

## Software

### Abhängigkeiten

1. **Arduino IDE** mit ESP32‑Unterstützung:
   - Über den Board Manager:
     - ESP32‑Boardpaket von Espressif hinzufügen,
     - Boards **ESP32 by Espressif Systems** installieren.

2. Bibliothek **ESP32-A2DP**:  
   https://github.com/pschatzmann/ESP32-A2DP  

   Installation z. B. über  
   **Sketch → Include Library → Add .ZIP Library**  
   oder durch Klonen nach `Documents/Arduino/libraries/ESP32-A2DP`.

### Funktionsweise des Sketches

Die Datei `BT_transiver.ino`:

- bindet `BluetoothA2DPSink` und `driver/i2s.h` ein,
- initialisiert **I2S0** mit:
  - 44,1 kHz, 16 Bit, Stereo, Philips‑I2S‑Format,
- setzt die I2S‑Pins:
  - BCLK = GPIO 26, LRCLK = GPIO 25, DATA = GPIO 22,
- registriert `set_stream_reader(audio_callback, false)`:
  - die Bibliothek dekodiert A2DP‑Audio (SBC) und liefert PCM‑Puffer an `audio_callback`,
  - der Parameter `false` deaktiviert die interne I2S‑Ausgabe der Bibliothek,
- in `audio_callback` werden die PCM‑Daten direkt via `i2s_write` an I2S ausgegeben,
- startet den A2DP‑Sink mit dem Gerätenamen `ESP32_BT_SPEAKER`.

Das Telefon / der PC erkennt den ESP32 als normalen Bluetooth‑Lautsprecher.

---

## Kompilieren und Flashen

1. `BT_transiver.ino` in der Arduino IDE öffnen.
2. Unter **Werkzeuge → Board** ein ESP32‑Board wählen  
   (z. B. `ESP32 Dev Module` oder das passende WROOM‑Board).
3. Den richtigen COM‑Port auswählen.
4. **Upload** starten.

Nach dem Flashen im **Serial Monitor** (115200 Baud) die Ausgaben von Bluetooth/I2S überprüfen.

---

## Verwendung

1. ESP32 und MAX98357A mit Spannung versorgen.
2. Bluetooth am Smartphone / PC aktivieren.
3. Gerät suchen:
   - `ESP32_BT_SPEAKER`
4. Kopplung herstellen.
5. Musik starten (Spotify, YouTube, lokaler Player usw.).
6. Der Ton kommt über den an den MAX98357 angeschlossenen Lautsprecher.

---

## Hinweise und Einschränkungen

- **ESP32‑Typ:**
  - Es wird ein **klassischer ESP32 mit Bluetooth Classic** benötigt.
  - Boards auf Basis von **ESP32‑C3, ESP32‑S3, ESP32‑C2, ESP32‑C6** besitzen nur BLE und **unterstützen kein A2DP** → mit diesen funktioniert `ESP32-A2DP` nicht als Lautsprecher.

- **Codecs:**
  - Die Standard‑Kombination ESP32‑A2DP nutzt **SBC** (Pflicht‑Codec für A2DP).
  - AAC / aptX / aptX HD / LDAC werden in einem typischen Arduino‑Setup nicht verwendet und sind hier nicht konfigurierbar.

- **Watchdog / Abstürze:**
  - Falsche I2S‑Konfiguration oder Verdrahtung kann zu Watchdog‑Resets (`task_wdt`) führen.
  - In diesem Projekt wird die Audioausgabe explizit per `i2s_write` in `audio_callback` realisiert, die interne I2S‑Ausgabe von ESP32‑A2DP ist deaktiviert.

- **Versorgung und Störungen:**
  - MAX98357A reagiert empfindlich auf Versorgungsschwankungen.
  - Für guten Klang: saubere Spannungsversorgung, ordentliche Masseführung, kurze Lautsprecherleitungen.

---

## Stereo mit 2× MAX98357A

Der ESP32 liefert über I2S bereits einen Stereostream (linker und rechter Kanal im selben DIN‑Signal).  
Um aus **zwei** MAX98357A‑Modulen ein Stereo‑System zu bauen, werden beide parallel an I2S gehängt, die Kanalwahl erfolgt über den Pin `SD`.

### Gemeinsame Verbindungen

Für beide MAX98357A identisch:

- `GPIO 26` → `BCLK` (`BCK`)
- `GPIO 25` → `LRCLK` (`LRC`, `WSEL`)
- `GPIO 22` → `DIN` (`SDATA`)
- `VIN` und `GND` → gemeinsame Versorgung

Die Ausgänge (`SPK+/OUT+`, `SPK-/OUT-`) jedes Moduls gehen jeweils auf einen Lautsprecher (links / rechts).

### Kanaltrennung auf Modulen mit SD(L+R)

Viele günstige MAX98357‑Module aus China haben den SD‑Pin über einen internen Widerstand (ca. 1 MΩ) an VIN gelegt und oft mit `SD (L+R)` beschriftet.  
Im Auslieferungszustand liefern sie dann ein Monosignal (L+R‑Mix).

Um echte Trennung in links/rechts zu erreichen, kann man:

- auf jedem Modul einen zusätzlichen Widerstand zwischen `SD` und GND ergänzen,
- zusammen mit dem internen 1‑MΩ‑Widerstand entsteht ein Spannungsteiler,
- abhängig von der resultierenden SD‑Spannung schaltet der Chip in den Modus „Left only“ oder „Right only“ (Details und genaue Schaltschwellen siehe Datenblatt MAX98357A).

Typischer Ansatz:

- für das **linke Modul** einen kleineren Widerstand nach GND wählen, sodass die SD‑Spannung näher am oberen Schaltbereich liegt (z. B. um 2 V bei 5‑V‑Versorgung),
- für das **rechte Modul** einen größeren Widerstand nach GND verwenden, sodass die SD‑Spannung niedriger ist (z. B. um 1 V).

Die genauen Werte hängen vom Modul und der Versorgungsspannung ab und sollten ggf. nachgemessen werden.  
Der Sketch `BT_transiver.ino` muss für dieses Stereo‑Setup nicht geändert werden: I2S ist bereits Stereo konfiguriert, und die PCM‑Daten werden unverändert per `i2s_write` ausgegeben.

---

## Links

- ESP32‑A2DP (Phil Schatzmann):  
  https://github.com/pschatzmann/ESP32-A2DP  
- Dokumentation / Beispiele:  
  https://github.com/pschatzmann/ESP32-A2DP?tab=readme-ov-file  
- Datenblatt MAX98357A: über eine Suche nach *MAX98357A datasheet* auffindbar.
