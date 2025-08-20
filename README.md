WorkSense — El dispositivo inteligente que convierte tu oficina en un lugar donde la gente quiere trabajar

Sueño grande: espacios que cuidan a las personas mientras trabajan.
Cómo lo logramos: un dispositivo compacto que escucha, observa y entiende el entorno (ruido, luz, temperatura, vibración) y propone acciones concretas para mantener condiciones óptimas de foco, bienestar y rendimiento.

WorkSense mide lo que sabotea la concentración —ruido, deslumbramiento, microcambios térmicos y vibraciones—, lo procesa en el dispositivo y te dice qué hacer ahora (en lenguaje humano): “baja persianas en el ala este 12:00–13:00”, “mueve las llamadas a phone booths”, “ventila 10 minutos”.
Al cierre del día, genera un informe claro con qué pasó, a qué hora y qué decisión tomar mañana.
Es instalación exprés, bajo costo y obsesionado con la acción, no con dashboards bonitos que nadie mira.

❤️ ¿Por qué esto importa?

El bienestar no es “nice to have”. La OMS estima 12 mil millones de jornadas laborales perdidas al año por depresión y ansiedad; US$1 billón en productividad. El entorno de trabajo sí influye: sobrecarga, ruido y condiciones pobres empeoran la salud mental y el rendimiento. 
Organización Mundial de la Salud
+2
Organización Mundial de la Salud
+2

El ruido en oficinas abiertas reduce atención y desempeño cognitivo; basta con voces inteligibles para mermar tareas complejas. 
sciencedirect.com
pmc.ncbi.nlm.nih.gov

Nuestro enfoque: medir → interpretar → actuar. Si no produce una decisión clara, no sirve.

🧠 Inspiración (videos para ver ya)

Adam Grant (psicólogo organizacional) — How to stop languishing and start finding flow (TED). Ideal para hablar de foco y desempeño sostenibles. 
ted.com

Amy Edmondson — Building a psychologically safe workplace (TEDx): la seguridad psicológica es el suelo fértil del rendimiento. 
YouTube

Bonus: Shawn Achor — The happy secret to better work: felicidad y productividad están conectadas. 
ted.com

🌟 ¿Qué nos hace diferentes?

Acción inmediata: recomendaciones concretas y temporizadas (“12:00–12:20 baja persianas en ala este”).

Lenguaje humano: sin jerga técnica; RR. HH. y Facilities lo entienden a la primera.

Offline-first: decide en el dispositivo; la nube es opcional.

Señalización local: semáforo a color que guía el comportamiento en el momento.

Escalable y realista: bajo costo por nodo; futuro plug-in a BMS (BACnet/Modbus/KNX/DALI) para automatizar setpoints, persianas y modos “Quiet”.

🎯 Lo que un juez verá en 2 minutos

Aplausos → el sistema detecta ruido alto y sugiere mover llamadas.

Linterna o tapar LDR → detecta deslumbramiento/oscuridad y sugiere ajustar persianas/iluminación.

Tocar el NTC → registra delta térmico y propone ventilar/ajustar setpoint.

Golpecito → evento de vibración (muestra alerta, guarda evento).

Cierre del día → informe con horas clave y lista de decisiones para mañana.

🔒 Ética y confianza

Sin audio crudo ni video: solo niveles agregados (nada de espionaje).

Sin PII.

Humano en el loop para cualquier automatización del edificio.

📈 Métricas que mueven la aguja

% de tiempo en confort (por piso/ala).

Quiet hours: cumplimiento vs. violaciones por ruido.

Minutos de deslumbramiento y episodios térmicos por franja.

Tendencias semanales que justifican decisiones (layout, mantenimiento, horarios).

🛠️ Detrás del telón (muy breve)

Dispositivo ESP32 (CircuitPython): sensores de ruido (ratio relativo), luz (%), delta térmico y vibración. Señalización con NeoPixel.

Informe diario: redactor en la nube que convierte eventos en acciones priorizadas.

Dashboard opcional (Adafruit IO / Streamlit) para visualización rápida.
