# Sistema de Compensacion Automático de Iluminación
> **Asignatura:** Electrónica Digital III - Universidad Nacional de Córdoba
> 
> **Integrantes:**
> * Martina Ciruzzi
> * Mauricio Agustin Herrera
> 
> **Profesor:** Marcos Blasco

---

## 🚀 1. Descripción General del Proyecto

El proyecto consiste en un sistema automático de control de iluminación implementado sobre el microcontrolador LPC1769. El sistema mide la luz ambiente mediante un sensor TEMT6000 conectado al ADC, adquiere muestras periódicas utilizando Timer0 y almacena bloques de datos mediante GPDMA para luego calcular una medición promedio más estable por medio del promedio.

A partir de la medición obtenida, el firmware estima el nivel de iluminación en milivolts, lux aproximados y porcentaje. El usuario puede ingresar por UART un porcentaje de iluminación deseado entre 0 % y 100 %, y el sistema calcula el error entre la luz deseada y la luz ambiente medida. Con ese error se ajusta el duty cycle de una señal PWM generada con Timer1, aplicada al gate de un MOSFET para regular la intensidad de la carga lumínica. Además, el DAC entrega una salida analógica proporcional a la medición de iluminación en lux y el estado del sistema se reporta periódicamente por terminal serie. El sistema también cuenta con un pulsador asociado a EINT0 para alternar entre estado activo y detenido.

### 🎯 Alcances del Proyecto (¿Qué hace y qué NO hace el sistema?)

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

## 📐 2. Arquitectura del Sistema: Hardware y Software

### 🔌 Hardware & Interconexión
* **Diagrama de Bloques:** [Insertar imagen o link al diagrama de bloques del hardware]
* **Esquemático del Circuito:** *[Inserte aquí la captura de imagen/render del esquemático completo desarrollado en KiCad/Altium]*
  `![Esquemático Completo](hardware/esquematico.png)`
* **Descripción del Circuito y Consideraciones de Diseño:** Breve explicación de las etapas (ej: acoplamiento de señales, protecciones inductivas, filtrado, etc.).

### 💻 Arquitectura de Software (Firmware)
* **Diagrama de Flujo o Máquina de Estados:** *[Inserte aquí la imagen del diagrama que explique el lazo principal o el comportamiento del sistema]*
  `![Diagrama de Flujo / Máquina de Estados](docs/diagrama_software.png)`

---

## ⚡ 3. Especificaciones Eléctricas, Alimentación y Entorno

### 🔌 Parámetros de Alimentación y Consumo (Común a ambas materias)
* **Tensión de operación del sistema:** 3.3V (lógica del LPC1769) y 7V (alimentación del LED de potencia vía MOSFET)
* **Método de alimentación:** Placa LPC1769: alimentación por USB (5V) con regulador interno a 3.3V -  Etapa de potencia (Carga LED externa controlada por un MOSFET IRLZ44N): fuente externa de 7V - Sensor TEMT6000: 3.3V tomados directamente de la placa
* **Consumo estimado o medido:** * En modo activo (Depende principalmente de la corriente de la carga LED externa y del duty aplicado. Se estima para la etapa LED + MOSFET en máxima conducción.): `300 - 600 mA`
  * En modo bajo consumo (Estado Detenido - Solo LPC1769 + Sensor): `55 mA medidos aproximadamente`

* **IDE y SDK:** MCUXpresso IDE v11.x con drivers CMSIS v2p00 para LPC17xx refactorizados por David Trujillo Medina (versión 2026)
* **Microcontrolador Principal:** NXP LPC1769
* **Bibliotecas de Terceros y Versiones:** No se utilizaron bibliotecas de terceros adicionales
* **Periféricos Avanzados Utilizados:** NVIC, GPDMA, SysTick, DAC, ADC, Timer, UART, EINT
* **Estrategia de Concurrencia:** Expliquen la arquitectura elegida: El sistema utiliza arquitectura bare-metal orientada a interrupciones, sin RTOS. El main loop actúa como despachador de tareas diferidas de baja prioridad, mientras que toda la lógica de tiempo real se ejecuta en ISR.
* Arquitectura de interrupciones y prioridades:
  * UART1_IRQHandler : Prioridad ( 0 ) (máxima) - Recepción de comandos por teclado
  * EINT0_IRQHandler : Prioridad ( 2 ) - Arranque y parada del sistema
  * DMA_IRQHandler : Prioridad ( 1 ) - Procesamiento de bloque ADC, cálculo de error, duty PWM y DAC
  * TIMER1_IRQHandler : Prioridad ( 3 ) - Generación de PWM, aplicación del duty pendiente al inicio del período
  * SysTick_Handler: Prioridad ( fijo CM3 ) - Contador de 2 segundos para reporte UART periódico
* Mecanismo para PWM con dobble buffer:
  * Al completarse un bloque de muestras, el DMA dispara una interrupción. En el handler correspondiente se procesa el bloque, se calcula el promedio, el error y el nuevo duty pendiente.
* Comunicación ISR → main loop via flags volátiles:
  * enviar_estado, aviso_arranque y comando_listo son flags que las ISR activan y el main consume. Esto evita ejecutar UART_Send BLOCKING dentro de interrupciones, que era la causa original del congelamiento del sistema.

---

## 🔄 4. Proceso de Integración y Desarrollo (Común)
Describan cronológicamente cómo fueron sumando y testeando las diferentes partes del proyecto (enfoque modular de ingeniería).

* **Etapa 1 (Validación inicial):** Se configuró el pin P0.0 como salida GPIO y se verificó el encendido del LED de prueba. Se configuró el Timer1 con PWM por software y se validó la señal con osciloscopio, probando manualmente distintos valores de duty via cambios en el código. Se verificó también que el MOSFET respondía correctamente a la señal PWM generada.
* **Etapa 2 (Adquisición/Comunicación):** Se implementó el ADC con trigger por Timer0 (MAT0.1 en toggle) y se enviaron los valores crudos de 12 bits por UART1 a 9600 baud. Se detectó en esta etapa que UART_Send BLOCKING a 9600 baud generaba períodos de bloqueo demasiado largos. Se migró a 115200 baud y se ajustaron las prioridades del NVIC, con UART1 en prioridad 0 para evitar que Timer1 interrumpiera la transmisión. Se validó la recepción de comandos numéricos por teclado con eco de confirmación.
* **Etapa 3 (Integración lógica):** Se incorporó el GPDMA con LLI circular para transferir automáticamente las conversiones del ADC al buffer en SRAM (0x2007C000). Se eliminó la interrupción del ADC, delegando todo el flujo al DMA. Se tomaron mediciones empíricas del sensor TEMT6000 con un luxómetro a distintos niveles de iluminación, construyendo la tabla de calibración de 6 tramos (luego extendida a 7) con interpolación lineal por tramos en aritmética entera. Se detectó y corrigió el problema del 1% detectado como 0% ajustando el primer tramo para devolver 1 directamente en lugar de aplicar división entera. Se agregó el DAC con salida proporcional al voltaje medido por el sensor, verificable con osciloscopio.
* **Etapa 4 (Sistema Completo):** Se integró la lógica de control completa: cálculo de error, zona muerta de ±2% para eliminar el parpadeo del PWM cerca del setpoint, y doble buffer duty_pwm/duty_pwm_pendiente para aplicar cambios de duty solo al inicio del período. Se realizaron pruebas de estrés subiendo el nivel de luz del ambiente al máximo, donde se detectó el congelamiento del UART por acumulación de flags del SysTick. Se corrigió reseteando contador_2_segundos al procesar cada comando y cancelando flags pendientes. Se calibró el valor ADC_MAX_CALIBRADO midiendo el voltaje real del sensor a máxima iluminación con luxómetro. Se validó el sistema completo en la caja controlada con el foco LED regulable desde el celular como fuente de luz del ambiente.

---

## 📊 5. Ensayos, Pruebas y Resultados (Común)
Demuestren con datos empíricos que el sistema funciona correctamente. **Es obligatorio incluir registro visual**.

* **Pruebas Funcionales Realizadas:**
  * Se testeo la obtencion de los datos del Sensor TEMT6000 regulando la luz ambiente, verificando si los datos obtenidos por el sensor coincidian con los porcentajes inyectados, ademas se testeo si respondia correctamente el PWM cuando se llegaba al valor del SetPoint y si la comunicacion Serie estaba en correcto funcionamiento
  * Se testeo la salida del DAC variando la intensidad luminica del ambiente

 



* **Evidencia Fotográfica y Gráficos:** * *Capturas de instrumental:*
* https://github.com/user-attachments/assets/d04b160f-7b00-40a7-9371-da4b359fe886
* https://github.com/user-attachments/assets/ec9590e8-c07a-4986-8deb-8884e89a1a99
  * *Foto del Prototipo Real:*
   >
  <img width="1200" height="1600" alt="PrototipoFinal" src="https://github.com/user-attachments/assets/50256f36-bfc8-433e-8340-ec1009083b11" />
  <img width="1200" height="1600" alt="PrototipoFinal" src="https://github.com/user-attachments/assets/0b1edfa1-cee7-4cf0-8454-85cf5c20715a" />


---

## 📂 6. Estructura del Repositorio
El repositorio debe mantener obligatoriamente la siguiente estructura limpia (¡Recuerden configurar correctamente el `.gitignore` para no subir carpetas temporales como `Debug/`, `Release/` o archivos `.p1` / `.d`!).

```text
├── firmware/          # Código fuente del proyecto (MPLABX / MCUXpresso / STM32Cube)
│   ├── src/           # Archivos de código (.c)
│   └── inc/           # Archivos de cabecera (.h)
├── hardware/          # Archivos de diseño (KiCad/Altium), esquemáticos en PDF/Imagen y BOM
├── docs/              # Datasheets clave, imágenes del README, notas de aplicación
└── README.md          # Este archivo de presentación
