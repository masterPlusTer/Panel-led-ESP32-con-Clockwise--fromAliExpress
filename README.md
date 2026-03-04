# ESP32 64x64 RGB Matrix Clock (AliExpress Reverse Engineering)

Hace un tiempo compré en AliExpress este pequeño gadget:

![esp32matrix1](https://github.com/user-attachments/assets/9122d3a7-0c69-4042-b326-7ec08ad73e53)\
![esp32matrix2](https://github.com/user-attachments/assets/a6f5e992-4ea1-4b2f-8697-119879209f23)

Lamentablemente el producto ya no está disponible, y toda la
documentación oficial desapareció junto con la página del vendedor.\
Lo único que queda es esta pequeña imagen guardada en el historial de
compras.

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
estándar**.\
El firmware de Clockwise asume una conexión estándar, pero el fabricante
decidió usar un mapeo completamente diferente.

Esto provoca que:

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

Sin esta línea funcionando correctamente:

-   solo se actualizan **16 filas**
-   el resto del panel no responde correctamente

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

# RGB Channel Swap

Durante las pruebas se detectó que los colores no coinciden con el orden
RGB esperado.

Comportamiento observado:

  Color esperado   Color mostrado
  ---------------- ----------------
  Red              Green
  Green            Blue
  Blue             Red

Esto significa que los canales están rotados:

    RGB → GBR

La solución es remapear los canales en software antes de convertir a
RGB565.

Ejemplo:

``` cpp
static inline uint16_t panel565(uint8_t r, uint8_t g, uint8_t b) {
    return display->color565(g, b, r);
}
```

------------------------------------------------------------------------

# Power Stability Issues

Al mostrar colores muy brillantes (especialmente **blanco o amarillo**)
pueden aparecer:

-   píxeles aleatorios
-   destellos
-   glitches visuales

Esto ocurre porque los paneles HUB75 generan **picos de corriente muy
rápidos** debido al multiplexado.

La fuente original es:

    5V 8A switching supply

Aunque es suficiente en teoría, los picos de corriente pueden provocar
pequeñas caídas de tensión.

## Solución

Agregar capacitancia adicional entre **5V y GND** cerca del panel.

Capacitor recomendado:

    1000µF – 2200µF

Ejemplo:

    5V ----+---- panel
           |
           | + 
         [2200µF]
           | -
           |
    GND ---+---- panel

Esto reduce significativamente los artefactos visuales.

------------------------------------------------------------------------

# Reverse Engineering Notes

El dispositivo parece estar construido a partir de:

-   4 submódulos LED 32×32
-   drivers tipo **FM6126A**
-   multiplexado 1/32

Esto explica:

-   la necesidad de la línea **E**
-   la sensibilidad a caídas de tensión
-   algunos artefactos cuando el brillo es alto

------------------------------------------------------------------------

# Current Status

✔ Panel funcionando correctamente\
✔ Pinout completamente identificado\
✔ Soporte completo para panel 64×64\
✔ Corrección de canales RGB\
✔ Animaciones y sprites personalizados

Pendiente:

-   control automático de brillo con LDR
-   sincronización NTP
-   animaciones más complejas
-   editor de sprites

------------------------------------------------------------------------

# How to Repurpose This Device

Una vez entendido el hardware, este dispositivo puede usarse como:

-   pantalla LED programable
-   reloj NTP conectado a WiFi
-   panel de animaciones
-   display para sensores IoT
-   pequeño tablero de notificaciones

Básicamente se puede convertir en **una pantalla LED HUB75 completamente
programable basada en ESP32**.

------------------------------------------------------------------------

# Notes

Si llegaste aquí porque compraste uno de estos relojes y el firmware
alternativo no funciona:

no estás loco.

El problema no es el firmware.

El problema es que el fabricante decidió cablear el panel de una forma
bastante... creativa.





