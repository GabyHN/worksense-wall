# WorkSense — El dispositivo inteligente que convierte tu espacio de trabajo en un lugar donde la gente *quiere* estar
**Ruido · Luz · Temperatura · Vibración · Acciones concretas**

> **Sueño grande:** oficinas que cuidan a las personas **mientras trabajan**.  
> **Cómo lo logramos:** un dispositivo compacto que *escucha, observa y entiende* el entorno y propone **acciones simples** para mantener condiciones **óptimas** de foco, bienestar y rendimiento.

---

**WorkSense** mide lo que *sabotea* la concentración —ruido, deslumbramientos, microcambios térmicos y vibraciones—, lo procesa **en el propio dispositivo** y te dice **qué hacer ahora** (en lenguaje humano):  
*“12:00–13:00: baja persianas en ala Este”*, *“mueve llamadas a phone booths”*, *“ventila 10 minutos”*.  
Al cierre del día, genera un **resumen claro** con *qué pasó, a qué hora y qué decisión tomar mañana*. Es **instalación exprés**, **bajo costo** y **obsesionado con la acción**, no con dashboards que nadie ve.

---

## ❤️ ¿Por qué importa?

- El ambiente físico impacta el **foco, la salud y la productividad**.  
- El **ruido conversacional** destruye la atención; el **deslumbramiento** fatiga; los **deltas térmicos** incomodan y distraen.  
- Cuando el entorno ayuda, la gente **fluye**. Cuando estorba, todo se vuelve cuesta arriba.

> **Mantra:** medir → interpretar → **actuar**. Si no produce una decisión clara, no sirve.

---

## 🌟 Lo que nos hace diferentes

- **Acción inmediata**: recomendaciones **temporizadas** y **concretas** (por zona y hora).  
- **Lenguaje humano**: sin jerga técnica; RR. HH. y Facilities lo entienden a la primera.  
- **Offline-first**: decide en el dispositivo; la nube es opcional.  
- **Señalización local**: NeoPixel tipo semáforo (prioridad: **tap** → **ruido** → **luz**).  
- **Escalable**: bajo costo por nodo, listo para múltiples áreas.

---

## 🎯 Lo que verás en la demo (2 min)

1. **Aplausos** → detecta **ruido alto** y sugiere mover llamadas.  
2. **Linterna / tapar LDR** → detecta **deslumbrante / oscuro** y sugiere ajustar persianas/iluminación.  
3. **Tocar el NTC** → registra **delta térmico** y propone ventilar/ajustar setpoint.  
4. **Golpecito en la mesa** → evento de **vibración**.  
5. **Cierre del día** → **informe** con horas clave y **acciones** para mañana.

---

## 🔧 De qué está hecho (demo hardware)

- **TAP/Vibración (IO4):** detecta golpes/eventos (latcheado 3 s para visibilidad).  
- **Mic (IO36):** ventana rápida (`std/p2p`) y **noise_ratio** relativo al “silencio base”.  
- **LDR (IO39):** **luz% invertida** (0 = oscuro, 100 = deslumbrante) + zonas.  
- **NTC (IO34):** baseline **EMA** y **delta** (cambios térmicos percibidos).  
- **NeoPixel (IO2):** semáforo de estado.

**Opcional (nube):** publica métricas a Adafruit IO para un tablero web de pared.

---

## 📈 Indicadores que sí mueven la aguja

- **% de tiempo en confort** (por piso/ala).  
- **Quiet hours**: cumplimiento vs. violaciones (ruido > umbral).  
- **Minutos de deslumbramiento** y **episodios térmicos**.  
- Tendencias por día/semana para justificar decisiones (layout, mantenimiento, horarios).

---

## 🛡️ Privacidad y confianza

- **Sin audio crudo ni video**; solo niveles agregados.  
- **Sin PII**.  
- **Humano en el loop** para cualquier automatización futura del edificio.

---

## 🧭 Roadmap → edificio inteligente

- Integración con BMS (BACnet/IP, Modbus, KNX, DALI, Zigbee/Z-Wave o APIs).  
- Políticas por horario/zona: *si luz% > 85 durante N min → bajar persianas/atenuar iluminación*.  
- “Quiet mode” automático y tickets por vibraciones anómalas.  
- Auditoría y reversión con aprobación humana.

---

## ⚙️ Instalación rápida (demo)

1. **/firmware**: copiar `code.py` y `secrets.py` a la unidad **CIRCUITPY** (ESP32/CircuitPython).  
2. **(Opcional) Dashboard**: crear feeds `worksense.noise_ratio`, `worksense.luz_pct`, `worksense.ntc_delta`, `worksense.tap_recent` y `worksense.json` en Adafruit IO.  
3. **(Opcional) Informe diario**: script en `/pc` que resume el día en lenguaje humano.

---

## 🎥 Video recomendado

> Psicología aplicada al trabajo: foco, bienestar y rendimiento.

[![Adam Grant — How to stop languishing and start finding flow (TED)](https://img.youtube.com/vi/a3zPgyvCiJI/maxresdefault.jpg)](https://www.youtube.com/watch?v=a3zPgyvCiJI "Ver en YouTube")

<details>
  <summary>Más charlas útiles (opcional)</summary>

[![Amy Edmondson — Building a psychologically safe workplace (TEDx)](https://img.youtube.com/vi/LhoLuui9gX8/hqdefault.jpg)](https://www.youtube.com/watch?v=LhoLuui9gX8 "Ver en YouTube")

[![Shawn Achor — The happy secret to better work (TED)](https://img.youtube.com/vi/fLJsdqxnZb0/hqdefault.jpg)](https://www.youtube.com/watch?v=fLJsdqxnZb0 "Ver en YouTube")

</details>

---

## ⚠️ Disclaimer

Prototipo para bienestar y productividad. No sustituye instrumentos ni evaluaciones de seguridad industrial.

---

> **Mensaje final:** *La tecnología vale cuando hace que el trabajo se sienta bien y salga mejor.*  
> **WorkSense** convierte el entorno en un aliado — **hoy**.


