# ESP32 64x64 RGB Matrix Clock (AliExpress Reverse Engineering)

Hace un tiempo compré en AliExpress este pequeño gadget:

![esp32matrix1](https://github.com/user-attachments/assets/9122d3a7-0c69-4042-b326-7ec08ad73e53)\
![esp32matrix2](https://github.com/user-attachments/assets/a6f5e992-4ea1-4b2f-8697-119879209f23)

El producto ya no está disponible y prácticamente **no existe
documentación oficial**.\
Este repositorio documenta el **hardware real del dispositivo**, el
**pinout correcto** y los problemas encontrados al intentar usar
firmware alternativo.

------------------------------------------------------------------------

# Hardware Architecture

El dispositivo está compuesto por:

-   **ESP32‑WROOM‑32**
-   **Panel RGB HUB75 64×64**
-   **Buzzer**
-   **Sensor de luz (LDR)**
-   **Fuente de alimentación 5V**

Diagrama simplificado:

               +------------------+
               |      ESP32       |
               |                  |
               | GPIO2  -> Buzzer|
               | GPIO34 -> LDR   |
               |                  |
               | HUB75 Signals    |
               +--------+---------+
                        |
                        |
                 +------v------+
                 | 64x64 RGB   |
                 | HUB75 Panel |
                 +-------------+

------------------------------------------------------------------------

# Firmware Compatibility Issue

El firmware original funciona correctamente.

Sin embargo, cuando se instala firmware alternativo desde **Clockwise**,
solo **la mitad del panel funciona**.

La razón es que el panel HUB75 **no está conectado con el pinout
estándar**.

El firmware asume una conexión estándar pero el fabricante decidió usar
un mapeo completamente diferente.

Esto provoca:

-   solo se actualicen **16 filas**
-   el resto del panel quede apagado o muestre artefactos

------------------------------------------------------------------------

# HUB75 Pin Mapping

El cableado real entre el ESP32 y el panel es:

  Pin HUB75   GPIO ESP32
  ----------- ---------------------
  1           GPIO25
  2           GPIO27
  3           GPIO14
  4           GPIO9
  5           GPIO23
  6           GPIO5
  7           GPIO16
  8           GPIO15
  9           GND
  10          GPIO4
  11          GPIO17
  12          GPIO19
  13          **GPIO32 (E line)**
  14          GPIO12
  15          GND
  16          GPIO26

⚠ **La línea E es obligatoria para paneles 64×64.**

Los paneles HUB75 64×64 usan multiplexado **1/32**, por lo que la línea
**E selecciona las filas adicionales**.

------------------------------------------------------------------------

# Peripheral Connections

## Buzzer

    GPIO2
    Activo bajo

    GPIO2 = 0  -> suena
    GPIO2 = 1  -> silencio

## Sensor de luz (LDR)

    GPIO34
    ADC1

Puede usarse para ajustar automáticamente el brillo del panel.

------------------------------------------------------------------------

# LED Matrix Scan Architecture

El panel **no enciende todos los LEDs al mismo tiempo**.

Usa multiplexado de filas:

    1/32 scan

Solo **dos filas se activan simultáneamente**.

Secuencia simplificada:

    Step 1
    Row 0 + Row 32

    Step 2
    Row 1 + Row 33

    Step 3
    Row 2 + Row 34

    ...

    Step 32
    Row 31 + Row 63

Esto ocurre cientos de veces por segundo.

Debido a este sistema pueden aparecer:

-   ghosting
-   artefactos
-   sensibilidad al brillo

------------------------------------------------------------------------

# HUB75 Address Lines

Las filas se seleccionan usando:

    A
    B
    C
    D
    E

Para un panel 64×64:

    A B C D E = selector de fila

Ejemplo:

    00000 → filas 0 / 32
    00001 → filas 1 / 33
    00010 → filas 2 / 34
    ...
    11111 → filas 31 / 63

Sin la línea **E** solo se pueden direccionar **16 filas**.

------------------------------------------------------------------------

# RGB Channel Swap

Durante las pruebas se detectó que los colores no coinciden con el orden
RGB esperado.

  Color esperado   Color real
  ---------------- ------------
  Red              Green
  Green            Blue
  Blue             Red

Esto significa que el panel realmente funciona como:

    RGB → GBR

Solución en software:

``` cpp
static inline uint16_t panel565(uint8_t r, uint8_t g, uint8_t b) {
    return display->color565(g, b, r);
}
```

------------------------------------------------------------------------

# Display Artifacts

Durante las pruebas se observaron artefactos visuales:

-   reflejos rojos
-   ghosting
-   píxeles fantasma cerca de sprites brillantes

Esto ocurre principalmente cuando:

-   el sprite está en ciertas posiciones del panel
-   se usan colores muy brillantes (blanco o amarillo)

Esto es típico en paneles HUB75 económicos.

------------------------------------------------------------------------

# Power Stability

Los paneles HUB75 generan **picos de corriente muy rápidos**.

Incluso con una fuente:

    5V 8A

pueden aparecer:

-   píxeles aleatorios
-   ghosting
-   glitches

### Solución

Agregar capacitancia adicional.

    1000µF – 2200µF

Conexión:

    5V ----+---- panel
           |
         [2200µF]
           |
    GND ---+---- panel

Esto reduce significativamente los artefactos.

------------------------------------------------------------------------

# Internal Hardware Layout

Después de desmontar el dispositivo se observa:

    +-------------------------------------+
    |                                     |
    |         RGB LED MATRIX              |
    |                                     |
    |   [driver IC] [driver IC]           |
    |   [driver IC] [driver IC]           |
    |                                     |
    |        HUB75 DATA CONNECTOR         |
    |                                     |
    +-------------------------------------+

               |
               | ribbon cable
               |

    +-----------------------------+
    |         ESP32 BOARD         |
    |                             |
    |  WiFi MCU                   |
    |                             |
    |  USB Programming Interface  |
    |                             |
    +-----------------------------+

------------------------------------------------------------------------

# Identified Components

### ESP32

Controlador principal.

Responsable de:

-   WiFi
-   sincronización de tiempo
-   control del panel HUB75

### LED Drivers

El panel parece usar chips similares a:

    FM6126A

Estos manejan:

-   PWM RGB
-   multiplexado
-   shift registers

------------------------------------------------------------------------

# Observed Hardware Quirks

  Issue                         Description
  ----------------------------- ---------------------------------------------
  Pinout no estándar            GPIOs no coinciden con placas HUB75 típicas
  RGB rotado                    Orden GBR
  Ghosting                      Visible con alto brillo
  Sensibilidad a alimentación   Mejorado con capacitor grande

------------------------------------------------------------------------

# Current Status

✔ Panel funcionando\
✔ Pinout identificado\
✔ Soporte completo 64×64\
✔ Animaciones y sprites funcionando

Pendiente:

-   brillo automático con LDR
-   sincronización NTP
-   editor de sprites

------------------------------------------------------------------------

# How to Repurpose This Device

Una vez entendido el hardware, este dispositivo puede convertirse en:

-   reloj WiFi
-   display programable
-   panel de animaciones
-   tablero de sensores
-   mini display IoT

Básicamente una **pantalla HUB75 totalmente programable basada en
ESP32**.

------------------------------------------------------------------------

# Final Note

Si llegaste aquí porque compraste uno de estos relojes y el firmware
alternativo no funciona:

no es tu culpa.

El fabricante decidió cablear el panel de una forma bastante creativa.





