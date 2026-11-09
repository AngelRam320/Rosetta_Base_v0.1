# Rosetta Flight Computer Platform

**Rosetta** es una plataforma de desarrollo y computadora embebida basada en el **STM32F446RET6**, diseñada para pruebas, aprendizaje, integración de sensores y desarrollo de sistemas de vuelo para proyectos aeroespaciales y CanSat.

La plataforma fue diseñada desde cero como PCB propia con el objetivo de disponer de una base reutilizable para desarrollo de firmware, pruebas de aviónica y futuras computadoras de vuelo del equipo Cuauhtémoc IPN.

---

# Hardware

## Rosetta Rev A.0

- MCU: **STM32F446RET6**
- Encapsulado: LQFP64
- Frecuencia de sistema: **180 MHz**
- Flash interna: 512 KB
- SRAM: 128 KB
- PCB: 4 capas
- Alimentación:
  - USB-C 5 V
  - Entrada externa
  - Regulador buck a 3.3 V
- Monitor de potencia: **INA260**
- OLED integrada 128×64
- RTC con LSE de 32.768 kHz
- EEPROM externa
- Flash externa **W25Q128JVSIQ – 16 MB**
- Socket microSD mediante **SDIO de 4 bits**
- LED de usuario
- Botón `KEY`
- Buzzer pasivo mediante PWM
- SWD para programación y debugging
- Interfaces externas para UART, I2C, SPI, CAN, PWM, ADC y GPIO.

---

# MCU y clocks

Rosetta utiliza:

- **HSE:** cristal externo de 8 MHz
- **SYSCLK:** 180 MHz mediante PLL
- **LSE:** cristal externo de 32.768 kHz
- **RTC:** alimentado desde LSE
- **SDIO clock:** dominio de 48 MHz generado mediante PLLSAI

Configuración principal:

```text
HSE = 8 MHz

PLLM = 4
PLLN = 180
PLLP = 2

SYSCLK = 180 MHz
HCLK   = 180 MHz
APB1   = 45 MHz
APB2   = 90 MHz
```

Para SDIO:

```text
PLLSAIM = 4
PLLSAIN = 96
PLLSAIP = 4

8 MHz / 4 × 96 / 4 = 48 MHz
```

---

# LSE y RTC

Durante el bring-up inicial se detectaron dificultades de arranque del LSE.

El problema resultó ser una combinación de una soldadura deficiente del cristal y una carga capacitiva que podía mejorarse.

La configuración actualmente utilizada es:

```text
LSE = 32.768 kHz
C13 = 15 pF
C14 = 15 pF
```

Después de la corrección, el LSE inicia consistentemente aproximadamente en:

```text
< 1 segundo
```

El firmware actual conserva la inicialización convencional mediante HAL/CubeMX.

Se desarrolló de forma experimental una alternativa de inicialización asíncrona del LSE, pero **no se encuentra implementada en la versión estable actual**.

---

# Correcciones durante Bring-Up

Durante el ensamblaje y validación de Rosetta Rev A.0 se identificaron varios problemas de manufactura que fueron corregidos.

### Regulador de alimentación

Una resistencia asociada al regulador buck presentaba una soldadura deficiente.

Síntomas observados:

- LEDs con brillo reducido.
- Reinicios.
- Arranque incompleto.
- Comportamiento inestable mediante USB-C.

Después de resoldarla, el sistema recuperó una alimentación estable.

### LSE

El cristal de 32.768 kHz presentó una unión deficiente.

Fue reemplazado y posteriormente se modificaron los capacitores de carga a:

```text
15 pF / 15 pF
```

Resultado:

```text
LSE OK
RTC OK
Cold start < ~1 s
```

### Diodos

Se detectaron dos diodos instalados con orientación incorrecta durante el ensamblaje inicial.

Fueron corregidos antes de continuar con las pruebas funcionales.

---

# Consumo

Durante las primeras pruebas, Rosetta presenta un consumo aproximado de:

```text
~95 mA
```

El valor se utiliza actualmente como primera referencia de consumo basal de Rosetta Rev A.0.

Posteriormente se documentarán consumos para diferentes estados y periféricos.

---

# Firmware

El firmware se desarrolla utilizando:

- STM32CubeMX
- STM32 HAL
- VS Code
- STM32Cube Extension
- CMake
- Ninja
- ST-Link
- Git / GitHub

---

# Estructura del proyecto

```text
Rosetta_Base/
│
├── Core/
│   ├── Inc/
│   └── Src/
│
├── Drivers/
│
├── FATFS/
│
├── Middlewares/
│
├── Rosetta/
│   ├── Inc/
│   │   ├── rosetta.h
│   │   ├── rosetta_status.h
│   │   ├── rosetta_oled.h
│   │   ├── rosetta_ina260.h
│   │   ├── rosetta_eeprom.h
│   │   ├── rosetta_flash.h
│   │   ├── rosetta_buzzer.h
│   │   ├── rosetta_sd.h
│   │   ├── rosetta_ui.h
│   │   ├── rosetta_logo.h
│   │   └── rosetta_font5x7.h
│   │
│   └── Src/
│       ├── rosetta.c
│       ├── rosetta_oled.c
│       ├── rosetta_ina260.c
│       ├── rosetta_eeprom.c
│       ├── rosetta_flash.c
│       ├── rosetta_buzzer.c
│       ├── rosetta_sd.c
│       └── rosetta_ui.c
│
├── Tools/
│   └── rosetta_splash_converter.py
│
├── Rosetta_Base.ioc
├── CMakeLists.txt
└── README.md
```

---

# Rosetta Bring-Up

El primer firmware desarrollado tuvo como objetivo comprobar que la PCB podía arrancar correctamente y comunicarse con sus periféricos internos.

La etapa de bring-up permitió validar:

```text
STM32F446RET6       PASS
SWD                 PASS
HSE                 PASS
PLL / 180 MHz       PASS
LSE                 PASS
RTC                 PASS
3.3 V power         PASS
OLED                PASS
I2C1                PASS
USER KEY            PASS
USER LED            PASS
Buzzer              PASS
```

El estado estable del bring-up se conserva mediante Git.

Tag:

```text
bringup-v0.1.1
```

---

# Acceptance Test

La rama:

```text
acceptance-test
```

contiene el firmware utilizado para realizar una validación funcional de Rosetta Rev A.0.

El objetivo es disponer de un firmware que pueda cargarse en cualquier unidad Rosetta recién ensamblada y comprobar rápidamente si sus principales subsistemas funcionan correctamente.

---

# Interfaz del Acceptance Test

La OLED presenta una página independiente para cada subsistema.

Actualmente existen:

```text
1/9  SUMMARY
2/9  POWER / INA260
3/9  HSE / SYSTEM CLOCK
4/9  LSE
5/9  RTC
6/9  OLED
7/9  EEPROM
8/9  SPI FLASH
9/9  MICROSD / SDIO
```

El botón:

```text
USER_KEY
```

permite cambiar de página.

Cada pulsación genera un beep corto mediante el buzzer.

Durante el arranque Rosetta reproduce un pequeño tono de inicio.

---

# OLED

La OLED integrada utiliza:

```text
I2C1
SCL = PB6
SDA = PB7
```

Durante la integración de SDIO se detectó un remapeo accidental de PB6 en CubeMX.

El Acceptance Test permitió detectar inmediatamente el fallo del bus I2C.

Configuración correcta:

```text
PB6 = I2C1_SCL
PB7 = I2C1_SDA
```

---

# INA260

El INA260 permite medir:

- Voltaje de alimentación.
- Corriente.
- Potencia.

El Acceptance Test muestra estas mediciones en tiempo real.

Durante las pruebas iniciales se ha observado un consumo basal aproximado de:

```text
95 mA
```

Las mediciones serán posteriormente comparadas y caracterizadas mediante instrumentación externa.

---

# EEPROM

Rosetta incluye memoria EEPROM externa mediante I2C1.

La unidad ensamblada utiliza actualmente:

```text
AT24C512C
64 KB
```

El firmware detecta la EEPROM y realiza pruebas de comunicación.

En versiones posteriores se utilizará también para almacenar:

- Número de serie.
- Revisión de hardware.
- Fecha de fabricación.
- Contadores de arranque.
- Parámetros persistentes.
- Calibraciones.
- Resultados de autodiagnóstico.

---

# SPI Flash

Rosetta integra:

```text
W25Q128JVSIQ
128 Mbit
16 MB
```

Interfaz:

```text
SPI3

PB3 → SCK
PB4 → MISO
PC1 → MOSI
PB5 → CS
```

El Acceptance Test verifica la identificación JEDEC del dispositivo.

---

# microSD / SDIO

Rosetta incorpora un socket microSD conectado mediante SDIO de 4 bits.

Pinout:

```text
PB2  → SDIO_CK
PD2  → SDIO_CMD
PC8  → SDIO_D0
PB0  → SDIO_D1
PB1  → SDIO_D2
PC11 → SDIO_D3
```

Las líneas:

```text
CMD
D0
D1
D2
D3
```

cuentan con resistencias pull-up de 10 kΩ.

La PCB no utiliza la línea física `CARD_DETECT` del socket.

Por este motivo, el firmware determina la presencia de una tarjeta intentando inicializarla.

---

# microSD Acceptance Test

La página 9 ejecuta una prueba bajo demanda.

Secuencia:

```text
SDIO initialization
↓
4-bit bus
↓
Card information
↓
FATFS mount
↓
Temporary file creation
↓
Write test pattern
↓
Read test pattern
↓
Compare
↓
Delete temporary file
```

La prueba no sobrescribe archivos existentes.

Los archivos temporales utilizan nombres del tipo:

```text
ROS00.TST
ROS01.TST
...
```

y se eliminan después de una prueba correcta.

---

# Estado actual de microSD

Durante la primera prueba se utilizó una microSD de aproximadamente 64 GB procedente de una impresora 3D y sin verificar previamente su formato.

Resultado:

```text
STATUS   ERR
SDIO     OK
FAT      ERR
R/W      ERR
SIZE     ~60000 MB
FATFS    1
```

Esto confirma que:

```text
STM32 ↔ microSD communication = OK
Card identification          = OK
Capacity detection           = OK
SDIO bus                     = OK
```

La prueba FATFS todavía debe repetirse con una tarjeta conocida y correctamente formateada.

`FATFS = 1` corresponde a:

```text
FR_DISK_ERR
```

La siguiente prueba recomendada es utilizar una microSD:

```text
≤ 32 GB
FAT32
```

Si FATFS y R/W pasan correctamente, el subsistema microSD podrá marcarse completamente como `PASS`.

El soporte exFAT podrá incorporarse posteriormente para tarjetas SDXC de mayor capacidad.

---

# Splash personalizado

El splash de Rosetta se almacena en:

```text
Rosetta/Inc/rosetta_logo.h
```

Para convertir una imagen a formato compatible con la OLED:

```powershell
python Tools/rosetta_splash_converter.py logo_rosetta.png Rosetta/Inc/rosetta_logo.h
```

La imagen se convierte a:

```text
128 × 64
1 bit
```

compatible con el framebuffer utilizado por la OLED.

Requiere Pillow:

```powershell
python -m pip install pillow
```

---

# Git Workflow

El desarrollo utiliza Git para conservar versiones estables y separar diferentes etapas de Rosetta.

Actualmente:

```text
main
└── Rosetta Bring-Up

acceptance-test
└── Rosetta Hardware Acceptance Test
```

Tags relevantes:

```text
bringup-v0.1.1
```

Se crearán tags adicionales cuando el Acceptance Test alcance una versión estable.

---

# Guardar cambios

Para guardar el estado actual:

```powershell
git status
git add .
git commit -m "Add SDIO and microSD acceptance test"
git push
```

Para crear un tag:

```powershell
git tag -a acceptance-v0.4 -m "Rosetta Acceptance Test with SDIO"
git push origin acceptance-v0.4
```

---

# Proyectos derivados

El repositorio base se utilizará posteriormente como punto de partida para proyectos independientes.

Ejemplos previstos:

```text
Rosetta-WakeMeUp
Rosetta-FlightComputer
Rosetta-CAN-Demo
Rosetta-Sensor-Demo
Rosetta-SD-Demo
```

Estos proyectos reutilizarán la configuración y drivers ya validados de Rosetta sin modificar el firmware de Acceptance Test.

---

# Próximos pasos

- Validar microSD FAT32 conocida.
- Comparar medición INA260 contra multímetro.
- Realizar múltiples cold boots.
- Verificar retención RTC mediante batería.
- Completar pruebas destructivas/no destructivas de EEPROM y SPI Flash.
- Documentar consumo mínimo, nominal y máximo.
- Crear identificación individual de cada PCB.
- Implementar watchdog y registro de causa de reset para proyectos de vuelo.
- Crear Rosetta Flight Computer como aplicación independiente.

---

# Estado de Rosetta Rev A.0

Actualmente la plataforma ha completado exitosamente su etapa inicial de bring-up y se encuentra en fase de **hardware acceptance testing**.

```text
POWER           PASS
STM32           PASS
SWD             PASS
HSE             PASS
SYSCLK 180 MHz  PASS
LSE             PASS
RTC             PASS
OLED            PASS
I2C1            PASS
INA260          PASS
EEPROM          PASS
SPI FLASH       PASS
USER KEY        PASS
BUZZER          PASS
SDIO            PASS
FATFS           PENDING
microSD R/W     PENDING
```

---

**Rosetta Flight Computer Platform**  
**Hardware Revision: A.0**  
**MCU: STM32F446RET6**