// LCD Controller for Yaesu FT-747GX / FT-80C
// Uses HT1621B to drive the original FTD8627PZ display
// Receives CAT commands via Serial

#include "segment_map.h"

#define PIN_CS   4
#define PIN_WR   5
#define PIN_DATA 6
#define PIN_SMETER 3

#define CMD_SYS_EN    0x01
#define CMD_LCD_ON    0x03
#define CMD_LCD_OFF   0x02
#define CMD_BIAS_3COM 0x25
#define CMD_RC_256K   0x18

uint8_t ram[32];
String cmdBuffer = "";
bool catActive = false;

// HT1621B low-level functions
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

// Segment control functions
void setSegment(uint8_t seg, bool on) {
  uint8_t addr = SEG_ADDR(seg);
  uint8_t bit = SEG_BIT(seg);
  if (addr > 31 || bit > 3) return;
  if (on) ram[addr] |= (1 << bit);
  else ram[addr] &= ~(1 << bit);
}

void clearAll() {
  for (int i = 0; i < 32; i++) ram[i] = 0x00;
}

void updateDisplay() {
  ht_writeAll();
}

// Display a single digit (0-9, 10=blank, 11=dash)
void setDigit(uint8_t digitIndex, uint8_t value) {
  if (digitIndex > 6 || value > 11) return;
  uint8_t pattern = CHAR_TABLE[value];
  for (int seg = 0; seg < 7; seg++) {
    setSegment(DIGITS[digitIndex][seg], (pattern >> seg) & 1);
  }
}

// Display a raw pattern on a digit
void setDigitPattern(uint8_t digitIndex, uint8_t pattern) {
  if (digitIndex > 6) return;
  for (int seg = 0; seg < 7; seg++) {
    setSegment(DIGITS[digitIndex][seg], (pattern >> seg) & 1);
  }
}

// Display callsign LU3Arn with all indicators on
void displayCallsign() {
  clearAll();
  setDigitPattern(0, CHAR_BLANK);
  setDigitPattern(1, CHAR_L);
  setDigitPattern(2, CHAR_u);
  setDigitPattern(3, CHAR_3);
  setDigitPattern(4, CHAR_A);
  setDigitPattern(5, CHAR_r);
  setDigitPattern(6, CHAR_n);

  setIndicator(IND_BAND, true);
  setIndicator(IND_SCAN, true);
  setIndicator(IND_SPLIT, true);
  setIndicator(IND_VFO_A, true);
  setIndicator(IND_VFO_B, true);
  setIndicator(IND_BUSY, true);
  setIndicator(IND_CLAR, true);
  setIndicator(IND_LOCK, true);
  setIndicator(IND_FAST, true);
  setIndicator(IND_MR, true);
  setIndicator(IND_LSB, true);
  setIndicator(IND_USB, true);
  setIndicator(IND_CW, true);
  setIndicator(IND_AM, true);
  setIndicator(IND_FM, true);
  setIndicator(IND_NAR, true);
  setIndicator(IND_PRI, true);
  setIndicator(IND_GEN, true);
  setIndicator(IND_CAT, true);
}

// Display frequency in Hz (e.g., 14250000 = 14.250.0)
// Format: XXX.XXX.X (digits 1-7)
void displayFrequency(uint32_t freqHz) {
  uint32_t freqKhz = freqHz / 100;
  uint8_t digits[7];

  for (int i = 6; i >= 0; i--) {
    digits[i] = freqKhz % 10;
    freqKhz /= 10;
  }

  bool leadingZero = true;
  for (int i = 0; i < 7; i++) {
    if (digits[i] != 0) leadingZero = false;
    if (leadingZero && i < 6) {
      setDigit(i, 10);
    } else {
      setDigit(i, digits[i]);
    }
  }
}

// Indicator control
void setIndicator(uint8_t indicator, bool on) {
  setSegment(indicator, on);
}

void clearModeIndicators() {
  setIndicator(IND_LSB, false);
  setIndicator(IND_USB, false);
  setIndicator(IND_CW, false);
  setIndicator(IND_AM, false);
  setIndicator(IND_FM, false);
  setIndicator(IND_NAR, false);
}

void clearAllIndicators() {
  clearModeIndicators();
  setIndicator(IND_BAND, false);
  setIndicator(IND_SCAN, false);
  setIndicator(IND_SPLIT, false);
  setIndicator(IND_VFO_A, false);
  setIndicator(IND_VFO_B, false);
  setIndicator(IND_BUSY, false);
  setIndicator(IND_CLAR, false);
  setIndicator(IND_LOCK, false);
  setIndicator(IND_FAST, false);
  setIndicator(IND_MR, false);
  setIndicator(IND_PRI, false);
  setIndicator(IND_GEN, false);
  setIndicator(IND_CAT, false);
}

void displayMode(uint8_t mode) {
  clearModeIndicators();
  switch (mode) {
    case 1: setIndicator(IND_LSB, true); break;
    case 2: setIndicator(IND_USB, true); break;
    case 3: setIndicator(IND_CW, true); break;
    case 4: setIndicator(IND_FM, true); break;
    case 5: setIndicator(IND_AM, true); break;
    case 7: setIndicator(IND_CW, true); break;
  }
}

// VFO control
void setVFO(uint8_t vfo) {
  setIndicator(IND_VFO_A, vfo == 0);
  setIndicator(IND_VFO_B, vfo == 1);
}

// S-Meter control (PWM output)
// Input: 0-255 from bridge (already scaled from dB)
void setSMeter(uint16_t value) {
  if (value > 255) value = 255;
  analogWrite(PIN_SMETER, value);
}

void initCatMode() {
  if (!catActive) {
    catActive = true;
    clearAllIndicators();
    setIndicator(IND_CAT, true);
  }
}

// CAT command processing
void processCommand(String cmd) {
  initCatMode();

  if (cmd.startsWith("FA")) {
    String freq = cmd.substring(2);
    freq.trim();
    if (freq.length() > 0) {
      uint32_t f = freq.toInt();
      displayFrequency(f);
      updateDisplay();
    }
  }
  else if (cmd.startsWith("MD")) {
    String mode = cmd.substring(2);
    mode.trim();
    if (mode.length() > 0) {
      displayMode(mode.toInt());
      updateDisplay();
    }
  }
  else if (cmd.startsWith("FR")) {
    String vfo = cmd.substring(2);
    vfo.trim();
    if (vfo.length() > 0) {
      setVFO(vfo.toInt());
      updateDisplay();
    }
  }
  else if (cmd.startsWith("TX")) {
    String val = cmd.substring(2);
    val.trim();
    if (val.length() > 0) {
      setIndicator(IND_BUSY, val.toInt() == 0);
    } else {
      setIndicator(IND_BUSY, false);
    }
    updateDisplay();
  }
  else if (cmd.startsWith("SM")) {
    String val = cmd.substring(2);
    val.trim();
    if (val.length() > 0) {
      setSMeter(val.toInt());
    }
  }
}

void setup() {
  pinMode(PIN_CS, OUTPUT);
  pinMode(PIN_WR, OUTPUT);
  pinMode(PIN_DATA, OUTPUT);
  pinMode(PIN_SMETER, OUTPUT);
  analogWrite(PIN_SMETER, 0);

  Serial.begin(9600);

  ht_init();

  displayCallsign();
  updateDisplay();
}

void loop() {
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == ';' || c == '\n') {
      if (cmdBuffer.length() > 0) {
        processCommand(cmdBuffer);
        cmdBuffer = "";
      }
    } else if (c >= 32) {
      cmdBuffer += c;
    }
  }
}
