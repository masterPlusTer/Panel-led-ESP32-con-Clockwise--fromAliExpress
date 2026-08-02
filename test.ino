#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

// =====================================================
// CONFIGURACIÓN GENERAL
// =====================================================

#define TARGET_FPS 15

static const int PANEL_WIDTH  = 64;
static const int PANEL_HEIGHT = 64;
static const int PANEL_CHAIN  = 1;

// =====================================================
// PINOUT DEL KIT CLOCKWISE / ESP32-WROOM-32
// =====================================================

#define PIN_R1   25
#define PIN_G1   26
#define PIN_B1   27

#define PIN_R2   14
#define PIN_G2   12
#define PIN_B2   13

#define PIN_A    23
#define PIN_B    19
#define PIN_C     5
#define PIN_D    17
#define PIN_E    32

#define PIN_LAT   4
#define PIN_OE   15
#define PIN_CLK  16

MatrixPanel_I2S_DMA* display = nullptr;

// =====================================================
// SPRITE
// =====================================================
//
// 0 = transparente
// 1 = amarillo
// 2 = negro
// 3 = rojo
// 4 = verde
// 5 = azul
// 6 = blanco
// 7 = magenta
// 8 = cian
// 9 = naranja
//
// Transparente significa:
// "no dibujar nada en ese píxel".
//
// Negro significa:
// "dibujar un píxel negro explícitamente".
//

static const int FACE_W = 16;
static const int FACE_H = 16;

const uint8_t face[FACE_H][FACE_W] = {
  {0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0},
  {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
  {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},

  // Ojos con borde negro y brillo blanco
  {1,1,2,2,2,1,1,1,1,1,2,2,2,1,1,1},
  {1,1,2,6,2,1,1,1,1,1,2,6,2,1,1,1},
  {1,1,2,2,2,1,1,1,1,1,2,2,2,1,1,1},

  // Mejillas
  {1,7,7,1,1,1,1,1,1,1,1,1,1,7,7,1},

  // Nariz pequeña
  {1,1,1,1,1,1,9,9,9,1,1,1,1,1,1,1},

  // Sonrisa
  {1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1},
  {1,1,1,2,1,3,3,3,3,3,3,1,2,1,1,1},
  {1,1,2,1,1,3,6,6,6,6,3,1,1,2,1,1},
  {1,1,1,2,1,1,3,3,3,3,1,1,2,1,1,1},

  {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
  {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
  {0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0}
};

// Ahora hay 10 entradas: índices 0 a 9.
uint16_t palette[10];

// =====================================================
// OBJETO EN MOVIMIENTO
// =====================================================

struct MovingFace {
  float x;
  float y;
  float vx;
  float vy;
};

MovingFace obj;

// =====================================================
// COLOR CORRECTO PARA TU PANEL
// =====================================================
//
// El orden correcto confirmado es GBR.
//
// Entrada lógica:
//   rojo, verde, azul
//
// Salida física:
//   verde, azul, rojo
//

static inline uint16_t panelColor(
  uint8_t r,
  uint8_t g,
  uint8_t b
) {
  return display->color565(g, b, r);
}

// =====================================================
// CONSTRUIR PALETA
// =====================================================

void buildPalette() {
  palette[0] = panelColor(0, 0, 0);         // transparente
  palette[1] = panelColor(255, 220, 0);     // amarillo
  palette[2] = panelColor(0, 0, 0);         // negro
  palette[3] = panelColor(255, 0, 0);       // rojo
  palette[4] = panelColor(0, 255, 0);       // verde
  palette[5] = panelColor(0, 0, 255);       // azul
  palette[6] = panelColor(255, 255, 255);   // blanco
  palette[7] = panelColor(255, 0, 255);     // magenta
  palette[8] = panelColor(0, 255, 255);     // cian
  palette[9] = panelColor(255, 120, 0);     // naranja
}

// =====================================================
// DIBUJAR CARITA
// =====================================================

void drawFace(int x0, int y0) {
  for (int y = 0; y < FACE_H; y++) {
    const int screenY = y0 + y;

    if (screenY < 0 || screenY >= PANEL_HEIGHT) {
      continue;
    }

    for (int x = 0; x < FACE_W; x++) {
      const int screenX = x0 + x;

      if (screenX < 0 || screenX >= PANEL_WIDTH) {
        continue;
      }

      const uint8_t colorIndex = face[y][x];

      // 0 es transparente: no se dibuja.
      if (colorIndex == 0) {
        continue;
      }

      display->drawPixel(
        screenX,
        screenY,
        palette[colorIndex]
      );
    }
  }
}

// =====================================================
// ACTUALIZAR MOVIMIENTO
// =====================================================

void updateMovement() {
  obj.x += obj.vx;
  obj.y += obj.vy;

  if (obj.x <= 0.0f) {
    obj.x = 0.0f;
    obj.vx = fabsf(obj.vx);
  }

  if (obj.y <= 0.0f) {
    obj.y = 0.0f;
    obj.vy = fabsf(obj.vy);
  }

  const float maxX = PANEL_WIDTH - FACE_W;
  const float maxY = PANEL_HEIGHT - FACE_H;

  if (obj.x >= maxX) {
    obj.x = maxX;
    obj.vx = -fabsf(obj.vx);
  }

  if (obj.y >= maxY) {
    obj.y = maxY;
    obj.vy = -fabsf(obj.vy);
  }
}

// =====================================================
// CONFIGURAR MATRIZ
// =====================================================

void setupDisplay() {
  HUB75_I2S_CFG::i2s_pins pins = {
    PIN_R1,
    PIN_G1,
    PIN_B1,

    PIN_R2,
    PIN_G2,
    PIN_B2,

    PIN_A,
    PIN_B,
    PIN_C,
    PIN_D,
    PIN_E,

    PIN_LAT,
    PIN_OE,
    PIN_CLK
  };

  HUB75_I2S_CFG config(
    PANEL_WIDTH,
    PANEL_HEIGHT,
    PANEL_CHAIN,
    pins
  );

  /*
   * Configuración estable confirmada:
   *
   * - ESP32 Arduino Core 2.0.17
   * - versión antigua compatible de la librería HUB75
   * - driver FM6126A
   * - clkphase = true
   * - sin doble buffer
   */

  config.driver = HUB75_I2S_CFG::FM6126A;
  config.clkphase = true;
  config.double_buff = false;

  display = new MatrixPanel_I2S_DMA(config);

  if (!display->begin()) {
    Serial.println("ERROR: no se pudo iniciar la matriz.");

    while (true) {
      delay(1000);
    }
  }

  display->setBrightness8(40);
  display->clearScreen();

  Serial.printf(
    "Refresh calculado: %d Hz\n",
    display->calculated_refresh_rate
  );
}

// =====================================================
// SETUP
// =====================================================

void setup() {
  Serial.begin(115200);
  delay(500);

  setupDisplay();
  buildPalette();

  obj.x = 5.0f;
  obj.y = 5.0f;

  obj.vx = 0.65f;
  obj.vy = 0.48f;

  Serial.println();
  Serial.println("Carita movil iniciada.");
  Serial.println("Orden de color fijo: GBR.");
}

// =====================================================
// LOOP
// =====================================================

void loop() {
  const unsigned long frameStart = millis();

  // Borra todo el cuadro anterior con negro.
  display->fillScreen(palette[2]);

  drawFace(
    static_cast<int>(obj.x),
    static_cast<int>(obj.y)
  );

  updateMovement();

  const unsigned long targetFrameTime =
    1000UL / TARGET_FPS;

  const unsigned long elapsed =
    millis() - frameStart;

  if (elapsed < targetFrameTime) {
    delay(targetFrameTime - elapsed);
  }
}
