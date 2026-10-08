# Rosetta Acceptance Test v0.3

Firmware de aceptación para **Rosetta Rev A.0 / STM32F446RET6**.

Parte de Rosetta Firmware Base v0.1.1 y añade navegación por páginas, telemetría local más clara y buzzer.

## Páginas

KEY avanza una página y emite un beep corto:

1. Summary
2. INA260 / Power
3. HSE / System Clock
4. LSE 32.768 kHz
5. RTC / Calendar
6. OLED
7. EEPROM AT24C512C
8. SPI Flash W25Q128JV

La pantalla se refresca cada 500 ms para mantener vivas las lecturas de INA260 y RTC.

## Sonido

- Al iniciar: chime original ascendente de tres notas.
- Cada click de KEY: beep corto de 1.8 kHz.

El buzzer de Rosetta está en **PB14 / TIM12_CH1** y se maneja por PWM a través del MOSFET de la placa.

## CubeMX requerido

### RCC / clocks
- HSE: Crystal/Ceramic Resonator, 8 MHz.
- LSE: Crystal/Ceramic Resonator, 32.768 kHz.
- Para el bring-up actual: **LSE Drive Capability = High**.
- SYSCLK: 180 MHz.
- HCLK: 180 MHz.
- APB1: 45 MHz.
- APB2: 90 MHz.

### GPIO
- PA15: GPIO Output, label `USER_LED`, initial Low.
- PB15: GPIO Input, label `USER_KEY`, sin pull interno.
- PB5: GPIO Output, label `FLASH_CS`, initial High.

### I2C1
- PB6: SCL.
- PB7: SDA.
- 100 kHz para bring-up.

### SPI3
- PB3: SCK.
- PB4: MISO.
- PC1: MOSI.
- PB5: CS por GPIO.
- Master, Full Duplex, 8 bit, CPOL Low, CPHA 1 Edge, NSS Software, MSB First.

### RTC
- Clock source: LSE.
- Calendar enabled.

### TIM12 / buzzer
- PB14: `TIM12_CH1`.
- PWM Generation CH1.
- Pulse inicial 0.

El driver de Rosetta recalcula prescaler/ARR dinámicamente, por lo que no depende de una frecuencia de audio fija configurada en CubeMX.

## Integración en main.c

Ver `main_integration_snippet.c`.

Importante: `Rosetta_EarlyClockAssist()` va en `USER CODE BEGIN Init`, **antes de `SystemClock_Config()`**. Además deja LSE Drive Capability en High desde CubeMX para que el código generado no lo vuelva a bajar.

No hagas backup-domain reset en cada arranque: perderías la retención del RTC cuando uses CR1220.

## Sobre el cold-start del LSE

Si un power-on desde cero tarda varios segundos y un reset posterior arranca instantáneamente con `LSE OK`, el cristal sí puede estar oscilando pero su arranque en frío tiene poco margen o tarda más que la ventana esperada.

Primero:
- usar LSE Drive High;
- limpiar bien la zona del cristal;
- comprobar soldaduras;
- repetir al menos 10 cold boots.

Si sigue tardando 5-10 s, antes de declarar el hardware terminado conviene revisar la carga del cristal (C13/C14), ESR y especificación exacta del cristal. No aumentar indefinidamente el timeout como solución final.

## Tests automáticos v0.2

- HSE: flag HSERDY.
- LSE: flag LSERDY.
- OLED: ACK + inicialización.
- INA260: dirección + Manufacturer ID + Die ID + lecturas eléctricas.
- EEPROM: ACK + lectura no destructiva con dirección interna de 16 bits.
- Flash: JEDEC ID; W25Q128 esperado `EF4018`.
- RTC: lectura válida de Time + Date.

El test automático sigue siendo no destructivo. Una versión posterior puede añadir un **Extended Acceptance Test** manual con escritura/restauración de una dirección reservada de EEPROM y un sector reservado de Flash.

## CMake

Añade el contenido de `cmake_rosetta_snippet.txt` al target de aplicación. Esta versión agrega `rosetta_buzzer.c`.

## Demo de texto sincronizado

`Examples/timed_text_template.c` contiene un motor mínimo para mostrar texto por timestamps. No se compila por defecto y usa texto de ejemplo. Sustituye las líneas y tiempos por material que puedas reproducir.


## Git y conservación del bring-up

La carpeta `Git/` incluye un `.gitignore` y `GIT_WORKFLOW.md` para congelar el
bring-up estable como `bringup-v0.1.1`, trabajar el Acceptance Test en
`acceptance-test` y mantener la demo OLED en una rama independiente.

## LSE no bloqueante (opcional)

`Optional_Async_LSE/` contiene una variante experimental para que el sistema
arranque OLED/buzzer/tests sin esperar a `LSERDY`. Requiere que CubeMX **no**
habilite LSE dentro de `SystemClock_Config()` ni llame `MX_RTC_Init()` antes de
que el cristal esté listo. Lee `README_ASYNC_LSE.md` antes de activarla.

## Demo Wake Me Up

`Examples/wake_me_up_demo.c` contiene exclusivamente el fragmento de texto
proporcionado por el propietario del proyecto. Los timestamps incluidos son una
base de ajuste, no una sincronización verificada contra una grabación concreta.
La idea es mantener esta demo en `demo-wake-me-up`, separada del firmware de
aceptación.
