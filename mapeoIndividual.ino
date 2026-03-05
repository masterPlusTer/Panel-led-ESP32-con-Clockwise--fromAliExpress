// escribir help en la consola y ahi te dice lo que tienes que hacer, esto es una herramienta util para mapear y entender el display


#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

#define W 64
#define H 64
#define CHAIN 1
#define PIN_E 32

static constexpr bool CLKPHASE = true;   // vos dijiste que con true se ve menos borroso
static constexpr uint8_t BRIGHT = 40;

MatrixPanel_I2S_DMA *display = nullptr;

struct RGB { uint8_t r, g, b; };
static RGB bmp[H][W];

// Tu panel: RGB -> GBR
static inline RGB panelRGB(uint8_t r, uint8_t g, uint8_t b) {
  return RGB{ g, b, r };
}

static void clearBmp(uint8_t r=0, uint8_t g=0, uint8_t b=0) {
  for (int y=0; y<H; y++) for (int x=0; x<W; x++) bmp[y][x] = {r,g,b};
}

static void pushBmp() {
  for (int y=0; y<H; y++) {
    for (int x=0; x<W; x++) {
      display->drawPixelRGB888(x, y, bmp[y][x].r, bmp[y][x].g, bmp[y][x].b);
    }
  }
  display->flipDMABuffer();
}

enum Mode { SinglePixel=0, Row=1, Col=2, Manual=3 };
static Mode mode = SinglePixel;

static void serialHelp() {
  Serial.println();
  Serial.println("Commands:");
  Serial.println("  help          -> this help");
  Serial.println("  m N           -> mode 0=SinglePixel 1=Row 2=Col 3=Manual");
  Serial.println("  c             -> clear");
  Serial.println("  p x y r g b   -> set pixel (0..63) (0..255)");
  Serial.println("Examples:");
  Serial.println("  m 3");
  Serial.println("  p 0 0 255 0 0");
  Serial.println("  p 63 63 0 0 255");
  Serial.println();
}

static void handleSerial() {
  if (!Serial.available()) return;
  String line = Serial.readStringUntil('\n');
  line.trim();
  if (!line.length()) return;

  if (line == "help" || line == "h") { serialHelp(); return; }

  if (line == "c") {
    clearBmp(0,0,0);
    display->clearScreen(); display->flipDMABuffer();
    display->clearScreen(); display->flipDMABuffer();
    pushBmp();
    Serial.println("Cleared.");
    return;
  }

  if (line.startsWith("m ")) {
    int m = line.substring(2).toInt();
    if (m < 0) m = 0;
    if (m > 3) m = 3;
    mode = (Mode)m;
    Serial.print("Mode="); Serial.println(m);
    return;
  }

  if (line.startsWith("p ")) {
    int x,y,r,g,b;
    int n = sscanf(line.c_str(), "p %d %d %d %d %d", &x,&y,&r,&g,&b);
    if (n == 5 && x>=0 && x<W && y>=0 && y<H) {
      bmp[y][x] = panelRGB((uint8_t)r,(uint8_t)g,(uint8_t)b);
      pushBmp();
      Serial.printf("Pixel (%d,%d) set\n", x, y);
    } else {
      Serial.println("Bad command. Use: p x y r g b");
    }
    return;
  }

  Serial.println("Unknown command. Type 'help'.");
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("Booting pixel test...");

  HUB75_I2S_CFG cfg(W, H, CHAIN);

  // ✅ lo mínimo que sabemos que necesitás
  cfg.gpio.e = PIN_E;
  cfg.driver = HUB75_I2S_CFG::FM6126A;
  cfg.clkphase = CLKPHASE;
  cfg.double_buff = true;

  // Si tu versión tiene esta opción, podés probarla después:
  // cfg.i2sspeed = HUB75_I2S_CFG::HZ_10M;

  display = new MatrixPanel_I2S_DMA(cfg);
  display->begin();
  display->setBrightness8(BRIGHT);

  // limpia ambos buffers
  display->clearScreen(); display->flipDMABuffer();
  display->clearScreen(); display->flipDMABuffer();

  clearBmp(0,0,0);
  pushBmp();

  Serial.println("READY. Type 'help'.");
}

void loop() {
  handleSerial();

  static uint32_t t0 = 0;
  static int x = 0, y = 0;

  if (mode == Manual) { delay(5); return; }

  if (millis() - t0 < 40) return;
  t0 = millis();

  clearBmp(0,0,0);

  // marcadores de esquina (para ver offsets/corrimiento)
  bmp[0][0]       = panelRGB(255,0,0);
  bmp[0][W-1]     = panelRGB(0,255,0);
  bmp[H-1][0]     = panelRGB(0,0,255);
  bmp[H-1][W-1]   = panelRGB(255,255,0);

  if (mode == SinglePixel) {
    bmp[y][x] = panelRGB(255,255,255);
    pushBmp();
    x++;
    if (x >= W) { x = 0; y++; }
    if (y >= H) { y = 0; }
  }
  else if (mode == Row) {
    for (int xx=0; xx<W; xx++) bmp[y][xx] = panelRGB(255,255,255);
    pushBmp();
    y = (y + 1) % H;
  }
  else if (mode == Col) {
    for (int yy=0; yy<H; yy++) bmp[yy][x] = panelRGB(255,255,255);
    pushBmp();
    x = (x + 1) % W;
  }
}
