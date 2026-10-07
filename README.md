# Rosetta Firmware Base v0.1

Primer firmware de bring-up para **Rosetta Rev A.0 / STM32F446RET6**.

## Objetivo

Al encender la placa sin conectar sensores o módulos externos:

1. Inicializa MCU y buses mínimos.
2. Detecta la OLED integrada.
3. Muestra una pantalla gráfica de inicio.
4. Ejecuta un self-test no destructivo de:
   - HSE 8 MHz
   - LSE 32.768 kHz
   - OLED I2C
   - INA260
   - EEPROM AT24C512C (configuración as-built)
   - Flash W25Q128JV
   - RTC interno
5. Muestra tensión/corriente medidas por INA260.
6. Enciende USER LED si todos los tests anteriores pasan.
7. KEY alterna entre página de estado y página de detalles.

## Qué NO marca como fallo en v0.1

No se prueban físicamente todavía:
- socket microSD (no hay Card Detect conectado al STM32);
- transceptor/cableado CAN (requiere otro nodo o fixture);
- conector USB (requiere host);
- UART/I2C/SPI de expansión (requieren dispositivo externo).

Es intencional: no queremos presentar como "FAIL" algo que no puede verificarse sin hardware externo.

# CubeMX: configuración mínima

MCU: STM32F446RET6Tx

## RCC
- HSE: Crystal/Ceramic Resonator
- LSE: Crystal/Ceramic Resonator
- HSE físico: 8 MHz
- LSE físico: 32.768 kHz

## SYS
- Debug: Serial Wire

## Clock
Objetivo:
- SYSCLK = 180 MHz
- HCLK = 180 MHz
- APB1 = 45 MHz
- APB2 = 90 MHz
- Clock de 48 MHz (cuando se activen USB/SDIO): usar PLLSAI, no asumir que PLLQ del PLL principal puede dar 48 MHz con SYSCLK=180 MHz.

Configuración principal típica desde HSE=8 MHz:
- PLLM = 4
- PLLN = 180
- PLLP = 2
- activar OverDrive

Cuando USB/SDIO se incorporen:
- configurar fuente de CK48 desde PLLSAI en CubeMX;
- una combinación válida es PLLSAIM=4, PLLSAIN=96, PLLSAIP=4 -> 48 MHz, siempre dejando que CubeMX valide el árbol.

## GPIO
PA15:
- GPIO_Output
- User Label: USER_LED
- Initial: Low

PB15:
- GPIO_Input
- User Label: USER_KEY
- Pull: No pull (la PCB ya tiene pull-up externo)

PB5:
- GPIO_Output
- User Label: FLASH_CS
- Initial: High

## I2C1
PB6 = I2C1_SCL
PB7 = I2C1_SDA
- I2C mode
- 100 kHz para primer bring-up
- 7-bit addressing
- no internal pull-up (la PCB tiene 4.7 kOhm)

## SPI3
PB3 = SPI3_SCK
PB4 = SPI3_MISO
PC1 = SPI3_MOSI
PB5 = FLASH_CS como GPIO

Ajustes:
- Master
- Full Duplex
- 8 bits
- CPOL Low
- CPHA 1 Edge
- NSS Software
- MSB First
- Prescaler 16 para bring-up inicial

## RTC
- Clock source: LSE
- Calendar enabled

# Integración

1. Genera el proyecto CMake en CubeMX.
2. Copia `Rosetta/` al root del proyecto.
3. Agrega `Rosetta/Inc` al include path.
4. Agrega todos los `.c` de `Rosetta/Src` al target de CMake.
5. Copia las líneas de `main_integration_snippet.c` a las USER CODE sections del `main.c`.
6. Compila.
7. Programa por ST-Link/SWD.

## Resultado esperado

Al encender:

- splash `ROSETTA / REV A.0 / FLIGHT COMPUTER`
- pantalla `SELF TEST`
- revisión INA260
- revisión EEPROM
- revisión FLASH
- revisión RTC/LSE
- pantalla final con estado y medición de entrada

KEY cambia a una segunda página con:
- MCU
- HSE/LSE
- dirección OLED
- dirección INA260
- JEDEC ID de Flash

## Notas de diagnóstico

### OLED no aparece
Primero medir:
- 3V3 en OLED
- SCL PB6
- SDA PB7

El driver prueba 0x3C y 0x3D automáticamente.

### INA260 ERR
El driver no confía solo en ACK I2C:
- busca direcciones 0x40..0x4F
- comprueba Manufacturer ID
- comprueba Die ID

### EEPROM ERR
El driver busca 0x50..0x57 y hace una lectura no destructiva con dirección interna de 16 bits.

### FLASH ERR
W25Q128JV esperado:
- JEDEC = EF 40 18

Si aparece otro valor, revisar SPI3 y CS PB5.

## Filosofía

Ninguna lectura usa HAL_MAX_DELAY.
Un periférico ausente no bloquea todo Rosetta.
El self-test registra el fallo y continúa.


## Display fix v0.1.1
Fuente 5x7 corregida y herramienta para cambiar el splash incluida en Tools/.
