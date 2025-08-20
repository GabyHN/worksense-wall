# WorkSense Wall — Monitor de Ambiente Laboral (Ruido • Luz • Temperatura • Vibración)

> **Idea**: Pantallas de pared que muestran el estado del ambiente en tiempo real, y un **informe diario** (lenguaje humano) con **recomendaciones accionables** para RR. HH. y Facilities.
> **Objetivo**: mejorar **bienestar**, **foco** y **productividad** con datos claros, no técnicos.

---

## 🧭 Pitch (30 s)

**WorkSense** convierte mediciones de **ruido, luz, temperatura y vibraciones** en señales **claras** para todos y en **informes** comprensibles para RR. HH. Al final del día, **Gemini** redacta un resumen con **“qué pasó, a qué hora y qué hacer”**. Es **bajo costo**, **rápido de instalar** y enfocado en **acciones**, no solo en gráficos.

---

## 👀 Qué se muestra en la pantalla (pared)

* **Estado general**: Tranquilo y confortable ✅
* **Ruido**: Moderado (apto para tareas rutinarias)
* **Luz**: Confortable (evita reflejos)
* **Temperatura**: Estable (sin cambios bruscos)
* **Sugerencia ahora**: Si necesitas foco alto, usa la zona norte (más silenciosa)

Nudges cortos (rotativos):

* “Hablemos en voz baja en espacios abiertos”
* “Evita llamadas largas en el área abierta; usa phone booths”
* “Libera las salas de concentración cuando termines”

---

## 📝 Informe diario (lenguaje humano, para RR. HH./Facilities)

**Estructura:**

1. **Resumen no técnico**: “Hoy el piso 7 estuvo **confortable 72%** del tiempo; **4 picos de ruido** (máx 10:42); **deslumbramiento** 12:00–12:20; **temperatura** estable.”
2. **Momentos clave (con horas)**:

   * 10:35–10:50 ruido alto en logística/impresoras
   * 12:00–12:20 deslumbrante en ala este
   * 15:30 delta térmico leve por apertura de ventanas
3. **Recomendaciones (priorizadas)**:

   1. Reubicar impresora o programar trabajos fuera de horas pico
   2. Difusores/films o persianas 12:00–13:00
   3. Señalizar “zonas de foco” y reforzar etiqueta de llamadas
   4. Ajustar setpoint/ventilación 15:00–16:00
4. **Indicadores**: % tiempo confortable (meta ≥80%), eventos de ruido, minutos de deslumbramiento, episodios térmicos.
5. **Observaciones de bienestar** (p. ej. “quiet hours” recomendadas).

> Los datos técnicos (p2p, desviaciones, curvas) van en **anexos/CSV**, no en el cuerpo del informe.

---

## 🔧 Sensores y métricas (hardware de demo)

* **TAP/Vibración (IO4)**: eventos de golpe.
* **Mic (IO36)**: ventana rápida con `mean/std/p2p`; calibración de “silencio base”; **noise\_ratio** relativo.
* **LDR (IO39)**: **luz% invertida** (0 = oscuro, 100 = deslumbrante) y zonas.
* **NTC (IO34)**: baseline **EMA** y **delta** (cambios térmicos percibidos).
* **NeoPixel (IO2)**: semáforo (tap > ruido > luz).

---

## 🏗️ Arquitectura simple

* **IdeaBoard (ESP32)** emite **JSON 1 Hz** por USB (o Wi-Fi, a futuro).
* **PC** (Python)

  * **Dashboard de pared** (Streamlit) — *opcional*
  * **Logger CSV**
  * **Generación de informe** con **Gemini** al cierre del día (texto claro).

---

## 🚀 Instalación rápida (demo local)

### 1) Firmware en la IdeaBoard

* Copia `code.py` (sensores) en la unidad **CIRCUITPY**.
* Asegúrate de que imprime **una línea JSON/seg** con esta forma:

```json
{
  "ts": 12345.6,
  "tap_recent": false,
  "mic": {"min":15600,"max":16000,"mean":15800,"std":120,"p2p":300,"noise_ratio":1.8},
  "ldr": {"raw":2048,"luz_pct":65.4,"zona":"confort"},
  "ntc": {"raw":3120,"baseline":3105,"delta":15,"estado":"estable"}
}
```

> Si aún no emites JSON, añade en tu `code.py` un **print(json.dumps(payload))** cada 1 s con esos campos.

### 2) Script en PC (consejo de Gemini — solo texto)

```bash
pip install google-generativeai pyserial python-dotenv
```

Crea `.env`:

```
GOOGLE_API_KEY=TU_API_KEY_DE_GEMINI
SERIAL_PORT=COM8
BAUD=115200
```

Ejecuta:

```bash
python coach_local.py
```

> Muestra métricas clave + **“Consejo:”** corto y accionable. (Sin voces).

---

## 🖥️ Dashboard de pared (opcional)

* Pantalla fullscreen con **estado grande**, tarjetas (Ruido/Luz/Temp/Vibración), marcas de eventos y un botón **“Generar informe”**.
* Se puede implementar en **Streamlit** (1 archivo) consumiendo el mismo JSON/CSV.

---

## 🔒 Privacidad y gobernanza

* **Sin audio crudo ni video**; solo niveles agregados.
* **Sin PII**.
* Logs de acciones y **modo manual** disponible.
* Informe redactado para RR. HH., no técnico.

---

## 🤖 Automatización futura (edificio inteligente)

> Diseñado para conectarse al BMS y **actuar** con permisos.

**Qué automatiza**

* **HVAC**: ajustes de setpoint y ventilación ante deltas térmicos sostenidos.
* **Luz/Persianas**: bajar persianas o atenuar luminarias si **luz%>85** por N min.
* **Ruido**: activar “Quiet mode” en pantallas; redistribuir trabajos ruidosos.
* **Zonificación**: sugerir zonas de foco; derivar llamadas a phone booths.
* **Energía/Mantenimiento**: modo ahorro fuera de picos; tickets por vibración anómala.

**Conectividad**

* Protocolos: **BACnet/IP**, **Modbus**, **KNX**, **DALI**, Zigbee/Z-Wave; o APIs del BMS.
* **Human-in-the-loop** → **auto-aplicar** por franja/zonas → **auditoría** y **reversión**.

**Ejemplos de políticas (YAML)**

```yaml
policies:
  glare_control:
    if_luz_pct_over: 85
    for_minutes: 3
    actions:
      - set_persianas: 50
      - set_iluminacion_pct: 80
  quiet_hours:
    hours: ["10:30-12:00"]
    if_noise_ratio_over: 3.0
    for_minutes: 2
    actions:
      - show_message: "Quiet mode en área abierta"
      - notify_role: "Facilities"
  thermal_delta:
    if_ntc_delta_abs_over: 150
    for_minutes: 5
    actions:
      - adjust_setpoint_celsius: -1
      - boost_ventilation_minutes: 10
```

---

## 📊 KPI que importan a RR. HH./Workplace

* **% tiempo en confort** (por piso/ala).
* **Quiet hours**: cumplimiento vs. violaciones (ruido > umbral).
* **Minutos de deslumbramiento** y **episodios térmicos**.
* Tendencias por día/semana para planificar layout, limpieza y mantenimiento.

---

## 🧪 Flujo de demo (2 min)

1. Pantalla en vivo.
2. Aplauso → **Ruido alto** (amarillo/rojo).
3. Linterna al LDR → **Deslumbrante**; tapar → **Oscuro**.
4. Tocar NTC → **Delta térmico**.
5. Clic “Generar informe” → resumen con horas y acciones.

---

## 🗺️ Roadmap

* **Semana 1**: Dashboard pared + logger CSV + botón “Generar informe”.
* **Semana 2**: Mapa de calor por hora, umbrales configurables.
* **Semana 3**: Multi-nodo + alertas por correo.
* **Fase 2–3**: Integración BMS → **modo asistido** → **auto-aplicar** con auditoría.

---

## 📁 Estructura sugerida del repo

```
/firmware/
  code.py                 # sensores y JSON 1 Hz
/pc/
  coach_local.py          # lectura serial + consejo Gemini (texto)
/dashboard/
  wall_app.py             # Streamlit (opcional)
/docs/
  sample_report.md        # plantilla informe diario (humano)
  policies.example.yaml   # ejemplo de políticas futuras
README.md
```

## 📁 Módulo de ritmo cardíaco en el mouse (biofeedback contextual)

Integrar un sensor de pulso (MAX3010x) dentro del mouse nos permite medir el BPM de forma pasiva mientras la persona trabaja y correlacionarlo con el ambiente que monitorea WorkSense (ruido, luz, temperatura, vibración). Cuando el sistema detecta desviaciones sostenidas respecto al rango personal/esperado (p. ej., BPM alto por encima de 110 o bajo por debajo de 50 durante varios segundos), genera micro-intervenciones: un nudje visual en la pantalla y un coach de respiración breve (por audio local) para ayudar a volver a un estado estable. A nivel de analítica, no se guarda señal cruda, solo métricas agregadas (BPM/ventanas de tiempo) y marcas de eventos (“pico de ruido 10:42 → BPM ↑ durante 3 min”), lo que permite a RR. HH./Facilities entender cómo el entorno afecta a las personas y priorizar acciones (quiet hours, films/persianas ante deslumbramiento, mover equipos ruidosos, ajustar setpoints). Esto es bienestar y foco, no diagnóstico médico: el objetivo es avisar a la persona a tiempo y enseñar hábitos de recuperación (respiración 4-2-6, pausas breves, cambio de zona) mientras el informe diario traduce los hallazgos en recomendaciones operables para mejorar el lugar de trabajo.

---

## ⚠️ Disclaimer

Proyecto de **prototipo** para bienestar y productividad en oficinas. No es instrumento médico ni sustituye evaluaciones de seguridad industrial reglamentarias.




