# Rosetta - arranque LSE no bloqueante (opcional)

Esta variante existe para que la OLED, buzzer y Acceptance Test puedan comenzar
sin esperar a que el cristal de 32.768 kHz llegue a LSERDY.

## Importante

El bloqueo ocurre antes de `main()` si CubeMX mete LSE dentro de
`SystemClock_Config()`. Por eso **no basta** con hacer polling en `Rosetta_Update()`:
para un arranque realmente inmediato hay que impedir que Cube/HAL habilite LSE
durante `SystemClock_Config()`.

La secuencia de esta variante es:

1. HSI/HSE + PLL -> sistema a 180 MHz.
2. GPIO/I2C/SPI/TIM -> OLED y buzzer disponibles.
3. `Rosetta_Init()` -> splash/Acceptance Test.
4. `RosettaLSEAsync_Begin()` -> pone LSEON y regresa inmediatamente.
5. `RosettaLSEAsync_Update()` -> comprueba LSERDY en cada vuelta.
6. Cuando LSERDY = 1 -> selecciona LSE como RTC source e inicializa RTC.

No resetea el backup domain si ya existe una fuente RTC distinta, porque hacerlo
borraría la hora/fecha retenida por VBAT.

## Recomendación

Primero deja el Acceptance Test funcionando con tu configuración CubeMX actual.
Haz un commit. Después prueba esta variante en otro commit dentro de la misma
rama `acceptance-test`. Así `git restore` / `git switch` siempre te devuelve al
estado que ya sabes que funciona.
