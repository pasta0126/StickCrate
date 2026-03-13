#include <Arduino.h>
#include <M5Unified.h>
#include <Preferences.h>

enum View {
  VIEW_MENU = 0,
  VIEW_CLICKER = 1,
  VIEW_IMU = 2,
};

Preferences prefs;
View currentView = VIEW_MENU;
bool needFullRedraw = true;
int menuIndex = 0;
const int menuCount = 2;

// -----------------------------
// Clicker (persistente)
// -----------------------------
uint32_t clickCount = 0;

// -----------------------------
// IMU
// -----------------------------
float ax = 0, ay = 0, az = 0;
float gx = 0, gy = 0, gz = 0;
unsigned long lastImuDraw = 0;
bool imuFreeze = false;
int imuMode = 0;

// -----------------------------
// Botones (short/long press)
// -----------------------------
bool aPressed = false;
bool bPressed = false;
bool pPressed = false;
unsigned long aDownMs = 0;
unsigned long bDownMs = 0;
unsigned long pDownMs = 0;
const unsigned long LONG_PRESS_MS = 700;

struct ButtonEvents {
  bool aShort = false;
  bool aLong = false;
  bool bShort = false;
  bool bLong = false;
  bool pShort = false;
  bool pLong = false;
};

// -----------------------------
// Utilidades
// -----------------------------
void nextOption() {
  if (currentView == VIEW_MENU) {
    menuIndex = (menuIndex + 1) % menuCount;
    needFullRedraw = true;
    return;
  }

  if (currentView == VIEW_CLICKER) {
    clickCount++;
    prefs.putUInt("clicks", clickCount);
    needFullRedraw = true;
    return;
  }

  if (currentView == VIEW_IMU) {
    imuMode = (imuMode + 1) % 2;
    needFullRedraw = true;
  }
}

void prevMenuOption() {
  if (currentView == VIEW_MENU) {
    menuIndex = (menuIndex - 1 + menuCount) % menuCount;
    needFullRedraw = true;
  }
}

void backOrCancel() {
  if (currentView == VIEW_MENU) {
    prevMenuOption();
    return;
  }

  currentView = VIEW_MENU;
  needFullRedraw = true;
}

ButtonEvents readButtonEvents() {
  ButtonEvents e;

  const bool aNow = M5.BtnA.isPressed();
  const bool bNow = M5.BtnB.isPressed();
  const bool pNow = M5.BtnPWR.isPressed();

  if (aNow && !aPressed) {
    aPressed = true;
    aDownMs = millis();
  }
  if (!aNow && aPressed) {
    aPressed = false;
    if (millis() - aDownMs >= LONG_PRESS_MS) e.aLong = true;
    else e.aShort = true;
  }

  if (bNow && !bPressed) {
    bPressed = true;
    bDownMs = millis();
  }
  if (!bNow && bPressed) {
    bPressed = false;
    if (millis() - bDownMs >= LONG_PRESS_MS) e.bLong = true;
    else e.bShort = true;
  }

  if (pNow && !pPressed) {
    pPressed = true;
    pDownMs = millis();
  }
  if (!pNow && pPressed) {
    pPressed = false;
    if (millis() - pDownMs >= LONG_PRESS_MS) e.pLong = true;
    else e.pShort = true;
  }

  return e;
}

void drawHeader(const char* title, uint16_t bgColor) {
  M5.Display.fillRect(0, 0, M5.Display.width(), 20, bgColor);
  M5.Display.setTextColor(TFT_WHITE, bgColor);
  M5.Display.setCursor(6, 4);
  M5.Display.print(title);
}

void drawMenuScreen() {
  M5.Display.fillScreen(TFT_DARKGREY);
  drawHeader("STICKCRATE MENU", TFT_BLUE);

  M5.Display.setTextSize(2);
  for (int i = 0; i < menuCount; ++i) {
    int y = 40 + (i * 38);
    bool selected = (i == menuIndex);
    uint16_t bg = selected ? TFT_YELLOW : TFT_NAVY;
    uint16_t fg = selected ? TFT_BLACK : TFT_WHITE;
    const char* label = (i == 0) ? "Clicker" : "IMU Visual";

    M5.Display.fillRoundRect(10, y, M5.Display.width() - 20, 30, 6, bg);
    M5.Display.setTextColor(fg, bg);
    M5.Display.setCursor(18, y + 8);
    M5.Display.print(label);
  }

  M5.Display.setTextSize(1);
  M5.Display.setTextColor(TFT_GREENYELLOW, TFT_DARKGREY);
  M5.Display.setCursor(8, 130);
  M5.Display.print("B: adelante | PWR: atras");
  M5.Display.setCursor(8, 142);
  M5.Display.print("A: seleccionar | B long: cancelar");
}

void drawClickerScreen() {
  M5.Display.fillScreen(TFT_NAVY);
  drawHeader("CLICKER", TFT_DARKGREEN);

  M5.Display.setTextColor(TFT_YELLOW, TFT_NAVY);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(12, 34);
  M5.Display.print("Clicks:");

  M5.Display.setTextColor(TFT_WHITE, TFT_NAVY);
  M5.Display.setTextSize(4);
  M5.Display.setCursor(14, 62);
  M5.Display.printf("%lu", clickCount);

  M5.Display.setTextSize(1);
  M5.Display.setTextColor(TFT_CYAN, TFT_NAVY);
  M5.Display.setCursor(8, 130);
  M5.Display.print("B:+1 | A corto:+10");
  M5.Display.setCursor(8, 144);
  M5.Display.print("PWR/B long: menu | A long: reset");
}

void drawImuScreen(bool fullBackground) {
  if (fullBackground) {
    M5.Display.fillScreen(TFT_BLACK);
    drawHeader("IMU VISUAL", TFT_MAROON);
  }

  // Área de gráfico
  const int graphX = 0, graphY = 20;
  const int graphW = M5.Display.width();
  const int graphH = M5.Display.height() - 20;
  M5.Display.fillRect(graphX, graphY, graphW, graphH, TFT_BLACK);

  // Modo 0: bola de inclinación + barras gyro
  int cx = graphW / 2;
  int cy = graphY + graphH / 2;
  if (imuMode == 0) {
    int px = cx + (int)(ax * 40.0f);
    int py = cy + (int)(ay * 40.0f);

    M5.Display.drawCircle(cx, cy, 42, TFT_DARKGREY);
    M5.Display.fillCircle(px, py, 8, TFT_SKYBLUE);

    int baseY = graphY + graphH - 24;
    int gxw = (int)constrain(gx * 0.2f, -55, 55);
    int gyw = (int)constrain(gy * 0.2f, -55, 55);
    int gzw = (int)constrain(gz * 0.2f, -55, 55);

    M5.Display.drawFastVLine(cx, baseY - 18, 18, TFT_DARKGREY);
    M5.Display.fillRect(cx, baseY - 16, gxw, 4, TFT_RED);
    M5.Display.fillRect(cx, baseY - 10, gyw, 4, TFT_GREEN);
    M5.Display.fillRect(cx, baseY - 4, gzw, 4, TFT_BLUE);
  } else {
    // Modo 1: barras de aceleración
    int baseX = 16;
    int topY = 56;
    int barW = 22;
    int maxH = 70;

    int hax = (int)constrain(abs(ax) * 60.0f, 0, maxH);
    int hay = (int)constrain(abs(ay) * 60.0f, 0, maxH);
    int haz = (int)constrain(abs(az) * 60.0f, 0, maxH);

    M5.Display.drawRect(baseX - 2, topY - 2, barW + 4, maxH + 4, TFT_RED);
    M5.Display.drawRect(baseX + 36 - 2, topY - 2, barW + 4, maxH + 4, TFT_GREEN);
    M5.Display.drawRect(baseX + 72 - 2, topY - 2, barW + 4, maxH + 4, TFT_BLUE);

    M5.Display.fillRect(baseX, topY + (maxH - hax), barW, hax, TFT_RED);
    M5.Display.fillRect(baseX + 36, topY + (maxH - hay), barW, hay, TFT_GREEN);
    M5.Display.fillRect(baseX + 72, topY + (maxH - haz), barW, haz, TFT_BLUE);
  }

  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setCursor(6, 24);
  M5.Display.printf("A(%.2f %.2f %.2f)", ax, ay, az);
  M5.Display.setCursor(6, 36);
  M5.Display.printf("G(%.1f %.1f %.1f)", gx, gy, gz);
  M5.Display.setCursor(138, 24);
  M5.Display.printf("M%d", imuMode + 1);
  if (imuFreeze) {
    M5.Display.setCursor(138, 36);
    M5.Display.print("PAUSE");
  }

  M5.Display.setTextColor(TFT_CYAN, TFT_BLACK);
  M5.Display.setCursor(6, 144);
  M5.Display.print("B: modo | A: pausa | PWR: menu");
}

void redrawCurrentScreen() {
  switch (currentView) {
    case VIEW_MENU:
      drawMenuScreen();
      break;
    case VIEW_CLICKER:
      drawClickerScreen();
      break;
    case VIEW_IMU:
      drawImuScreen(true);
      break;
  }
}

void handleEvents(const ButtonEvents& e) {
  // PWR largo = apagar siempre
  if (e.pLong) {
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.setCursor(20, 66);
    M5.Display.print("Apagando...");
    delay(250);
    M5.Power.powerOff();
    return;
  }

  // Cancelar/atrás global
  if (e.pShort || e.bLong) {
    backOrCancel();
    return;
  }

  // Navegación general de listas/opciones (adelante)
  if (e.bShort) {
    nextOption();
    return;
  }

  // A corto = seleccionar / acción
  if (e.aShort) {
    if (currentView == VIEW_MENU) {
      currentView = (menuIndex == 0) ? VIEW_CLICKER : VIEW_IMU;
      needFullRedraw = true;
      return;
    }

    if (currentView == VIEW_CLICKER) {
      clickCount += 10;
      prefs.putUInt("clicks", clickCount);
      needFullRedraw = true;
      return;
    }

    if (currentView == VIEW_IMU) {
      imuFreeze = !imuFreeze;
      needFullRedraw = true;
      return;
    }

    return;
  }

  // Acciones por pantalla
  switch (currentView) {
    case VIEW_CLICKER:
      if (e.aLong) {
        clickCount = 0;
        prefs.putUInt("clicks", clickCount);
        needFullRedraw = true;
      }
      break;

    case VIEW_IMU:
      // Sin acción de A largo en IMU
      break;

    case VIEW_MENU:
      // Sin acción de A largo en menú
      break;
  }
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(1);
  M5.Display.setTextSize(1);

  prefs.begin("stickcrate", false);
  clickCount = prefs.getUInt("clicks", 0);

  needFullRedraw = true;
}

void loop() {
  M5.update();

  // Lectura de IMU continua para la pantalla 3
  if (!imuFreeze) {
    M5.Imu.getAccel(&ax, &ay, &az);
    M5.Imu.getGyro(&gx, &gy, &gz);
  }

  ButtonEvents e = readButtonEvents();
  handleEvents(e);

  if (needFullRedraw) {
    redrawCurrentScreen();
    needFullRedraw = false;
  }

  // Redibujo más frecuente solo para IMU
  if (currentView == VIEW_IMU && millis() - lastImuDraw > 120) {
    drawImuScreen(false);
    lastImuDraw = millis();
  }

  delay(10);
}
