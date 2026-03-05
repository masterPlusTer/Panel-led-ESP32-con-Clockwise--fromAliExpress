# ESP32 64×64 RGB Matrix Clock (AliExpress Reverse Engineering)

Este repo existe porque (como es tradición) un vendedor de AliExpress vendió un producto “compatible” y después desapareció del mapa con toda la documentación.  
Resultado: **ingeniería inversa** a la vieja usanza.

## TL;DR

- Dispositivo: **ESP32‑WROOM‑32 + panel HUB75 RGB 64×64 + buzzer + LDR**
- El firmware original funciona (reloj “Super Mario”).
- “Clockwise” y otros firmwares genéricos **no** funcionan bien porque el fabricante usó un **pinout no estándar**, especialmente **PIN E**.
- Además, el panel/driver parece venir con **orden de color distinto (RGB → GBR)**.
- Hay **ghosting / reflejos** (especialmente abajo) que mejoran con **alimentación decente + capacitor grande + brillo moderado + timing correcto**.
- Algunas “bandas diagonales” o “franjas” **solo aparecen en fotos** (rolling shutter de la cámara + PWM del panel).

---

# 1) El bicho

Fotos del dispositivo:

![esp32matrix1](https://github.com/user-attachments/assets/9122d3a7-0c69-4042-b326-7ec08ad73e53)  
![esp32matrix2](https://github.com/user-attachments/assets/a6f5e992-4ea1-4b2f-8697-119879209f23)

Lo que era (promesa de marketing):
- Reloj preinstalado “Mario”
- “Se le pueden instalar más relojes desde Clockwise”

Lo que es (realidad):
- **Funciona bien solo con el firmware que trae**
- Si cargás firmware genérico sin configurar el panel, **funciona media pantalla / filas raras / colores cruzados**

---

# 2) Hardware Architecture

Componentes identificados:

- **ESP32‑WROOM‑32**
- **Panel RGB HUB75 64×64** (multiplexado 1/32)
- **Buzzer**
- **LDR** (sensor de luz)
- Entrada **5V** (USB/step-down según versión del dispositivo)

Diagrama simplificado:

```
           +------------------+
           |      ESP32       |
           |                  |
           | GPIO2  -> Buzzer |
           | GPIO34 -> LDR    |
           |                  |
           | HUB75 Signals    |
           +--------+---------+
                    |
                    | ribbon cable
                    v
             +--------------+
             | 64×64 HUB75  |
             | RGB Matrix   |
             +--------------+
```

---

# 3) El problema principal: pinout “creativo” (y el PIN E)

El panel es **64×64**, así que normalmente es **1/32 scan** y necesita la línea **E** para direccionar todas las filas.

Cuando cargábamos firmware alternativo sin E:
- solo se actualizaban partes del panel (por ejemplo “primera tanda de 16 filas” y otra “tanda”)
- el resto quedaba apagado o con artefactos

**Solución clave:** configurar **PIN E** en el ESP32.

Ejemplo (Arduino + ESP32-HUB75-MatrixPanel-I2S-DMA):

```cpp
HUB75_I2S_CFG mxconfig(64, 64, 1);
mxconfig.gpio.e = 32;           // <<<<<< PIN E (CLAVE EN ESTE HARDWARE)
```

---

# 4) Pin mapping real (HUB75 → ESP32)

Cableado real entre el ESP32 y el conector HUB75 del panel (como viene en este dispositivo):

| Pin HUB75 | Señal (según panel) | GPIO ESP32 |
|---:|---|---|
| 1  | (R1 típico) | GPIO25 |
| 2  | (G1 típico) | GPIO27 |
| 3  | (B1 típico) | GPIO14 |
| 4  | (R2 típico) | GPIO09 |
| 5  | (G2 típico) | GPIO23 |
| 6  | (B2 típico) | GPIO05 |
| 7  | A | GPIO16 |
| 8  | B | GPIO15 |
| 9  | GND | GND |
| 10 | LAT / STB (según placa) | GPIO04 |
| 11 | D | GPIO17 |
| 12 | C | GPIO19 |
| 13 | **E** | **GPIO32** ✅ (MUY IMPORTANTE) |
| 14 | OE | GPIO12 |
| 15 | GND | GND |
| 16 | CLK | GPIO26 |

⚠ Nota: algunos nombres “típicos” (R1/G1/B1, LAT, OE, CLK) pueden variar en serigrafía.  
Acá lo importante es el **mapeo real a GPIO**, no la etiqueta bonita.

---

# 5) Periféricos

## 5.1 Buzzer
- **GPIO2**
- **Activo-bajo**
  - GPIO2 = 0 → suena
  - GPIO2 = 1 → silencio

## 5.2 LDR (sensor de luz)
- **GPIO34 (ADC1)**
- Útil para brillo automático (pendiente/por implementar).

---

# 6) Driver IC (panel)

En fotos del reverso aparece el IC de driver. Por comportamiento y pruebas:
- funcionan configuraciones tipo **FM6126A** / **FM6124** (depende del panel real)
- En tu caso, **FM6126A** fue el que “simplemente anduvo”.

Ejemplo:

```cpp
mxconfig.driver = HUB75_I2S_CFG::FM6126A;
// mxconfig.driver = HUB75_I2S_CFG::FM6124;
```

---

# 7) Multiplexado / Scan (por qué esto es sensible)

Panel 64×64 típico:
- **1/32 scan**
- Selección de filas con: **A, B, C, D, E**
- Se actualizan filas por “barrido” a alta velocidad.

Simplificado:

```
Step 1  : Row 0  + Row 32
Step 2  : Row 1  + Row 33
...
Step 32 : Row 31 + Row 63
```

Si **E no está cableado/configurado**, el panel no puede direccionar correctamente las filas extra → media pantalla muerta o duplicada.

---

# 8) Colores “entreverados” (RGB swap / orden real)

Durante pruebas, los colores no coincidían con lo esperado.

Observación típica en tu panel:

| Color pedido | Color que ves |
|---|---|
| Rojo | Verde |
| Verde | Azul |
| Azul | Rojo |
| Amarillo | Cyan (dependiendo mezcla) |
| Cyan | Magenta (según mezcla) |

Esto sugiere un orden real tipo:

**RGB → GBR**

## 8.1 Solución práctica (software)
Crear un helper para mapear colores al orden real del panel:

```cpp
static inline uint16_t panel565(MatrixPanel_I2S_DMA* d, uint8_t r, uint8_t g, uint8_t b) {
  // Panel en orden GBR:
  return d->color565(g, b, r);
}
```

Y después usar `drawPixel(x,y, panel565(...))` o precalcular paleta con ese orden.

---

# 9) “Bandas diagonales / franjas” que solo salen en la foto

Esto es *clásico*:
- PWM del panel + barrido (scan) + rolling shutter de cámara
- Resultado: la cámara “ve” bandas que el ojo no ve.

Si el ojo no las ve y el panel se ve bien en vivo, no es un problema real del panel. Es tu cámara intentando entender la realidad.

---

# 10) Ghosting / reflejos / deformaciones (especialmente abajo)

Sí, eso puede ser **eléctrico**, **timing**, o **ambos**.

Síntomas observados:
- al bajar el sprite o al acercarse a la mitad inferior:
  - “reflejo rojo” a un costado
  - números/sprites “borrosos” o “deformados”
  - destellos en otras partes al usar **blanco/amarillo** (colores con más LEDs encendidos simultáneamente)

Causas típicas en HUB75:
- alimentación insuficiente (picos de corriente)
- cables finos/largos → caída de tensión
- brillo alto
- refresh/timing borderline (clkphase / i2sspeed / latch timing)
- ruido/masa floja
- panel barato con drivers meh

---

# 11) Alimentación (esto NO es opcional)

El panel 64×64 puede consumir bastante, sobre todo con blanco.

- Fuente usada: **5V 8A** (según foto)
- Igual pueden aparecer glitches por picos.

## 11.1 Capacitor “gordo”
Recomendado:
- **1000µF a 2200µF** (o más) electrolítico, cerca del panel

Ubicación:
- lo más cerca posible entre **5V y GND** **en la entrada de alimentación del panel** (o en pads/borneras de power).

Esquema:

```
5V ----+---- panel +
       |
     [ 2200µF ]
       |
GND ---+---- panel -
```

Polaridad:
- **+** al 5V
- **-** al GND

## 11.2 Cableado de power
- cables gruesos
- masa sólida
- evitar que el ESP32 y el panel compartan retorno finito y ruidoso

---

# 12) Mitigaciones por software (cuando la física te odia)

Lo que ayudó en tus pruebas:

- **Bajar brillo** (por ejemplo 60–120 sobre 255)
- Activar **double buffering** y flip con timing razonable
- Ajustar **clkphase** (en tu hardware: `true` te funcionó mejor)
- Mantener FPS moderado (ej. 25 FPS)

Ejemplo base de config “la que anda”:

```cpp
HUB75_I2S_CFG mxconfig(64, 64, 1);
mxconfig.gpio.e = 32;
mxconfig.driver = HUB75_I2S_CFG::FM6126A;
mxconfig.clkphase = true;
mxconfig.double_buff = true;   // para flips limpios
```

---

# 13) Problemas típicos de compilación (los que ya te pegaron)

## 13.1 `SCAN_16` / `SHIFT` “no existe”
Causa: versión/fork incorrecto de la librería o bibliotecas duplicadas.

Solución:
- dejar **una sola** librería (la correcta)
- borrar carpetas duplicadas en `Documents/Arduino/libraries`
- reiniciar Arduino IDE

## 13.2 Errores I2S con ejemplos viejos
Causa: ejemplos/librerías viejas incompatibles con core ESP32 moderno.

Solución:
- usar ejemplos de la librería actual
- actualizar código viejo

## 13.3 `undefined reference to setup()/loop()`
Causa: archivo incompleto o estructura `.ino` rota.

---

# 14) Setup recomendado (Arduino IDE)

- Board: **ESP32 Dev Module** (o similar para WROOM-32)
- Core ESP32: el que ya tenés funcionando (mencionaste 3.3.7)
- Librería: **ESP32-HUB75-MatrixPanel-I2S-DMA**

---

# 15) Estado actual

✅ Pinout identificado  
✅ 64×64 completo con **E**  
✅ Driver: **FM6126A**  
✅ Doble buffer funciona  
✅ Animaciones/sprites andando

Pendiente:
- brillo automático con LDR
- NTP en firmware custom
- convertidor/editor de sprites

---

# 16) Nota final

Si tu panel “solo prende media pantalla” con firmware alternativo:  
no es tu culpa. El fabricante cableó esto con creatividad.




