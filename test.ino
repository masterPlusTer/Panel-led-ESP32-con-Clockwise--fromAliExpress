#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

#define PANEL_RES_X 64
#define PANEL_RES_Y 64
#define PANEL_CHAIN 1

HUB75_I2S_CFG mxconfig(PANEL_RES_X, PANEL_RES_Y, PANEL_CHAIN);
MatrixPanel_I2S_DMA *dma = nullptr;

void setup() {
  Serial.begin(115200);

  // Colores
  mxconfig.gpio.r1 = 25;
  mxconfig.gpio.g1 = 26;
  mxconfig.gpio.b1 = 27;
  mxconfig.gpio.r2 = 14;
  mxconfig.gpio.g2 = 12;
  mxconfig.gpio.b2 = 13;   // o 5 si no tienes 13

  // Direcciones de fila
  mxconfig.gpio.a  = 23;
  mxconfig.gpio.b  = 17;
  mxconfig.gpio.c  = 19;
  mxconfig.gpio.d  = 32;
  mxconfig.gpio.e  = 16;

  // Control
  mxconfig.gpio.clk = 5;
  mxconfig.gpio.lat = 4;
  mxconfig.gpio.oe  = 15;

  dma = new MatrixPanel_I2S_DMA(mxconfig);
  dma->begin();
  dma->setBrightness8(80);
  dma->clearScreen();
}

void loop() {
  // Fill blanco: si el cableado está bien, llena TODO sin bandas
  dma->fillScreen(dma->color565(255,255,255));
  delay(700);

  // Barrido de filas: debe bajar suave sin saltos
  dma->fillScreen(0);
  uint16_t green = dma->color565(0,255,0);
  for (int y=0; y<dma->height(); y++) {
    dma->fillScreen(0);
    dma->drawLine(0,y,dma->width()-1,y,green);
    delay(40);
  }
}
