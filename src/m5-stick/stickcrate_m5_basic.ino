#include <M5Unified.h>
#include <NimBLEDevice.h>
#include <Preferences.h>

#include <vector>

struct BLEItem {
  String name;
  String address;
  int rssi;
};

enum Screen {
  SCREEN_CLICKER = 0,
  SCREEN_BLE = 1,
  SCREEN_IMU = 2,
};

Preferences prefs;
Screen currentScreen = SCREEN_CLICKER;
bool needFullRedraw = true;

// -----------------------------
// Clicker (persistente)
// -----------------------------
uint32_t clickCount = 0;

// -----------------------------
// BLE scanner + emparejado simple (persistente)
// -----------------------------
NimBLEScan* bleScan = nullptr;
std::vector<BLEItem> scannedDevices;
std::vector<String> pairedDevices;
int selectedBleIndex = 0;
String bleStatus = "Listo";

// -----------------------------
// IMU
// -----------------------------
float ax = 0, ay = 0, az = 0;
float gx = 0, gy = 0, gz = 0;
unsigned long lastImuDraw = 0;

// -----------------------------
// Botones (short/long press)
// -----------------------------
bool aPressed = false;
bool bPressed = false;
unsigned long aDownMs = 0;
unsigned long bDownMs = 0;
const unsigned long LONG_PRESS_MS = 700;

struct ButtonEvents {
  bool aShort = false;
  bool aLong = false;
  bool bShort = false;
  bool bLong = false;
};

// -----------------------------
// Utilidades
// -----------------------------
bool isPaired(const String& addr) {
  for (const auto& p : pairedDevices) {
    if (p == addr) return true;
  }
  return false;
}

void savePairedDevices() {
  String joined;
  for (size_t i = 0; i < pairedDevices.size(); ++i) {
    joined += pairedDevices[i];
    if (i + 1 < pairedDevices.size()) joined += ";";
  }
  prefs.putString("paired", joined);
}

void loadPairedDevices() {
  pairedDevices.clear();
  String raw = prefs.getString("paired", "");
  if (raw.length() == 0) return;

  int start = 0;
  while (start < raw.length()) {
    int sep = raw.indexOf(';', start);
    if (sep < 0) sep = raw.length();
    String token = raw.substring(start, sep);
    if (token.length() > 0) pairedDevices.push_back(token);
    start = sep + 1;
  }
}

void togglePairSelectedDevice() {
  if (scannedDevices.empty()) {
    bleStatus = "Sin dispositivos";
    return;
  }

  if (selectedBleIndex < 0 || selectedBleIndex >= (int)scannedDevices.size()) {
    selectedBleIndex = 0;
  }

  String addr = scannedDevices[selectedBleIndex].address;

  for (size_t i = 0; i < pairedDevices.size(); ++i) {
    if (pairedDevices[i] == addr) {
      pairedDevices.erase(pairedDevices.begin() + i);
      savePairedDevices();
      bleStatus = "Desemparejado";
      return;
    }
  }

  pairedDevices.push_back(addr);
  savePairedDevices();
  bleStatus = "Emparejado";
}

void scanBleDevices() {
  bleStatus = "Escaneando...";
  scannedDevices.clear();
  selectedBleIndex = 0;

  if (!bleScan) {
    bleStatus = "BLE no disponible";
    return;
  }

  M5.Display.fillRect(0, 20, M5.Display.width(), M5.Display.height() - 20, TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setCursor(8, 40);
  M5.Display.println("Escaneando BLE (3s)...");

  NimBLEScanResults results = bleScan->start(3, false);
  const int count = results.getCount();

  for (int i = 0; i < count; ++i) {
    const NimBLEAdvertisedDevice* dev = results.getDevice(i);
    if (!dev) continue;

    BLEItem item;
    item.name = dev->haveName() ? String(dev->getName().c_str()) : String("(sin nombre)");
    item.address = String(dev->getAddress().toString().c_str());
    item.rssi = dev->getRSSI();

    scannedDevices.push_back(item);
  }

  bleScan->clearResults();

  if (scannedDevices.empty()) {
    bleStatus = "No encontrados";
  } else {
    bleStatus = String(scannedDevices.size()) + " encontrados";
  }
}

ButtonEvents readButtonEvents() {
  ButtonEvents e;

  const bool aNow = M5.BtnA.isPressed();
  const bool bNow = M5.BtnB.isPressed();

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

  return e;
}

void drawHeader(const char* title, uint16_t bgColor) {
  M5.Display.fillRect(0, 0, M5.Display.width(), 20, bgColor);
  M5.Display.setTextColor(TFT_WHITE, bgColor);
  M5.Display.setCursor(6, 4);
  M5.Display.print(title);
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
  M5.Display.print("A: pantalla / A long: reset");
  M5.Display.setCursor(8, 144);
  M5.Display.print("B: +1 (persistente)");
}

void drawBleScreen() {
  M5.Display.fillScreen(TFT_DARKCYAN);
  drawHeader("BLE MANAGER", TFT_PURPLE);

  M5.Display.setTextColor(TFT_WHITE, TFT_DARKCYAN);
  M5.Display.setTextSize(1);
  M5.Display.setCursor(8, 24);
  M5.Display.printf("Estado: %s", bleStatus.c_str());

  int y = 40;
  for (int i = 0; i < (int)scannedDevices.size() && i < 4; ++i) {
    const auto& d = scannedDevices[i];
    bool selected = (i == selectedBleIndex);

    uint16_t lineBg = selected ? TFT_ORANGE : TFT_DARKGREY;
    uint16_t lineFg = selected ? TFT_BLACK : TFT_WHITE;
    M5.Display.fillRect(6, y - 2, M5.Display.width() - 12, 20, lineBg);
    M5.Display.setTextColor(lineFg, lineBg);
    M5.Display.setCursor(10, y + 2);
    M5.Display.printf("%d) %s %ddBm", i + 1, d.name.substring(0, 10).c_str(), d.rssi);
    y += 22;
  }

  if (scannedDevices.empty()) {
    M5.Display.setTextColor(TFT_WHITE, TFT_DARKCYAN);
    M5.Display.setCursor(10, 52);
    M5.Display.print("Pulsa B para escanear");
  }

  M5.Display.setTextColor(TFT_GREENYELLOW, TFT_DARKCYAN);
  M5.Display.setCursor(8, 128);
  M5.Display.print("A: pantalla / A long: sig. disp");
  M5.Display.setCursor(8, 142);
  M5.Display.print("B: scan / B long: pair-unpair");
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

  // Bola de inclinación usando acelerómetro X/Y
  int cx = graphW / 2;
  int cy = graphY + graphH / 2;
  int px = cx + (int)(ax * 40.0f);
  int py = cy + (int)(ay * 40.0f);

  M5.Display.drawCircle(cx, cy, 42, TFT_DARKGREY);
  M5.Display.fillCircle(px, py, 8, TFT_SKYBLUE);

  // Barras giroscopio
  int baseY = graphY + graphH - 24;
  int gxw = (int)constrain(gx * 0.2f, -55, 55);
  int gyw = (int)constrain(gy * 0.2f, -55, 55);
  int gzw = (int)constrain(gz * 0.2f, -55, 55);

  M5.Display.drawFastVLine(cx, baseY - 18, 18, TFT_DARKGREY);

  M5.Display.fillRect(cx, baseY - 16, gxw, 4, TFT_RED);
  M5.Display.fillRect(cx, baseY - 10, gyw, 4, TFT_GREEN);
  M5.Display.fillRect(cx, baseY - 4, gzw, 4, TFT_BLUE);

  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setCursor(6, 24);
  M5.Display.printf("A(%.2f %.2f %.2f)", ax, ay, az);
  M5.Display.setCursor(6, 36);
  M5.Display.printf("G(%.1f %.1f %.1f)", gx, gy, gz);

  M5.Display.setTextColor(TFT_CYAN, TFT_BLACK);
  M5.Display.setCursor(6, 144);
  M5.Display.print("A: pantalla");
}

void redrawCurrentScreen() {
  switch (currentScreen) {
    case SCREEN_CLICKER:
      drawClickerScreen();
      break;
    case SCREEN_BLE:
      drawBleScreen();
      break;
    case SCREEN_IMU:
      drawImuScreen(true);
      break;
  }
}

void handleEvents(const ButtonEvents& e) {
  // Navegación global entre pantallas
  if (e.aShort) {
    currentScreen = (Screen)((currentScreen + 1) % 3);
    needFullRedraw = true;
    return;
  }

  // Acciones por pantalla
  switch (currentScreen) {
    case SCREEN_CLICKER:
      if (e.bShort) {
        clickCount++;
        prefs.putUInt("clicks", clickCount);
        needFullRedraw = true;
      }
      if (e.aLong) {
        clickCount = 0;
        prefs.putUInt("clicks", clickCount);
        needFullRedraw = true;
      }
      break;

    case SCREEN_BLE:
      if (e.bShort) {
        scanBleDevices();
        needFullRedraw = true;
      }
      if (e.aLong && !scannedDevices.empty()) {
        selectedBleIndex = (selectedBleIndex + 1) % scannedDevices.size();
        needFullRedraw = true;
      }
      if (e.bLong) {
        togglePairSelectedDevice();
        needFullRedraw = true;
      }
      break;

    case SCREEN_IMU:
      if (e.aLong) {
        // Reservado para futuras acciones
      }
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
  loadPairedDevices();

  NimBLEDevice::init("StickCrate");
  bleScan = NimBLEDevice::getScan();
  if (bleScan) {
    bleScan->setActiveScan(true);
    bleScan->setInterval(45);
    bleScan->setWindow(15);
  }

  needFullRedraw = true;
}

void loop() {
  M5.update();

  // Lectura de IMU continua para la pantalla 3
  M5.Imu.getAccel(&ax, &ay, &az);
  M5.Imu.getGyro(&gx, &gy, &gz);

  ButtonEvents e = readButtonEvents();
  handleEvents(e);

  if (needFullRedraw) {
    redrawCurrentScreen();
    needFullRedraw = false;
  }

  // Redibujo más frecuente solo para IMU
  if (currentScreen == SCREEN_IMU && millis() - lastImuDraw > 120) {
    drawImuScreen(false);
    lastImuDraw = millis();
  }

  delay(10);
}
