# Rosetta Acceptance Test v0.4 — microSD / SDIO

Esta versión añade la **página 9/9** al Acceptance Test estable.

No incluye ni requiere LSE asíncrono. Mantén exactamente el HSE/LSE/RTC que ya
te funciona.

## Qué prueba la página 9

Al entrar a `9/9 MICROSD / SDIO`, Rosetta ejecuta el test una sola vez:

1. Inicializa la tarjeta por SDIO.
2. Cambia a bus de 4 bits.
3. Lee la información de la tarjeta y calcula su capacidad.
4. Monta FATFS.
5. Crea un archivo temporal `ROSxx.TST` usando `FA_CREATE_NEW`.
6. Escribe 64 bytes de patrón.
7. Cierra y vuelve a abrir.
8. Lee los 64 bytes y compara.
9. Borra el archivo temporal.

No sobrescribe archivos existentes. Si `ROS00.TST` existe, busca otro nombre
hasta `ROS99.TST`.

Volver a salir y entrar a la página 9 vuelve a ejecutar la prueba; esto permite
insertar una tarjeta después del arranque.

## Importante: Rosetta A.0 no tiene CARD_DETECT

El socket no lleva una señal física `CARD_DETECT` al STM32. Por ello:

- `CARD: NO RESPONSE` puede significar socket vacío, tarjeta dañada, tarjeta
  mal insertada o fallo eléctrico del bus/conector.
- Con una **microSD conocida y funcional** insertada, un PASS sí valida de
  extremo a extremo el conector y las seis señales SDIO.

Por eso una tarjeta ausente se muestra como `--` / UNTESTED y no como FAIL.

## CubeMX

### 1. SDIO

Activa `SDIO -> SD 4 bits Wide bus`.

Usa exactamente el ruteo de Rosetta A.0:

- PB2  -> SDIO_CK
- PD2  -> SDIO_CMD
- PC8  -> SDIO_D0
- PB0  -> SDIO_D1
- PB1  -> SDIO_D2
- PC11 -> SDIO_D3

CubeMX puede escoger PC12/PC9/PC10 por defecto; si lo hace, remapea CK/D1/D2
a PB2/PB0/PB1.

### 2. Reloj de 48 MHz

SDIO necesita el dominio de 48 MHz correctamente configurado.

Con HSE = 8 MHz y SYSCLK = 180 MHz de Rosetta puedes conservar el PLL principal
tal como está y generar 48 MHz con PLLSAI. Una combinación válida es:

- PLLSAIM = 4
- PLLSAIN = 96
- PLLSAIP = 4

2 MHz x 96 / 4 = 48 MHz.

Selecciona PLLSAI como fuente del dominio de 48 MHz en Clock Configuration.
Si CubeMX resuelve automáticamente 48.000 MHz de otra manera válida, puedes
usar su configuración.

### 3. FATFS

Activa `Middleware -> FATFS -> SD Card`.

Para esta primera validación usa el driver polling/no-DMA si CubeMX ofrece la
elección; DMA puede añadirse después.

### 4. MUY IMPORTANTE: no inicialices SDIO al arranque

El `MX_SDIO_SD_Init()` generado suele terminar en `Error_Handler()` si el socket
está vacío. Rosetta debe poder arrancar sin tarjeta.

Mantén generado `sdio.c`, `sdio.h` y el handle `hsd`, pero evita la llamada
automática a:

    MX_SDIO_SD_Init();

Si tu CubeMX muestra `Project Manager -> Advanced Settings -> Generated
Function Calls`, marca SDIO como **Do Not Generate Function Call**.

Sí deja:

    MX_FATFS_Init();

porque sólo registra el driver FATFS y no debe bloquear el boot por socket vacío.

## Integración Rosetta

Copia:

- `Rosetta/Inc/rosetta_sd.h`
- `Rosetta/Src/rosetta_sd.c`

y sustituye los archivos Rosetta actualizados de esta versión.

En el handle agrega:

    .sd = &hsd,

Añade a CMake:

    ${CMAKE_CURRENT_SOURCE_DIR}/Rosetta/Src/rosetta_sd.c

## Resultado esperado

Sin tarjeta:

    MICROSD / SDIO
    CARD: NO RESPONSE
    INSERT KNOWN-GOOD
    CARD + REENTER
    STATUS: --

Con tarjeta válida:

    MICROSD / SDIO
    STATUS: OK
    SDIO:OK FAT:OK
    R/W: OK
    SIZE: xxxx MB
    FATFS: 0

`FATFS: 0` corresponde a `FR_OK`.

## Git

Antes de integrar:

    git add .
    git commit -m "Acceptance Test stable before SDIO"
    git push

Cuando la página 9 funcione:

    git add .
    git commit -m "Add microSD SDIO acceptance test"
    git push

