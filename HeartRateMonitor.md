# WorkSense HR — Sensor de Ritmo Cardíaco para Puestos de Trabajo (BPM • Voz • Alertas)

> **Idea:** Un mouse “consciente” del estado fisiológico de la persona. El sensor MAX3010x (en el mouse o cerca de la mano) mide el pulso y el ESP32 **emite sonidos** y, opcionalmente, **voz** generada por un servidor local. **Objetivo:** avisar cuando el ritmo está **bajo** o **alto**, guiar una respiración breve y registrar el mensaje/URL del audio para auditoría.

---

## 🧭 Pitch (30 s)

**WorkSense HR** transforma el pulso en señales **claras y accionables**: semáforo de color, beeps y mensajes hablados (“inhala 4, sostén 2, exhala 6”). Cuando el BPM sale de rango, el sistema **alerta de forma continua** hasta volver a valores normales. Un servidor FastAPI genera audios en español y devuelve el **URL**; el ESP32 imprime en serie **qué dijo** y **dónde quedó el archivo**.

---

## 👀 Qué muestra / hace en tiempo real

* **Estado general (NeoPixel):**

  * 🟢 Verde: BPM **estable** (50–110)
  * 🟡 Amarillo: **precaución** (50> BPM ≥40 o 110< BPM ≤130)
  * 🔴 Rojo: **alerta** (BPM <40 o >130)
* **Sonidos:**

  * ✅ `beep_ok()` al estabilizarse.
  * ⚠️ `beep_warn_low()` / `beep_warn_high()` **cada 2 s** si está fuera de rango (precaución).
  * 🚨 `sirena_alerta()` **cada 1.5 s** si está en alerta.
* **Voz (opcional, vía servidor):** al sostenerse fuera de rango por ventana de 5 s:

  * Mensaje **calmante** (alto/bajo) + reproducción automática.
  * En el **Serial** verás:

    * **Texto** que dijo el audio.
    * **URL** del `.wav` (p. ej. `http://192.168.0.8:8000/out/tts8k_...wav`).

**Nudges rotativos (idea):** “Respiremos 4–2–6”, “Tómate una pausa breve”, “Ajusta postura y hombros”.

---

## 📝 Informe diario (opcional)

Si combinas este módulo con tu *WorkSense Wall*, puedes registrar:

* **Eventos fuera de rango:** hora de inicio/fin, tipo (bajo/alto).
* **Mensajes reproducidos:** texto + URL del audio (trazabilidad).
* **% tiempo estable** por persona/puesto (no médico, orientativo).

> Los CSV y los audios se guardan localmente en el servidor (`/out/`). El informe diario puede redactarse con Gemini (igual que el proyecto de ambiente).

---

## 🔧 Sensores y métricas (hardware)

* **MAX3010x (MAX30102/105)** en el mouse o reposamanos.

  * Pines: `SDA=21`, `SCL=22`.
  * **IR objetivo** con dedo: **\~80k–140k** (ajusta `ledBrightness`/`pulseWidth`).
* **NeoPixel** en `IO2` (1 LED, semáforo).
* **Salida de audio**: `DAC GPIO26` → **amplificador** (p. ej. LM386) → parlante.

  * Acopla con **condensador 0.047–0.1µF** y referencia con \~10k a GND.

---

## 🏗️ Arquitectura

```
[Usuario] ─ mano → Sensor MAX3010x ─(I2C)─ ESP32
    │                       │
    │            Estados/sonidos locales (beeps/sirena)
    │                       │ Wi-Fi
    └────────►  Servidor FastAPI (PC) ──► TTS (pyttsx3+pydub) ─► WAV 8k/8-bit
                                 │
                                 └─ /out/tts8k_*.wav  (URL devuelto + reproducido)
```

---

## 🚀 Instalación rápida (demo local)

### 1) Servidor (PC)

Requisitos: Python 3.10+, **ffmpeg** embebido (lo maneja `imageio-ffmpeg`), salida de audio **8kHz/8-bit/mono**.

```bash
# Clona tu repo y entra a la carpeta del server
cd server

# (opcional) Entorno virtual
python -m venv .venv
# Win
.venv\Scripts\activate
# macOS/Linux
source .venv/bin/activate

pip install fastapi uvicorn pydub imageio-ffmpeg pyttsx3 python-dotenv google-generativeai
```

Crea **`.env`**:

```dotenv
GEMINI_API_KEY=TU_API_KEY
```

Lanza el servidor:

```bash
uvicorn server:app --host 0.0.0.0 --port 8000 --reload
```

Endpoints clave:

* `GET /ping` → `{ "ok": true, "msg": "pong" }`
* `POST /reply` → body `{ "text": "..." }` → `{ "ok": true, "audio_url": "/out/tts8k_....wav" }`
* `POST /coach` → body `{ "bpm": 120, "avg": 118 }` → `{ "ok": true, "text": "...", "audio_url": "/out/..." }`

> El servidor genera audios WAV **8kHz/8-bit/mono** (aptos para el DAC interno del ESP32) y los sirve desde `/out/`.

---

### 2) Firmware en ESP32

* **IDE Arduino** (o PlatformIO).
* **Librerías:**

  * *SparkFun MAX3010x Sensor Library* (incluye `heartRate.h`)
  * *Adafruit NeoPixel*
* **Placa:** “ESP32 Dev Module”.

En el **sketch** (archivo `.ino`) ajusta:

```cpp
const char* SSID        = *******;
const char* PASS        = *******;
const char* SERVER_HOST = "192.169.*.*";
const uint16_t SERVER_PORT = 8000;
```

Cableado rápido:

```
MAX3010x  SDA → GPIO21
          SCL → GPIO22
NeoPixel  DIN → GPIO2
Audio     DAC → GPIO26 → LM386 → Parlante
GNDs comunes — 3V3 a sensor (según módulo)
```

Sube el firmware. Abre el **Serial Monitor** a **115200**.

---

## ⚙️ Umbrales y lógica de alertas

* **Normal:** `50 ≤ BPM ≤ 110` → verde.
* **Precaución:** `40 ≤ BPM < 50` o `110 < BPM ≤ 130` → amarillo + **beep** cada 2 s.
* **Alerta:** `BPM < 40` o `BPM > 130` → rojo + **sirena** cada 1.5 s.
* **Ventana de disparo TTS:** 5 s sostenidos fuera de rango → pedir audio al server.
* **Cooldown de TTS:** 30 s entre mensajes de calma.
* **Serial:** imprime **texto** del TTS y **URL** del WAV (p.ej. `http://192.168.0.8:8000/out/tts8k_...wav`).

---

## ▶️ Uso

1. Ejecuta el **servidor** (ver arriba).
2. Enciende el **ESP32**; espera “WiFi OK…”.
3. **Coloca el dedo** tapando el sensor (evita luz externa).
4. Observa en **Serial**: `IR=... BPM=... Avg BPM=...`
5. Si sales de rango: escucharás **beeps** o **sirena**; tras 5 s, el ESP32 pedirá TTS y **reproducirá** el audio.
6. El **texto** reproducido y la **URL del audio** quedan impresos en el Serial.

---

## 🌐 API (pruebas rápidas)

```bash
# Comprobar servidor
curl http://192.168.0.8:8000/ping

# Generar un audio arbitrario
curl -X POST http://192.168.0.8:8000/reply \
  -H "Content-Type: application/json" \
  -d '{"text":"Respiremos juntas. Inhala cuatro, sostén dos, exhala seis."}'
# → {"ok":true,"audio_url":"/out/tts8k_...wav"}
```

Abre el URL devuelto en un navegador para verificar el WAV.

---

## 🛠️ Solución de problemas

**BPM = 0 siempre**

* Asegura **contacto firme** del dedo y **bloquea luz** ambiente.
* Revisa que `IR` en Serial esté **> 50,000** con el dedo puesto.
* Ajusta en `setup()`:

  * `ledBrightness` (sube si IR bajo; baja si satura),
  * `pulseWidth`,
  * `sampleRate` (100–200 Hz).
* Comprueba cableado SDA/SCL y GND comunes.
* Verifica que usas la **librería SparkFun MAX3010x** correcta (incluye `heartRate.h`).

**Audio suena “sucio” o distorsiona**

* El DAC interno es 8-bit; usa **acoplo por capacitor** y **ganancia moderada** en el LM386.
* Reduce volumen en `playToneDAC(..., volume)` o la ganancia del ampli.
* Revisa GNDs y longitud de cables.
* (Avanzado) Cambiar a salida **I2S** con APLL para audio más limpio.

**No llega el TTS**

* Mira el **log** del servidor (debe aparecer `GET /out/...` tras el `POST /reply`).
* Revisa `SERVER_HOST`/`SERVER_PORT`, firewall y que `/out/` se sirva.
* Comprueba que el server imprime `[FFMPEG]`, y que `pyttsx3` puede sintetizar (en Windows suele necesitar voces instaladas).

---

## 🔒 Privacidad y alcance

* No se envía audio de la persona; solo **BPM** y mensajes TTS.
* Datos de salud **no clínicos**; **no** sustituye evaluación médica.
* Los audios y logs quedan **locales** en tu red.

---

## 🧪 Flujo de demo (2 min)

1. Conectas servidor + ESP32.
2. Pones el dedo y observas **BPM** subir/bajar (hacer respiración rápida/lenta).
3. Sales de rango → **beeps/sirena**; a los 5 s → **voz** con respiración guiada.
4. En **Serial**: texto + URL del WAV. (Abre el enlace para oírlo de nuevo.)

---

## 🗺️ Roadmap

* [ ] Guardar eventos (hora, BPM, texto, URL) en CSV.
* [ ] Enviar alertas a *WorkSense Wall* para correlacionar con ruido/luz.
* [ ] Modo “entrenador” con `/coach` (Gemini) activo.
* [ ] Salida **I2S** para voz más limpia.

---

## 📁 Estructura sugerida del repo

```
/heart-rate/
  firmware/
    esp32_hr_monitor.ino      # este sketch
  server/
    server.py                 # FastAPI + TTS (8k/8-bit/mono)
    .env.example              # plantilla de variables
  docs/
    README_HR.md              # este documento
```

---

## ⚠️ Disclaimer

Prototipo con fines de **bienestar** y **productividad**. **No es** un dispositivo médico ni reemplaza evaluaciones profesionales.

---

## 📜 Licencia

Elige la que uses en el repo (MIT/Apache-2.0/GPL-3.0).
