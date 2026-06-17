# [Nombre del Proyecto / Sistema]
> **Asignatura:** Electrónica Digital III - Universidad Nacional de Córdoba
> **Integrantes:** > * Martina Ciruzzi
> * Mauricio Agustin Herrera
> **Profesor:** Marcos Blasco

---

## 🚀 1. Descripción General del Proyecto (Común a DII y DIII)

El proyecto consiste en un sistema automático de control de iluminación implementado sobre el microcontrolador LPC1769. El sistema mide la luz ambiente mediante un sensor TEMT6000 conectado al ADC, adquiere muestras periódicas utilizando Timer0 y almacena bloques de datos mediante GPDMA para luego calcular una medición promedio más estable por medio del promedio.

A partir de la medición obtenida, el firmware estima el nivel de iluminación en milivolts, lux aproximados y porcentaje. El usuario puede ingresar por UART un porcentaje de iluminación deseado entre 0 % y 100 %, y el sistema calcula el error entre la luz deseada y la luz ambiente medida. Con ese error se ajusta el duty cycle de una señal PWM generada con Timer1, aplicada al gate de un MOSFET para regular la intensidad de la carga lumínica. Además, el DAC entrega una salida analógica proporcional a la medición de iluminación en lux y el estado del sistema se reporta periódicamente por terminal serie. El sistema también cuenta con un pulsador asociado a EINT0 para alternar entre estado activo y detenido.

### 🎯 Alcances del Proyecto (¿Qué hace y qué NO hace el sistema?)

### Alcances del Proyecto

### Alcances del Proyecto

**El sistema SÍ es capaz de:**

- Medir la iluminación ambiente mediante un sensor TEMT6000 conectado al ADC del LPC1769.
- Adquirir muestras periódicas usando Timer0 y almacenarlas automáticamente mediante GPDMA.
- Calcular un valor promedio de iluminación a partir de un bloque de muestras.
- Convertir la medición obtenida a milivolts, lux aproximados y porcentaje de iluminación.
- Recibir por UART un porcentaje de iluminación deseado entre 0 % y 100 %.
- Regular una carga lumínica mediante una señal PWM aplicada al gate de un MOSFET.
- Entregar por DAC una tensión proporcional a la medición de iluminación.
- Reportar por UART el estado del sistema, incluyendo valor deseado, medición, error y duty aplicado.
- Arrancar y detener el funcionamiento mediante un pulsador conectado a EINT0.

**El sistema NO incluye:**

- Conexión por Wi-Fi o Bluetooth.
- Guardado de datos en memoria o tarjeta SD.
- Interfaz gráfica para ver los datos.
- Diseño en PCB.
- Alimentación con batería.


### ⏩ Posibles Etapas Siguientes (Líneas Futuras)

En una versión futura del proyecto se podrían implementar las siguientes mejoras:

- Migrar el circuito armado en protoboard a un circuito impreso (PCB) para obtener un montaje más prolijo, seguro y estable.
- Agregar conectividad inalámbrica, como Wi-Fi o Bluetooth, para monitorear el sistema de forma remota.
- Diseñar una interfaz gráfica, ya sea en una aplicación de escritorio o móvil, para visualizar la luz medida, el valor deseado y el duty aplicado.
- Incorporar almacenamiento de datos para guardar mediciones históricas de iluminación.
- Agregar un modo de bajo consumo para reducir el consumo cuando el sistema se encuentre detenido.

---

## 📐 2. Arquitectura del Sistema: Hardware y Software (Común)

### 🔌 Hardware & Interconexión
* **Diagrama de Bloques:** <img width="987" height="790" alt="Diagrama_de_Bloques" src="https://github.com/user-attachments/assets/1d13b754-9351-402b-b494-42a38a11d208" />

* **Esquemático del Circuito:** 
![Esquemático Completo](hardware/esquematico.png)
  
### Descripción del Circuito y Consideraciones de Diseño

El circuito está organizado en etapas funcionales. La primera etapa corresponde a la **adquisición de luz ambiente**, realizada mediante el sensor TEMT6000. Este sensor entrega una señal analógica proporcional a la iluminación recibida, la cual se conecta al canal **AD0.0 del LPC1769, pin P0.23**. Dicha señal es digitalizada por el ADC del microcontrolador para obtener una representación numérica del nivel de luz presente en el ambiente.

La segunda etapa corresponde al **procesamiento y almacenamiento de muestras**. Las conversiones del ADC son disparadas periódicamente por el **Timer0**, utilizando la señal de match como evento de disparo. Luego, el **GPDMA** transfiere automáticamente las muestras obtenidas hacia una zona de memoria SRAM, reduciendo la carga de trabajo del procesador. Una vez completado el bloque de muestras, el firmware calcula un valor promedio para disminuir variaciones instantáneas y obtener una medición más estable.

La tercera etapa es la **etapa de control de iluminación**. A partir del porcentaje de luz medido y del porcentaje deseado ingresado por el usuario, el sistema calcula un error y determina el ciclo de trabajo de una señal PWM. Esta señal es generada mediante el **Timer1** y se entrega por el pin **P0.0**, conectado al gate de un MOSFET. El MOSFET funciona como etapa de potencia, permitiendo regular la corriente aplicada a la luminaria sin exigir corriente directamente al pin del microcontrolador.

Además, el sistema incorpora una salida analógica de monitoreo mediante el **DAC del LPC1769**, disponible en el pin **P0.26 / AOUT**. Esta salida entrega una tensión proporcional al nivel de luz medido, permitiendo observar externamente el comportamiento del sistema mediante un multímetro u osciloscopio.

La comunicación con el usuario se realiza mediante **UART1**, utilizando los pines **P0.15 como TXD1** y **P0.16 como RXD1**. Por este medio se recibe el porcentaje de iluminación deseado y se transmite el estado del sistema. También se incluye una entrada externa mediante **EINT0 en P2.10**, utilizada para iniciar o detener el funcionamiento del sistema mediante un pulsador.

Como consideración de diseño, se separa la etapa lógica de control de la etapa de potencia. El LPC1769 opera con niveles de **3,3 V**, por lo que el MOSFET permite manejar la carga de iluminación sin sobrecargar los pines del microcontrolador. Además, se utiliza un promedio de muestras ADC para reducir ruido o fluctuaciones propias de la medición analógica.

### 💻 Arquitectura de Software (Firmware)
* **Diagrama de Flujo o Máquina de Estados:** *
  `![Máquina de Estados](docs/diagrama_software.png)`

---

## ⚡ 3. Especificaciones Eléctricas, Alimentación y Entorno (Específico por Asignatura)

### 🔌 Parámetros de Alimentación y Consumo (Común a ambas materias)
* **Tensión de operación del sistema:** [Ej: 5V / 3.3V]
* **Método de alimentación:** [Ej: Fuente externa de 12V con regulador lineal LM7805 / Alimentación por USB]
* **Consumo estimado o medido:** * En modo activo (máxima carga, relés/motores encendidos): `XX mA`
  * En modo bajo consumo (si aplica): `XX uA`

### 📌 [OPCIÓN A: Solo para alumnos de Electrónica Digital II (PIC16F887)]
* **Herramientas de Software:** MPLAB X IDE [vX.XX] y compilador XC8 [vX.XX].
* **Hardware de Programación/Depuración:** [Ej: PICkit 3, PICkit 4].
* **Configuración de Bits (Fuses Críticos):**
  * *Oscilador:* [Ej: HS (Cristal externo de 20MHz) / INTRC (Interno 4MHz)]
  * *Watchdog Timer (WDT):* [Ej: ON / OFF]
  * *Master Clear (MCLRE):* [Ej: ON (Pin externo) / OFF (Digital IO)]
* **Periféricos Internos Utilizados:** [Ej: Timer0, ADC, EUSART, PWM].
* **Gestión de Interrupciones:** Al contar con un único vector de interrupción, expliquen la prioridad por software (*polling*) en la ISR: ¿Qué bandera (`flag`) evalúan primero y por qué?

### 📌 [OPCIÓN B: Solo para alumnos de Electrónica Digital III (Cortex-M / ARM)]
* **IDE y SDK:** [Ej: MCUXpresso IDE v11.8 con LPCOpen v2.10 / STM32CubeIDE v1.14 con HAL v1.28].
* **Microcontrolador Principal:** [Ej: NXP LPC1769 / STM32F411].
* **Bibliotecas de Terceros y Versiones:** [Ej: FreeRTOS v10.5.1 / Biblioteca LCD I2C v1.2].
* **Periféricos Avanzados Utilizados:** [Ej: NVIC, DMA, SysTick, DAC].
* **Estrategia de Concurrencia:** Expliquen la arquitectura elegida: [Ej: Bare-metal con máquina de estados cooperativa / RTOS (FreeRTOS) detallando las tareas creadas y sus prioridades].

---

## 🔄 4. Proceso de Integración y Desarrollo (Común)
Describan cronológicamente cómo fueron sumando y testeando las diferentes partes del proyecto (enfoque modular de ingeniería).

* **Etapa 1 (Validación inicial):** [Ej: Configuración del oscilador/reloj y parpadeo de LED de estado].
* **Etapa 2 (Adquisición/Comunicación):** [Ej: Implementación del ADC y envío de tramas crudas por UART].
* **Etapa 3 (Integración lógica):** [Ej: Procesamiento de datos, lógica de control o montado sobre el RTOS].
* **Etapa 4 (Sistema Completo):** [Ej: Acople de actuadores finales, calibración y pruebas de estrés].

---

## 📊 5. Ensayos, Pruebas y Resultados (Común)
Demuestren con datos empíricos que el sistema funciona correctamente. **Es obligatorio incluir registro visual**.

* **Pruebas Funcionales Realizadas:** Detallen los ensayos (Ej: "Se inyectó una señal controlada para medir la precisión del ADC...").
* **Evidencia Fotográfica y Gráficos:** * *Capturas de instrumental:* [Insertar capturas de Osciloscopio, Analizador Lógico o Terminal Serie]
  * *Foto del Prototipo Real:* [Insertar foto del hardware final cableado/armado en funcionamiento]

---

## 📂 6. Estructura del Repositorio (Común)
El repositorio debe mantener obligatoriamente la siguiente estructura limpia (¡Recuerden configurar correctamente el `.gitignore` para no subir carpetas temporales como `Debug/`, `Release/` o archivos `.p1` / `.d`!).

```text
├── firmware/          # Código fuente del proyecto (MPLABX / MCUXpresso / STM32Cube)
│   ├── src/           # Archivos de código (.c)
│   └── inc/           # Archivos de cabecera (.h)
├── hardware/          # Archivos de diseño (KiCad/Altium), esquemáticos en PDF/Imagen y BOM
├── docs/              # Datasheets clave, imágenes del README, notas de aplicación
└── README.md          # Este archivo de presentación
