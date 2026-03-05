#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

// =====================
// CONFIG DEL PANEL
// =====================
static const int PANEL_RES_X = 64;
static const int PANEL_RES_Y = 64;
static const int PANEL_CHAIN = 1;
static const int PIN_E = 32;

MatrixPanel_I2S_DMA *display = nullptr;

// =====================
// CONFIG DEL TEST
// =====================
static const uint8_t BRIGHTNESS  = 60;   // 20..90 (menos = menos ghosting)
static const uint8_t POWER_LIMIT = 150;  // 255 sin limite. probá 90..170
static const uint16_t HOLD_MS    = 250;  // más alto = más lento
static const uint8_t LINE_THICK  = 1;    // 1..3
static const uint8_t LEVELS      = 4;    // 4 => 0,85,170,255 (64 colores)

static inline uint8_t scale8(uint8_t v, uint8_t scale) {
  return (uint16_t)v * scale / 255;
}

static inline uint8_t levelTo8(uint8_t level) {
  if (LEVELS <= 1) return 0;
  return (uint16_t)level * 255 / (LEVELS - 1);
}

// Si tenés canales mezclados, podés remapear acá.
// Por ahora RGB normal:
static inline uint16_t panelColor(uint8_t r, uint8_t g, uint8_t b) {
  return display->color565(r, g, b);
}

static void clearBothBuffers() {
  display->clearScreen();
  display->flipDMABuffer();
  display->clearScreen();
  display->flipDMABuffer();
}

void setup() {
  delay(300);
  Serial.begin(115200);
  delay(200);

  Serial.println("Starting HUB75...");

  HUB75_I2S_CFG mxconfig(PANEL_RES_X, PANEL_RES_Y, PANEL_CHAIN);
  mxconfig.gpio.e = PIN_E;

  mxconfig.driver = HUB75_I2S_CFG::FM6126A;
  mxconfig.clkphase = true;

  // Doble buffer real
  mxconfig.double_buff = true;

  // Si tu versión lo soporta, bajar velocidad ayuda a evitar glitches:
  // mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_10M;
  // mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_8M;

  display = new MatrixPanel_I2S_DMA(mxconfig);

  if (!display->begin()) {
    Serial.println("DMA begin() falló.");
    while (true) delay(1000);
  }

  display->setBrightness8(BRIGHTNESS);

  // IMPORTANTÍSIMO: si no limpiás ambos buffers, aparece basura vieja (bloques rojos, etc.)
  clearBothBuffers();

  Serial.printf("Refresh rate calc: %d Hz\n", display->calculated_refresh_rate);
}

void loop() {
  static int y = 0;

  // Recorremos todos los colores “posibles” dentro de esta grilla (LEVELS^3)
  for (uint8_t rl = 0; rl < LEVELS; rl++) {
    for (uint8_t gl = 0; gl < LEVELS; gl++) {
      for (uint8_t bl = 0; bl < LEVELS; bl++) {

        // Base 0..255 por nivel
        uint8_t r = levelTo8(rl);
        uint8_t g = levelTo8(gl);
        uint8_t b = levelTo8(bl);

        // Limitar picos (esto baja ghosting por caída/ruido)
        r = scale8(r, POWER_LIMIT);
        g = scale8(g, POWER_LIMIT);
        b = scale8(b, POWER_LIMIT);

        uint16_t c = panelColor(r, g, b);

        // DIBUJAR en backbuffer
        display->clearScreen();

        // Línea horizontal en y, con grosor
        for (uint8_t t = 0; t < LINE_THICK; t++) {
          int yy = y + t;
          if (yy >= 0 && yy < display->height()) {
            display->drawFastHLine(0, yy, display->width(), c);
          }
        }

        // Mostrar frame completo
        display->flipDMABuffer();

        // micro-pausa: a veces reduce cola/ghosting por latch/blanking
        delayMicroseconds(200);

        // Aguantar el color para que lo veas
        delay(HOLD_MS);

        // Siguiente fila
        y++;
        if (y >= display->height()) y = 0;
      }
    }
  }

  // Cada vuelta completa, limpiamos ambos buffers por si tu panel “recuerda” basura.
  // (no debería, pero tu panel ya demostró que le gusta el caos)
  clearBothBuffers();
}
