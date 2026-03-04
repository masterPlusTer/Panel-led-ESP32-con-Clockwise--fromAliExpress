#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

#define TARGET_FPS 25

// ====== PANEL CONFIG (TU MATRIZ) ======
static const int PANEL_RES_X = 64;
static const int PANEL_RES_Y = 64;
static const int PANEL_CHAIN = 1;
static const int PIN_E = 32;

MatrixPanel_I2S_DMA *display = nullptr;

// ====== SPRITE CONFIG ======
static const int SPR_W = 16;
static const int SPR_H = 16;

// Paleta simple (índices 0..7). 0 = transparente
// Podés cambiar estos colores a gusto.
uint16_t pal[8];

// Sprite editable: 16x16, cada número es un índice a pal[]
// 0 = transparente, 1..7 = colores
// Dibujito ejemplo: “carita”
uint8_t sprite[SPR_H][SPR_W] = {
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,2,2,2,2,2,2,0,0,0,0,0},
  {0,0,0,0,2,2,2,2,2,2,2,2,0,0,0,0},
  {0,0,0,2,2,2,2,2,2,2,2,2,2,0,0,0},
  {0,0,2,2,2,2,2,2,2,2,2,2,2,2,0,0},
  {0,0,2,2,2,0,0,2,2,0,0,2,2,2,0,0},
  {0,2,2,2,2,0,0,2,2,0,0,2,2,2,2,0},
  {0,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0},
  {0,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0},
  {0,2,2,2,2,0,2,2,2,2,0,2,2,2,2,0},
  {0,0,2,2,2,2,0,0,0,0,2,2,2,2,0,0},
  {0,0,0,2,2,2,2,2,2,2,2,2,2,0,0,0},
  {0,0,0,0,2,2,2,2,2,2,2,2,0,0,0,0},
  {0,0,0,0,0,2,2,2,2,2,2,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
};

// ====== OBJETO QUE REBOTA ======
struct SpriteObj {
  float x, y;
  float vx, vy;
} obj;

// Dibuja sprite con transparencia
void drawSprite(int x0, int y0) {
  for (int y = 0; y < SPR_H; y++) {
    int yy = y0 + y;
    if (yy < 0 || yy >= display->height()) continue;

    for (int x = 0; x < SPR_W; x++) {
      int xx = x0 + x;
      if (xx < 0 || xx >= display->width()) continue;

      uint8_t idx = sprite[y][x];
      if (idx == 0) continue; // transparente

      display->drawPixel(xx, yy, pal[idx]);
    }
  }
}

void setup() {
  delay(300);
  Serial.begin(115200);
  delay(200);

  HUB75_I2S_CFG mxconfig(PANEL_RES_X, PANEL_RES_Y, PANEL_CHAIN);
  mxconfig.gpio.e = PIN_E;
  mxconfig.driver = HUB75_I2S_CFG::FM6126A;
  mxconfig.clkphase = true;

  // AHORA sí: doble buffer real
  mxconfig.double_buff = true;

  display = new MatrixPanel_I2S_DMA(mxconfig);

  if (!display->begin()) {
    Serial.println("DMA begin() falló.");
    while (true) delay(1000);
  }

  display->setBrightness8(90);
  display->clearScreen();

  // Paleta (0 = transparente, igual le damos un valor)
  pal[0] = display->color565(0, 0, 0);
  pal[1] = display->color565(255, 255, 255); // blanco
  pal[2] = display->color565(255, 210, 0);   // amarillo
  pal[3] = display->color565(0, 255, 0);     // verde
  pal[4] = display->color565(0, 0, 255);     // azul
  pal[5] = display->color565(255, 0, 0);     // rojo
  pal[6] = display->color565(255, 0, 255);   // magenta
  pal[7] = display->color565(0, 255, 255);   // cyan

  randomSeed(esp_random());

  obj.x = random(0, display->width()  - SPR_W);
  obj.y = random(0, display->height() - SPR_H);

  obj.vx = (random(10, 50) / 20.0f) * (random(0, 2) ? 1 : -1);
  obj.vy = (random(10, 50) / 20.0f) * (random(0, 2) ? 1 : -1);

  Serial.printf("Refresh: %d Hz\n", display->calculated_refresh_rate);
}

void loop() {
  // dibujar TODO en backbuffer
  display->clearScreen();

  // sprite
  drawSprite((int)obj.x, (int)obj.y);

  // flip: ahora mostrás el frame completo
  display->flipDMABuffer();

  // física simple + rebote
  obj.x += obj.vx;
  obj.y += obj.vy;

  if (obj.x <= 0) { obj.x = 0; obj.vx = abs(obj.vx); }
  if (obj.y <= 0) { obj.y = 0; obj.vy = abs(obj.vy); }

  if (obj.x + SPR_W >= display->width()) {
    obj.x = display->width() - SPR_W;
    obj.vx = -abs(obj.vx);
  }
  if (obj.y + SPR_H >= display->height()) {
    obj.y = display->height() - SPR_H;
    obj.vy = -abs(obj.vy);
  }

  delay(1000 / TARGET_FPS);
}
