// D4=CS, D5=WR, D6=DATA

#define PIN_CS   4
#define PIN_WR   5
#define PIN_DATA 6

#define CMD_SYS_EN    0x01
#define CMD_LCD_ON    0x03
#define CMD_BIAS_3COM 0x25  // 1/3 bias, 3 COM
#define CMD_RC_256K   0x18

uint8_t ram[32];

void ht_sendBits(uint8_t data, uint8_t bits) {
  for (int i = bits - 1; i >= 0; i--) {
    digitalWrite(PIN_WR, LOW);
    digitalWrite(PIN_DATA, (data >> i) & 1);
    digitalWrite(PIN_WR, HIGH);
  }
}

void ht_cmd(uint8_t cmd) {
  digitalWrite(PIN_CS, LOW);
  ht_sendBits(0b100, 3);
  ht_sendBits(cmd, 8);
  ht_sendBits(0, 1);
  digitalWrite(PIN_CS, HIGH);
}

void ht_write(uint8_t addr, uint8_t data) {
  digitalWrite(PIN_CS, LOW);
  ht_sendBits(0b101, 3);
  ht_sendBits(addr, 6);
  ht_sendBits(data, 4);
  digitalWrite(PIN_CS, HIGH);
  ram[addr] = data;
}

void ht_writeAll() {
  digitalWrite(PIN_CS, LOW);
  ht_sendBits(0b101, 3);
  ht_sendBits(0, 6);
  for (int i = 0; i < 32; i++) {
    ht_sendBits(ram[i], 4);
  }
  digitalWrite(PIN_CS, HIGH);
}

void ht_init() {
  digitalWrite(PIN_CS, HIGH);
  digitalWrite(PIN_WR, HIGH);
  digitalWrite(PIN_DATA, HIGH);
  delay(100);
  ht_cmd(CMD_SYS_EN);
  ht_cmd(CMD_RC_256K);
  ht_cmd(CMD_BIAS_3COM);
  ht_cmd(CMD_LCD_ON);
}

void allOn() {
  for (int i = 0; i < 32; i++) ram[i] = 0x0F;
  ht_writeAll();
  Serial.println("ALL ON");
}

void allOff() {
  for (int i = 0; i < 32; i++) ram[i] = 0x00;
  ht_writeAll();
  Serial.println("ALL OFF");
}

void setSegment(uint8_t addr, uint8_t bit, bool on) {
  if (addr > 31 || bit > 3) return;
  if (on) ram[addr] |= (1 << bit);
  else ram[addr] &= ~(1 << bit);
  ht_write(addr, ram[addr]);
  Serial.print("ADDR ");
  Serial.print(addr);
  Serial.print(" BIT ");
  Serial.print(bit);
  Serial.println(on ? " ON" : " OFF");
}

void printHelp() {
  Serial.println("\n--- HT1621B Test ---");
  Serial.println("a = all ON");
  Serial.println("c = all OFF (clear)");
  Serial.println("s AA B = set addr(hex) bit(0-3) ON");
  Serial.println("r AA B = reset addr(hex) bit(0-3) OFF");
  Serial.println("t AA = toggle all bits at addr");
  Serial.println("b = blink test (all on/off)");
  Serial.println("w = walk (auto, 300ms)");
  Serial.println("m = manual walk (ENTER for next)");
  Serial.println("? = help\n");
}

void blinkTest() {
  Serial.println("BLINK TEST (5x)");
  for (int i = 0; i < 5; i++) {
    allOn();
    delay(500);
    allOff();
    delay(500);
  }
}

void walkTest() {
  Serial.println("WALK TEST - press any key to stop");
  allOff();
  for (uint8_t addr = 0; addr < 32 && !Serial.available(); addr++) {
    for (uint8_t bit = 1; bit <= 3 && !Serial.available(); bit++) {
      setSegment(addr, bit, true);
      delay(300);
      setSegment(addr, bit, false);
    }
  }
  while (Serial.available()) Serial.read();
}

void manualWalk() {
  Serial.println("MANUAL WALK - press ENTER for next, 'q' to quit");
  Serial.println("(bits 1,2,3 = COM2,COM1,COM0)");
  allOff();
  for (uint8_t addr = 0; addr < 32; addr++) {
    for (uint8_t bit = 1; bit <= 3; bit++) {
      setSegment(addr, bit, true);
      Serial.print("--> ADDR ");
      Serial.print(addr);
      Serial.print(" BIT ");
      Serial.print(bit);
      Serial.println(" : ");
      while (!Serial.available()) {}
      char c = Serial.read();
      while (Serial.available()) Serial.read();
      if (c == 'q' || c == 'Q') {
        Serial.println("QUIT");
        allOff();
        return;
      }
      setSegment(addr, bit, false);
    }
  }
  Serial.println("DONE");
}

uint8_t parseHex(char* str) {
  return strtol(str, NULL, 16);
}

void processCommand(char* buf) {
  char cmd = buf[0];
  switch (cmd) {
    case 'a': allOn(); break;
    case 'c': allOff(); break;
    case 'b': blinkTest(); break;
    case 'w': walkTest(); break;
    case 'm': manualWalk(); break;
    case '?': printHelp(); break;
    case 's':
    case 'r': {
      char* p = buf + 1;
      while (*p == ' ') p++;
      uint8_t addr = parseHex(p);
      while (*p && *p != ' ') p++;
      while (*p == ' ') p++;
      uint8_t bit = atoi(p);
      setSegment(addr, bit, cmd == 's');
      break;
    }
    case 't': {
      char* p = buf + 1;
      while (*p == ' ') p++;
      uint8_t addr = parseHex(p);
      if (addr < 32) {
        ram[addr] ^= 0x0F;
        ht_write(addr, ram[addr]);
        Serial.print("TOGGLE ADDR ");
        Serial.print(addr);
        Serial.print(" = 0x");
        Serial.println(ram[addr], HEX);
      }
      break;
    }
  }
}

void setup() {
  pinMode(PIN_CS, OUTPUT);
  pinMode(PIN_WR, OUTPUT);
  pinMode(PIN_DATA, OUTPUT);

  Serial.begin(115200);
  while (!Serial) delay(10);

  ht_init();
  printHelp();

  Serial.println("Init done. Testing all ON...");
  allOn();
  delay(1000);
  allOff();
  Serial.println("Ready.");
}

void loop() {
  static char buf[32];
  static uint8_t idx = 0;

  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (idx > 0) {
        buf[idx] = 0;
        processCommand(buf);
        idx = 0;
      }
    } else if (idx < sizeof(buf) - 1) {
      buf[idx++] = c;
    }
  }
}
