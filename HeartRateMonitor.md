WorkSense Pulse — Sensor de Ritmo Cardiaco (ESP32 + MAX3010x + TTS)

Biofeedback en el puesto de trabajo. Un sensor de pulso (MAX3010x) embebido en el mouse mide BPM en tiempo real; el ESP32 muestra semáforo (NeoPixel), emite beeps/sirena cuando sales de rango y, si la situación persiste, solicita al server FastAPI un audio de guía (TTS), lo reproduce y muestra en consola el texto y la URL del WAV.

⚠️ No es un dispositivo médico. Prototipo para bienestar y foco. No sustituye evaluaciones clínicas ni de seguridad ocupacional.

🧭 Pitch (30 s)

WorkSense Pulse convierte la señal del MAX3010x en señales claras: OK, precaución y alerta. Si el pulso se mantiene fuera de rango, suena una sirena/beep y el ESP32 solicita al servidor un mensaje de calma que se reproduce por altavoz. En consola verás el texto que se dijo y el link del audio, útil para auditoría y correlación con ambiente (ruido, luz, temperatura) del proyecto WorkSense.

👀 Qué se muestra (Monitor Serie + LED)

Serial (cada ~1.8 s):

IR=102345  BPM=76.5  Avg BPM=77
[TTS] Texto:
Tu ritmo es elevado. Vamos a respirar juntas...
[TTS] URL: http://192.168.0.8:8000/out/tts8k_2025...


NeoPixel (IO2):

Verde: en rango

Amarillo: precaución (ligeramente bajo/alto)

Rojo: alerta (muy bajo/muy alto)

Flash rojo corto: en cada latido detectado

Sonidos:

Precaución: beep doble periódico (grave para bajo, agudo para alto)

Alerta: sirena barrida periódica

TTS: guía breve cuando se sostiene la condición (se imprime texto + URL)

📝 Cómo usar el informe en WorkSense

Indicadores:

% de tiempo en rango (meta ≥80%)

Eventos de BPM alto/bajo (hora y duración)

Minutos en alerta sostenida

Tendencias por franja y por persona/puesto

Acciones ejemplo:

Pausas breves/respiración 4–2–6 en horas de picos

Revisar zona/ruido/luz si se correlacionan con aumentos de BPM

Señalizar “zonas de foco” y derivar llamadas a phone booths

Los datos crudos/eventos pueden integrarse al dashboard WorkSense; este módulo expone tiempos y eventos.

🔧 Hardware (demo)

MAX30102/MAX30105 (I²C) — ritmo cardiaco (IR)

NeoPixel (IO2) — semáforo de estado

Audio: DAC interno del ESP32 (IO26) → LM386 → parlante 8 Ω

0.047–0.1 µF en serie y 10 kΩ a GND en la entrada del LM386 (filtra DC)

Cables cortos y GND común

Conexiones (ESP32):

Señal	Pin
MAX3010x SDA	21
MAX3010x SCL	22
NeoPixel DIN	2
DAC → LM386 IN	26
3V3/VIN + GND	—
🏗️ Arquitectura

ESP32 (Arduino)

Lee IR del MAX3010x; detecta latidos con heartRate.h

Suaviza (RATE_SIZE=4) → beatAvg

Umbrales:

Precaución: 50–110 BPM

Alerta: <40 o >130 BPM

Si permanece fuera de rango ≥5 s:

Beep/Sirena periódicos (mientras siga fuera de rango)

Solicita TTS al server (POST /reply), reproduce WAV (8 kHz/8-bit/mono)

Imprime el texto y la URL del audio

Server (FastAPI/Python)

POST /reply: recibe texto → pyttsx3+pydub → WAV 8 kHz/8-bit mono

POST /coach (opcional): genera texto con Gemini según BPM y luego TTS

Sirve audios en /out/<fname>

🚀 Instalación rápida (demo local)
1) Server (PC)

Dependencias

pip install fastapi uvicorn[standard] python-dotenv pydub imageio-ffmpeg pyttsx3 google-generativeai


.env (opcional, solo para /coach)

GEMINI_API_KEY=TU_API_KEY


Arranque

uvicorn server:app --host 0.0.0.0 --port 8000 --reload


Prueba: http://<IP_PC>:8000/ping → {"ok":true,"msg":"pong"}

Ejemplo cURL

curl -X POST http://<IP_PC>:8000/reply \
  -H "Content-Type: application/json" \
  -d '{"text":"Prueba de voz desde el servidor."}'

2) Firmware (ESP32)

En el sketch ajusta:

const char* SSID = "ARRIS-5308";
const char* PASS = "50A5DC0A5308";
const char* SERVER_HOST = "192.168.0.8";
const uint16_t SERVER_PORT = 8000;


Compila/flash y abre Monitor Serie @115200.

🧠 Reglas de decisión (firmware)

OK (50 ≤ Avg ≤ 110): LED verde, sin sonido

Precaución (40–50 o 110–130): LED amarillo, beeps cada 2 s

Alerta (<40 o >130): LED rojo, sirena cada 1.5 s

Si fuera de rango ≥ 5 s → pedir TTS (cooldown 30 s), imprimir texto y URL, reproducir audio

Los beeps/sirena continúan mientras permanezca fuera de rango; el TTS tiene cooldown para no saturar.

⚙️ Parámetros clave
// Detección
const byte RATE_SIZE = 4;        // ventana de suavizado
const int  sampleRate = 200;     // MAX3010x
const int  pulseWidth = 215;     // energía de pulso

// Umbrales
const int BPM_LOW  = 50;
const int BPM_HIGH = 110;
const int BPM_ALERT_LOW  = 40;
const int BPM_ALERT_HIGH = 130;

// Ventanas / períodos
const unsigned long WINDOW_MS             = 5000;
const unsigned long ALERT_COOLDOWN_MS     = 30000;
const unsigned long WARN_BEEP_PERIOD_MS   = 2000;
const unsigned long ALERT_SIREN_PERIOD_MS = 1500;

🧪 Flujo de demo (2 min)

Coloca el dedo; verás BPM en serie y flashes rojos por latido.

Simula alto BPM (movimiento, apretar dedo) → amarillo/rojo + beeps/sirena.

Tras ~5 s, el ESP32 pide TTS → suena voz; en serie: Texto y URL.

Vuelve a rango → verde y silencio.

🔒 Privacidad

No se transmite audio crudo ni datos personales.

Solo se usan métricas agregadas y mensajes breves.

🧰 Solución de problemas

BPM = 0 / “No finger?”

Tapa bien el sensor; evita luz ambiental directa.

Ajusta ledBrightness (sube a 0x80) o pulseWidth=411 si IR es bajo; baja si satura.

IR típico con dedo: 60k–140k.

Audio distorsionado/zumbido

Revisa 0.047–0.1 µF en serie y 10 kΩ a GND en la entrada del LM386.

Cables cortos; evita rutas cerca de la antena Wi-Fi.

Este diseño usa 8 kHz/8-bit por DAC; para más calidad, migrar a I2S.

No reproduce pero imprime URL

Verifica que el ESP32 alcance http://SERVER_HOST:PORT/out/... en la misma red.

Server corriendo (puerto 8000) y firewall abierto.

🧩 Políticas (YAML, ejemplo futuro)
policies:
  bpm_high:
    over: 110
    for_seconds: 5
    actions:
      - beep_warn_high_every: 2s
      - tts_reply: "Tu ritmo es elevado..."
  bpm_low:
    under: 50
    for_seconds: 5
    actions:
      - beep_warn_low_every: 2s
      - tts_reply: "Tu ritmo es bajo..."
  bpm_alert_high:
    over: 130
    actions:
      - siren_every: 1.5s
      - tts_cooldown: 30s
  bpm_alert_low:
    under: 40
    actions:
      - siren_every: 1.5s
      - tts_cooldown: 30s

📊 KPI para RR. HH./Workplace

% tiempo en rango (por puesto/persona)

Eventos alto/bajo (hora y duración)

Minutos en alerta

Tendencias por día/semana; correlación con ruido/luz/temperatura (WorkSense)

🗺️ Roadmap

Perfilado personal (rango adaptativo)

Reproductor I2S (mejor audio)

Exportar eventos a CSV/JSON para el tablero principal

UI de calibración (umbral, brillo, PWM)

📁 Estructura sugerida del repo
/worksense-pulse/
  /firmware/
    esp32_max3010x_pulse_tts.ino
  /server/
    server.py
    .env.example
    requirements.txt
  /docs/
    README.md


requirements.txt

fastapi
uvicorn[standard]
python-dotenv
pydub
imageio-ffmpeg
pyttsx3
google-generativeai

⚠️ Disclaimer

Proyecto de prototipo para bienestar y foco en oficinas. No es instrumento médico, ni diagnóstico, ni tratamiento. Usar bajo supervisión y respetar políticas de seguridad y privacidad vigentes.
