#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include <Adafruit_NeoPixel.h>
#include <math.h>
#include <string.h>

// ====== AJUSTA ESTO ======
const char* SSID        = "ARRIS-5308";
const char* PASS        = "50A5DC0A5308";
const char* SERVER_HOST = "192.168.0.8";    // IP/host del server FastAPI
const uint16_t SERVER_PORT = 8000;          // Puerto FastAPI
// ==========================

// --- Pines ---
#define SDA_PIN 21
#define SCL_PIN 22
#define NEOPIXEL_PIN 2
#define NEOPIXEL_COUNT 1
#define DAC_PIN 26  // GPIO 26 -> LM386 IN

// --- Sensor & NeoPixel ---
MAX30105 particleSensor;
Adafruit_NeoPixel strip(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// --- BPM smoothing ---
const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;
float beatsPerMinute = 0;
int   beatAvg = 0;

// --- Log & estado ---
unsigned long lastPrint = 0;

// --- Umbrales de alerta ---
const int BPM_LOW        = 50;   // bajo
const int BPM_HIGH       = 110;  // alto
const int BPM_ALERT_LOW  = 40;   // muy bajo
const int BPM_ALERT_HIGH = 130;  // muy alto

// Ventanas y cooldowns para TTS (mensaje de calma)
const unsigned long WINDOW_MS          = 5000;   // tiempo sostenido fuera de rango para disparar TTS
const unsigned long ALERT_COOLDOWN_MS  = 30000;  // 30 s entre TTS

unsigned long outOfRangeSince = 0;
bool wasOutOfRange = false;
unsigned long lastAlertMs = 0;

// Beeps continuos mientras esté fuera de rango
unsigned long lastWarnBeepMs  = 0;
unsigned long lastAlertBeepMs = 0;
const unsigned long WARN_BEEP_PERIOD_MS  = 2000; // cada 2 s
const unsigned long ALERT_SIREN_PERIOD_MS = 1500; // cada 1.5 s

// --- Utilidades LED ---
void flashNeo(uint8_t r, uint8_t g, uint8_t b, uint16_t on_ms) {
  strip.setBrightness(48);
  strip.setPixelColor(0, strip.Color(r, g, b));
  strip.show();
  delay(on_ms);
  strip.setPixelColor(0, strip.Color(0, 20, 0)); // verde suave estable
  strip.show();
}
void setNeo(uint8_t r, uint8_t g, uint8_t b, uint8_t bright=48) {
  strip.setBrightness(bright);
  strip.setPixelColor(0, strip.Color(r, g, b));
  strip.show();
}

// ---------- Beeps (DAC simple 8 kHz) ----------
void playToneDAC(float freq_hz, uint16_t dur_ms, uint8_t volume = 110) {
  if (volume > 127) volume = 127;
  const float twoPi = 6.28318530718f;
  const float dt_us = 125.0f;                         // 8 kHz
  const float step  = twoPi * freq_hz * (dt_us / 1e6f);
  unsigned long t0 = micros();
  float phase = 0.0f;
  unsigned long endt = millis() + dur_ms;
  while ((long)(millis() - endt) < 0) {
    uint8_t s = 128 + (int)(volume * sinf(phase));
    dacWrite(DAC_PIN, s);
    phase += step;
    if (phase > twoPi) phase -= twoPi;
    t0 += (unsigned long)dt_us;
    long wait = (long)t0 - (long)micros();
    if (wait > 0) delayMicroseconds((uint32_t)wait);
    else t0 = micros();
  }
  dacWrite(DAC_PIN, 128);
}
void beep_warn_low()  { playToneDAC(500.0f, 220, 120); }
void beep_warn_high() { playToneDAC(1000.0f, 220, 120); }
void sirena_alerta(uint16_t ms = 900) {
  unsigned long endt = millis() + ms;
  while ((long)(millis() - endt) < 0) {
    for (int f=600; f<=1400; f+=50) playToneDAC((float)f, 16, 120);
    for (int f=1400; f>=600; f-=50) playToneDAC((float)f, 16, 120);
  }
}

// ---------- Reproduce WAV 8kHz/8-bit/mono desde URL ----------
bool playWav8k8bitFromUrl(const String& url) {
  HTTPClient http;
  if (!http.begin(url)) { Serial.println("[AUDIO] http.begin() falló"); return false; }
  int code = http.GET();
  if (code != 200) {
    Serial.printf("[AUDIO] GET %s => %d\n", url.c_str(), code);
    http.end();
    return false;
  }
  WiFiClient* stream = http.getStreamPtr();
  uint8_t header[44];
  int readH = stream->readBytes((char*)header, 44);
  if (readH != 44) { Serial.println("[AUDIO] Header corto"); http.end(); return false; }
  if (memcmp(header, "RIFF", 4) != 0 || memcmp(header + 8, "WAVE", 4) != 0) {
    Serial.println("[AUDIO] No es WAV"); http.end(); return false;
  }

  const unsigned long usPerSample = 125; // 8 kHz
  uint8_t sample;
  dacWrite(DAC_PIN, 128);
  unsigned long t0 = micros();

  while (http.connected()) {
    if (stream->available()) {
      int b = stream->read();
      if (b < 0) break;
      sample = (uint8_t)b;
      dacWrite(DAC_PIN, sample);
      t0 += usPerSample;
      long toWait = (long)t0 - (long)micros();
      if (toWait > 0) delayMicroseconds((uint32_t)toWait);
      else t0 = micros();
    } else {
      delay(1);
      if (!stream->connected()) break;
    }
  }
  http.end();
  dacWrite(DAC_PIN, 128);
  return true;
}

// ---------- /reply: pide TTS, imprime TEXTO y URL, y reproduce ----------
bool askServerForTTS(const String& text, String& outAudioUrl) {
  HTTPClient http;
  String endpoint = String("http://") + SERVER_HOST + ":" + String(SERVER_PORT) + "/reply";
  if (!http.begin(endpoint)) {
    Serial.println("[TTS] http.begin() falló");
    return false;
  }
  http.addHeader("Content-Type", "application/json");

  String safe = text; safe.replace("\\", "\\\\"); safe.replace("\"", "\\\"");
  String body = String("{\"text\":\"") + safe + "\"}";
  int code = http.POST(body);
  String payload = http.getString();
  http.end();

  if (code != 200) {
    Serial.printf("[TTS] POST /reply => %d\n", code);
    return false;
  }

  int p = payload.indexOf("/out/");
  if (p < 0) {
    Serial.println("[TTS] No se encontró audio_url en respuesta");
    return false;
  }
  int q = payload.indexOf("\"", p);
  if (q < 0) q = payload.length();
  String path = payload.substring(p, q);
  outAudioUrl = String("http://") + SERVER_HOST + ":" + String(SERVER_PORT) + path;

  // Lo que pediste: imprimir el TEXTO que se dice y la URL del audio
  Serial.println("[TTS] Texto:");
  Serial.println(text);
  Serial.print("[TTS] URL: ");
  Serial.println(outAudioUrl);

  // Reproducir el audio
  playWav8k8bitFromUrl(outAudioUrl);
  return true;
}

// Dispara mensaje de calma (TTS)
void triggerCalmMessage(bool tooHigh) {
  if (millis() - lastAlertMs < ALERT_COOLDOWN_MS) return;
  lastAlertMs = millis();

  String msg = tooHigh
    ? "Tu ritmo es elevado. Vamos a respirar juntas. Inhala por cuatro, sostén dos, exhala por seis. Estoy contigo, todo va a estar bien."
    : "Tu ritmo es bajo. Si te sientes mareada, siéntate y respira lento. Estoy contigo, todo va a estar bien.";

  String url;
  if (!askServerForTTS(msg, url)) {
    Serial.println("[ALERTA] No se pudo obtener TTS");
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);

  strip.begin(); strip.setBrightness(32); strip.show();

  // WiFi
  Serial.printf("Conectando a %s ...\n", SSID);
  WiFi.begin(SSID, PASS);
  uint8_t tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 40) { delay(250); Serial.print("."); tries++; }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WiFi OK, IP: "); Serial.println(WiFi.localIP());
    setNeo(0,0,32); // azul
  } else {
    Serial.println("WiFi NO conectado (continuo sin audio TTS)");
    setNeo(32,0,0);
  }

  // I2C
  Wire.begin(SDA_PIN, SCL_PIN);

  // Sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD)) {
    Serial.println("MAX3010x no detectado. Revisa cableado/alimentación.");
    setNeo(32,0,0);
    while (1) delay(1000);
  }

  // Config recomendada (ajusta si satura o queda muy bajo)
  byte ledBrightness = 0x60; // mantener IR ~80k-140k con el dedo
  byte sampleAverage = 8;
  byte ledMode       = 2;    // Red + IR
  int  sampleRate    = 200;  // 100–200 Hz va bien
  int  pulseWidth    = 215;
  int  adcRange      = 16384;

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
  particleSensor.setPulseAmplitudeRed(ledBrightness);
  particleSensor.setPulseAmplitudeIR(ledBrightness);
  particleSensor.setPulseAmplitudeGreen(0x00);
  particleSensor.clearFIFO();

  Serial.println("Coloca el dedo cubriendo el sensor (bloquea luz).");

  // Centro DAC
  dacWrite(DAC_PIN, 128);

  // LED listo (verde)
  setNeo(0,20,0);
}

void loop() {
  // Lectura simple (si te vuelve a dar 0 BPM, cambia a la API con check()/available()/nextSample())
  long irValue = particleSensor.getIR();

  // Detección de latido
  if (checkForBeat(irValue)) {
    long delta = millis() - lastBeat;
    lastBeat = millis();

    float bpm = 60.0 / (delta / 1000.0);
    if (bpm > 40 && bpm < 180) {
      beatsPerMinute = bpm;

      rates[rateSpot++] = (byte)beatsPerMinute;
      rateSpot %= RATE_SIZE;

      int sum = 0;
      for (byte i = 0; i < RATE_SIZE; i++) sum += rates[i];
      beatAvg = sum / RATE_SIZE;

      flashNeo(32, 0, 0, 25);
    }
  }

  // Log cada ~1.8s
  if (millis() - lastPrint >= 1800) {
    if (irValue < 50000) Serial.print("No finger? ");
    Serial.print("IR=");        Serial.print(irValue);
    Serial.print("  BPM=");     Serial.print(beatsPerMinute, 1);
    Serial.print("  Avg BPM="); Serial.println(beatAvg);
    lastPrint = millis();
  }

  // Estados
  bool warnNow  = (beatAvg != 0) && ((beatAvg < BPM_LOW  && beatAvg >= BPM_ALERT_LOW) ||
                                     (beatAvg > BPM_HIGH && beatAvg <= BPM_ALERT_HIGH));
  bool alertNow = (beatAvg != 0) && (beatAvg < BPM_ALERT_LOW || beatAvg > BPM_ALERT_HIGH);

  // LED base
  if (alertNow) setNeo(32,0,0);       // rojo
  else if (warnNow) setNeo(32,24,0);  // amarillo
  else setNeo(0,20,0);                // verde

  // Beeps continuos mientras esté fuera de rango
  if (alertNow) {
    if (millis() - lastAlertBeepMs >= ALERT_SIREN_PERIOD_MS) {
      lastAlertBeepMs = millis();
      sirena_alerta(900);
    }
  } else if (warnNow) {
    if (millis() - lastWarnBeepMs >= WARN_BEEP_PERIOD_MS) {
      lastWarnBeepMs = millis();
      if (beatAvg > BPM_HIGH) beep_warn_high();
      else                    beep_warn_low();
    }
  } else {
    // estable: no sonar
  }

  // Ventana para disparar TTS (no muy frecuente)
  bool outNow = (beatAvg != 0) && (beatAvg < BPM_LOW || beatAvg > BPM_HIGH);
  if (outNow) {
    if (!wasOutOfRange) {
      wasOutOfRange = true;
      outOfRangeSince = millis();
    } else if (millis() - outOfRangeSince >= WINDOW_MS) {
      triggerCalmMessage(beatAvg > BPM_HIGH);
      wasOutOfRange = false; // reinicia ventana (luego aplica cooldown)
    }
  } else {
    wasOutOfRange = false;
  }

  delay(5);
}

