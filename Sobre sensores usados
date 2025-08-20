# WorkSense Wall — Sensores y Códigos de Prueba (README)

Este README describe **qué sensores usamos**, cómo están **conectados a la IdeaBoard (ESP32 con CircuitPython)** y reúne **los códigos de prueba individuales** que venimos usando para verificar cada sensor por separado.

> **Tip:** en CircuitPython el archivo que corre automáticamente es `code.py`. Para probar cada sensor, **copia uno de los sketches** de abajo dentro de `code.py`, guarda y reinicia (Ctrl+D en Thonny).

---

## 🧩 Sensores utilizados

| Variable                | Sensor / Módulo               | Pin en IdeaBoard | Tipo de lectura   | Notas                                                |
| ----------------------- | ----------------------------- | ---------------: | ----------------- | ---------------------------------------------------- |
| Vibración / Tap         | Módulo “TAP” digital          |          **IO4** | Digital (pull-up) | Pulso LOW cuando hay golpe                           |
| Ruido                   | Micrófono analógico           |         **IO36** | ADC (0–65535)     | Leemos ventana rápida y calculamos p2p y std         |
| Luz                     | LDR (fotoresistor) en divisor |         **IO39** | ADC (0–65535)     | Calibración 5 s y normalización a 0–100% (invertido) |
| Temperatura (tendencia) | NTC en divisor                |         **IO34** | ADC (0–65535)     | Baseline con **EMA** y uso de **delta** relativo     |
| Indicador visual        | NeoPixel onboard              |          **IO2** | Salida            | Semáforo y feedback de estado                        |

**Alimentación:** todos los módulos usan **3V3** y **GND** de la IdeaBoard (se pueden compartir).
**Escala ADC:** CircuitPython entrega valores aprox. **0–65535** para 0–3.3 V.

---

## ⚙️ Requisitos (rápido)

* **CircuitPython** instalado en la IdeaBoard (aparece como unidad **CIRCUITPY**).
* Editor recomendado: **Thonny** (selecciona *CircuitPython (generic) @ COMx*).
* Librería `neopixel` incluida en la build de CircuitPython (suele venir por defecto).

---

## ✅ Códigos de prueba (uno por sensor)

> Copia **uno** de estos bloques en `code.py` para probar ese sensor.
> Todos usan el **NeoPixel (IO2)** para feedback visual.

### 1) TAP (IO4) — detección de golpe

```python
import time, board, digitalio, neopixel

# Tap module en IO4
tap = digitalio.DigitalInOut(board.IO4)
tap.direction = digitalio.Direction.INPUT
tap.pull = digitalio.Pull.UP

# NeoPixel integrado en IO2
pix = neopixel.NeoPixel(board.IO2, 1, brightness=0.3, auto_write=True)
pix[0] = (0, 0, 20)  # azul = reposo

print("Golpea directamente el módulo o la mesa justo debajo...")

while True:
    if not tap.value:  # LOW = golpe
        pix[0] = (30, 0, 0)   # rojo
        print("¡Tap detectado!")
        time.sleep(0.1)       # mantener rojo un instante
        pix[0] = (0, 0, 20)   # vuelve a azul
    time.sleep(0.005)         # lee muy rápido (200 Hz)
```

---

### 2) Micrófono (IO36) — ventana rápida con p2p y std

```python
import time, math
import board, analogio

ADC_PIN = board.IO36
mic = analogio.AnalogIn(ADC_PIN)

# Lee una ventana rápida de N muestras SIN demoras extra
def read_window(adc, n=400):
    vmin = 65535
    vmax = 0
    s1 = 0.0
    s2 = 0.0
    for _ in range(n):
        v = adc.value
        if v < vmin: vmin = v
        if v > vmax: vmax = v
        s1 += v
        s2 += v * v
    mean = s1 / n
    var = max(0.0, (s2 / n) - (mean * mean))
    std = math.sqrt(var)
    p2p = vmax - vmin
    return vmin, vmax, mean, std, p2p

print("Diag mic: habla/aplaude cerca. Mueve el potenciómetro lentamente si hace falta.")
while True:
    vmin, vmax, mean, std, p2p = read_window(mic, n=600)  # ~ventana corta
    # Barras textuales para ver cambios de un vistazo
    bar_p2p = "#" * max(1, min(50, p2p // 500))
    bar_std =  "*" * max(1, min(50, int(std) // 200))
    print(f"min={vmin:5d} max={vmax:5d} mean={int(mean):5d}  std={int(std):5d}  p2p={p2p:5d}  |{bar_p2p}| |{bar_std}|")
    time.sleep(0.1)
```

---

### 3) LDR (IO39) — calibración 5 s y zonas de luz

```python
import time
import board, analogio, neopixel

# Pines en IDEABoard (ESP32)
PIN_LDR = board.IO39      # ADC donde conectaste AO del LDR/módulo
PIN_PIX = board.IO2       # NeoPixel onboard

ldr = analogio.AnalogIn(PIN_LDR)
pix = neopixel.NeoPixel(PIN_PIX, 1, brightness=0.25, auto_write=True)

def leer_prom(adc, n=20, delay=0.002):
    s = 0
    for _ in range(n):
        s += adc.value      # 0..65535 aprox para 0..3.3V
        time.sleep(delay)
    return s // n

print("Calibrando LDR (5 s). Tapa y destapa; usa la linterna si puedes...")
t0 = time.monotonic()
vmin = 65535
vmax = 0
while time.monotonic() - t0 < 5:
    v = leer_prom(ldr, 6)
    if v < vmin: vmin = v
    if v > vmax: vmax = v

span = vmax - vmin
if span < 500:            # por si no variamos mucho en la calibración
    vmin = 0
    vmax = 65535
    span = 65535

print(f"Calibrado: vmin={vmin}, vmax={vmax}")

def pct_norm(v):
    # Normaliza a 0..100% usando límites medidos
    p = (v - vmin) * 100.0 / max(1, span)
    if p < 0: p = 0
    if p > 100: p = 100
    return p

print("Listo. Tapa = 'oscuro' (azul). Linterna = 'deslumbrante' (rojo).")
while True:
    v = leer_prom(ldr, 16)
    p = pct_norm(v)
    p_luz = 100.0 - p   # << INVERSIÓN: 0% = oscuro, 100% = muy iluminado

    # Clasificación por zonas
    if p_luz < 25:
        pix[0] = (0, 0, 30)         # azul
        zona = "oscuro"
    elif p_luz < 70:
        pix[0] = (0, 30, 0)         # verde (confort)
        zona = "confort"
    elif p_luz < 85:
        pix[0] = (30, 15, 0)        # amarillo
        zona = "brillante"
    else:
        pix[0] = (30, 0, 0)         # rojo
        zona = "deslumbrante"

    print(f"LDR_raw={v:5d}  Luz={p_luz:5.1f}%  Zona={zona}      ", end="\r")
    time.sleep(0.1)
```

---

### 4) NTC (IO34) — baseline EMA y delta térmico

```python
import time
import board, analogio, neopixel

PIN_TEMP = board.IO34
PIN_PIX  = board.IO2

adc = analogio.AnalogIn(PIN_TEMP)
pix = neopixel.NeoPixel(PIN_PIX, 1, brightness=0.25, auto_write=True)

def leer_prom(adc, n=16):
    s = 0
    for _ in range(n):
        s += adc.value
        time.sleep(0.002)
    return s // n

print("Diagnóstico NTC: toca con los dedos (calor) y luego algo frío, ambos ~20 s.")
baseline = leer_prom(adc, 64)  # baseline inicial
alpha = 0.02                   # suavizado (EMA)
while True:
    v = leer_prom(adc, 16)
    # promedio móvil (EMA)
    baseline = int((1 - alpha) * baseline + alpha * v)
    delta = v - baseline
    # escala simple para ver cambios (no son °C)
    # si el divisor es típico, con calor el valor puede SUBIR o BAJAR según el módulo
    # por eso sólo miramos |delta|
    if delta > 60:
        pix[0] = (30, 0, 0)     # rojo = más alto que baseline (cambio)
        estado = "subio"
    elif delta < -60:
        pix[0] = (0, 0, 30)     # azul = más bajo que baseline
        estado = "bajo"
    else:
        pix[0] = (0, 25, 0)     # verde = estable
        estado = "estable"

    print(f"ADC={v:5d}  base={baseline:5d}  delta={delta:5d}  estado={estado}    ", end="\r")
    time.sleep(0.1)
```

---

## 🧪 Consejos de uso y calibración

* **LDR**: durante los 5 s de calibración, **tapa/destapa** para que el `span` no sea demasiado pequeño.
* **Mic**: si tu módulo tiene potenciómetro, ajusta hasta que el **p2p** en silencio sea moderado (no saturado).
* **NTC**: el valor puede subir o bajar con el calor según el divisor; por eso usamos **delta** relativo y no grados °C.
* **NeoPixel**: IO2 se usa en todos los tests; evita competir con otros LEDs externos en el mismo pin.

---

## 🧰 Siguientes pasos (opcional)

* **Code combinado**: integrar los cuatro sensores y emitir **JSON 1 Hz** (para dashboard y/o informe diario).
* **Dashboard pared** (Streamlit): tarjetas grandes y semáforo.
* **Informe humano (Gemini)**: leer CSV/JSON del día y redactar recomendaciones con horas.

