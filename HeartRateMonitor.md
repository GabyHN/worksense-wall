WorkSense Pulse — Sensor de Ritmo Cardiaco (ESP32 + MAX3010x + TTS)

Biofeedback en el puesto de trabajo. Un sensor de pulso (MAX3010x) embebido en el mouse mide BPM en tiempo real; el ESP32 muestra semáforo (NeoPixel), emite beeps/sirena cuando sales de rango y, si la condición persiste, pide al server FastAPI un audio de guía (TTS), lo reproduce por el DAC interno y deja en consola el texto y la URL del WAV.

⚠️ No es un dispositivo médico. Prototipo para bienestar/foco. No sustituye evaluaciones clínicas ni de seguridad ocupacional.

🗂️ Tabla de contenidos

Pitch (30 s)

Qué verás (LED + Sonidos + Consola)

Arquitectura

Hardware & Cableado

Estructura del repo

Instalación y ejecución

Servidor (FastAPI/TTS)

Firmware (ESP32)

Reglas de decisión y umbrales

API del servidor

Demostración rápida

Privacidad

Solución de problemas

Roadmap

KPI (para RRHH/Workplace)

Licencia / Disclaimer

🧭 Pitch (30 s)

WorkSense Pulse convierte la señal del MAX3010x en señales claras: OK, precaución y alerta. Si el pulso se mantiene fuera de rango, suenan beeps/sirena y el ESP32 solicita al servidor un mensaje de calma que reproduce por altavoz. En consola se imprime el texto y la URL del audio, útil para auditoría y correlación con el ambiente (ruido, luz, temperatura) del proyecto WorkSense Wall.

👀 Qué verás (LED + Sonidos + Consola)

LED (NeoPixel en IO2):

🟢 Verde: en rango

🟡 Amarillo: precaución (ligeramente bajo/alto)

🔴 Rojo: alerta (muy bajo/muy alto)

✴️ Flash rojo corto en cada latido detectado

Sonidos:

Precaución: beep doble periódico (grave para bajo, agudo para alto)

Alerta: sirena barrida periódica

TTS: guía breve cuando se sostiene la condición (voz)

Consola serie (cada ~1.8 s):

IR=102345  BPM=76.5  Avg BPM=77
[TTS] Texto:
Tu ritmo es elevado. Vamos a respirar juntas...
[TTS] URL: http://192.168.0.8:8000/out/tts8k_2025...

🏗️ Arquitectura
MAX3010x ──(I²C: SDA21/SCL22)── ESP32 ── Wi-Fi ──► FastAPI/TTS (PC)
     │                             │
     └─► NeoPixel (IO2)            └─► DAC (IO26) → LM386 → Parlante


ESP32 (Arduino): lee IR, detecta latidos (heartRate.h), suaviza (AVG 4 muestras), decide estado, genera beeps/sirena, solicita /reply y reproduce WAV 8 kHz/8-bit con el DAC interno.

Server (FastAPI): POST /reply crea TTS (pyttsx3+pydub) → WAV mono 8 kHz 8-bit y lo sirve en /out/....
(Opcional) POST /coach usa Gemini para redactar el texto antes del TTS.

🔧 Hardware & Cableado

Componentes

ESP32 (DevKitC u otro con DAC en GPIO 26)

Sensor MAX30102 / MAX30105

NeoPixel 1 LED (IO2)

LM386 + parlante 8 Ω

Condensador 0.047–0.1 µF en serie y resistor 10 kΩ a GND en la entrada del LM386 (filtra DC)

Cables cortos, GND común

Pinout

Señal	ESP32
MAX3010x SDA	21
MAX3010x SCL	22
NeoPixel DIN	2
DAC → LM386 IN	26
3V3/VIN + GND	—
📁 Estructura del repo
/worksense-pulse/
  /firmware/
    esp32_max3010x_pulse_tts.ino
  /server/
    server.py
    requirements.txt
    .env.example
  /docs/
    README.md


server/requirements.txt

fastapi
uvicorn[standard]
python-dotenv
pydub
imageio-ffmpeg
pyttsx3
google-generativeai


.env.example

# Solo necesario si usarás /coach (Gemini)
GEMINI_API_KEY=TU_API_KEY

🚀 Instalación y ejecución
Servidor (FastAPI/TTS)

Instala dependencias:

cd server
python -m venv .venv
source .venv/bin/activate  # en Windows: .venv\Scripts\activate
pip install -r requirements.txt


(Opcional) Copia .env.example a .env y agrega GEMINI_API_KEY si usarás /coach.

Arranca el server:

uvicorn server:app --host 0.0.0.0 --port 8000 --reload


Prueba:

curl http://<IP_PC>:8000/ping
# -> {"ok": true, "msg": "pong"}


Ten el PC y el ESP32 en la misma red. Desactiva firewall para el puerto 8000 si es necesario.

Firmware (ESP32)

Abre esp32_max3010x_pulse_tts.ino y ajusta:

const char* SSID        = "ARRIS-5308";
const char* PASS        = "50A5DC0A5308";
const char* SERVER_HOST = "192.168.0.8";
const uint16_t SERVER_PORT = 8000;


Sube el sketch y abre Monitor Serie @115200.

Coloca el dedo en el sensor y verifica que aparecen BPM y flashes por latido.

🧠 Reglas de decisión y umbrales
// Umbrales
const int BPM_LOW        = 50;     // precaución bajo
const int BPM_HIGH       = 110;    // precaución alto
const int BPM_ALERT_LOW  = 40;     // alerta muy bajo
const int BPM_ALERT_HIGH = 130;    // alerta muy alto

// Ventanas / periodos
const unsigned long WINDOW_MS             = 5000;   // sostener fuera de rango
const unsigned long ALERT_COOLDOWN_MS     = 30000;  // TTS cada 30 s máximo
const unsigned long WARN_BEEP_PERIOD_MS   = 2000;   // beep precaución
const unsigned long ALERT_SIREN_PERIOD_MS = 1500;   // sirena alerta


OK (50–110): LED verde, sin sonido

Precaución (40–50 o 110–130): LED amarillo + beeps cada 2 s

Alerta (<40 o >130): LED rojo + sirena cada 1.5 s

Si fuera de rango ≥ 5 s ⇒ TTS (cooldown 30 s): imprime texto y URL y reproduce voz

🌐 API del servidor
GET /ping

Salud del servidor.

{"ok": true, "msg": "pong"}

POST /reply

Genera TTS desde texto.

Body:

{"text": "Tu ritmo es elevado. Inhala 4, sostén 2, exhala 6."}


Respuesta:

{"ok": true, "audio_url": "/out/tts8k_2025...wav"}

POST /coach (opcional)

Crea texto con Gemini según BPM y luego TTS.

Body:

{"bpm": 125, "avg": 120}


Respuesta:

{"ok": true, "text": "...", "audio_url": "/out/tts8k_..." }

🧪 Demostración rápida

Coloca el dedo; verás BPM y Avg BPM en la consola.

Simula alto BPM (mueve/aprieta dedo) → amarillo/rojo + beeps/sirena.

Tras ~5 s, el ESP32 pide TTS → suena voz; en serie: Texto y URL.

Regresa a rango → verde y silencio.

🔒 Privacidad

No se envía audio crudo ni PII.

Solo métricas agregadas y mensajes breves.

Logs locales para depuración y mejora.

🛠️ Solución de problemas

BPM = 0 / “No finger?”

Cubre bien el sensor; evita luz ambiente directa.

Ajusta ledBrightness (sube a 0x80) o pulseWidth=411 si el IR es bajo; baja si satura.

Objetivo IR con dedo: 60k–140k.

Zumbido/distorsión en audio

Verifica el condensador (0.047–0.1 µF) en serie y 10 kΩ a GND en la entrada del LM386.

Cables cortos y GND común.

Este diseño usa 8 kHz/8-bit por DAC; para mayor calidad, migrar a I2S.

No reproduce pero imprime URL

Asegura que el ESP32 puede acceder a http://SERVER_HOST:PORT/out/... (misma red).

Server activo en puerto 8000 y firewall permitido.

🗺️ Roadmap

Perfil personal (rango adaptativo)

Reproductor I2S (mejor SNR)

Exportar eventos a CSV/JSON para el dashboard

UI de calibración (umbral, brillo, PWM)

📊 KPI (para RRHH/Workplace)

% tiempo en rango por puesto/persona

Eventos alto/bajo (hora y duración)

Minutos en alerta

Tendencias por día/semana; correlación con ruido/luz/temperatura (WorkSense Wall)

📜 Licencia / Disclaimer

Proyecto de prototipo para bienestar y foco en oficinas. No es instrumento médico, diagnóstico ni tratamiento. Úsese bajo supervisión y conforme a políticas de privacidad y seguridad vigentes.
