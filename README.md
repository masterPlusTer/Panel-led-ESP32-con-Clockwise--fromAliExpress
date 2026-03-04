# ESP32 64x64 RGB Matrix Clock (AliExpress Reverse Engineering)

Hace un tiempo compré en AliExpress este pequeño gadget:

![esp32matrix1](https://github.com/user-attachments/assets/9122d3a7-0c69-4042-b326-7ec08ad73e53)\
![esp32matrix2](https://github.com/user-attachments/assets/a6f5e992-4ea1-4b2f-8697-119879209f23)

Lamentablemente el producto ya no está disponible, y toda la
documentación oficial desapareció junto con la página del vendedor.\
Lo único que queda es esta pequeña imagen guardada en el historial de
compras.

------------------------------------------------------------------------

# Qué es este dispositivo

El dispositivo es bastante simple:

-   Un **ESP32‑WROOM‑32**
-   Un **panel RGB HUB75 de 64x64**
-   Un **buzzer**
-   Un **sensor de luz (LDR)**
-   Firmware preinstalado que muestra un **reloj estilo Super Mario**

El firmware original permitía instalar nuevos relojes desde la página de
**Clockwise**.

En teoría.

En la práctica... no es tan sencillo.

------------------------------------------------------------------------

# El problema

El sistema **solo funciona correctamente con el firmware original**.

Cuando se instala cualquier otro firmware desde Clockwise ocurre lo
siguiente:

-   Solo **la mitad del panel funciona**
-   La otra mitad queda apagada o muestra artefactos

Esto ocurre porque **las conexiones entre el ESP32 y el panel HUB75 no
siguen el pinout estándar**.

En otras palabras:\
el firmware asume un cableado normal... pero el fabricante decidió hacer
lo que quiso.

Así que me puse a hacer **ingeniería inversa del hardware** para mapear
todas las conexiones y poder usar el panel correctamente.

Y ya que estaba en eso, también aprovechar para hacer algo más
interesante que un simple reloj.

------------------------------------------------------------------------

# Hardware

## ESP32

El módulo usado es:

    ESP32‑WROOM‑32

------------------------------------------------------------------------

## Buzzer

    GPIO2
    Activo bajo

    GPIO2 = 0  -> suena
    GPIO2 = 1  -> silencio

------------------------------------------------------------------------

## Sensor de luz (LDR)

    GPIO34
    ADC1

Se puede usar para ajustar automáticamente el brillo de la matriz.

------------------------------------------------------------------------

# Conexión del panel HUB75

Este es el **pinout real del conector HUB75 según está cableado en esta
placa**:

  Pin HUB75   GPIO ESP32
  ----------- --------------------
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
  13          **GPIO32 (PIN E)**
  14          GPIO12
  15          GND
  16          GPIO26

⚠️ **El pin más importante es el PIN E conectado a GPIO32.**

Los paneles **64x64 HUB75 usan escaneo 1/32**, lo que significa que **el
pin E es obligatorio** para seleccionar las filas adicionales.

Si el firmware no usa ese pin correctamente:

-   solo se actualizan **16 filas**
-   el resto del panel queda muerto o duplicado

Esto explica por qué el firmware de Clockwise rompe la pantalla.

------------------------------------------------------------------------

# Objetivo de este proyecto

Este repositorio documenta:

-   el **hardware real** del dispositivo

-   el **mapeo correcto de pines**

-   cómo usar el panel con **ESP32‑HUB75‑MatrixPanel‑I2S‑DMA**

-   ejemplos para:

    -   relojes
    -   animaciones
    -   bitmaps personalizados
    -   sensores
    -   efectos visuales

La idea es convertir este gadget de AliExpress en algo **mucho más
interesante que un reloj con skins descargables**.

------------------------------------------------------------------------

# Estado actual

✔ Panel funcionando correctamente\
✔ Pinout completamente identificado\
✔ Soporte para panel 64x64\
✔ Control de brillo\
✔ Animaciones y bitmaps personalizados

Pendiente:

-   usar el **LDR para brillo automático**
-   soporte de **WiFi + NTP**
-   animaciones más complejas
-   editor de sprites

------------------------------------------------------------------------

# Notas finales

Si encontraste este repositorio porque también compraste uno de estos
relojes y te pasó lo mismo:

no estás loco.

El problema **no es el firmware**.

El problema es que el fabricante decidió cablear el panel de una forma
bastante creativa.






