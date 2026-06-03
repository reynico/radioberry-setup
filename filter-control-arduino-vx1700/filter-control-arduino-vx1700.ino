#include <Wire.h>

const uint8_t I2C_ADDRESS = 0x21;

struct FilterMap {
  int command;
  uint8_t bpfValue;
};

const FilterMap FILTER_TABLE[] PROGMEM = {
  {3208, 4},  // 2200M (160m filter, relay 4)
  {1608, 4},  // 160M (relay 4)
  {1604, 3},  // 80M (relay 3)
  {1602, 7},  // 60M (relay 7)
  {802, 2},   // 40M (relay 2)
  {401, 5},   // 30M (relay 5)
  {101, 1},   // 20M (relay 1)
  {164, 1},   // 17M (relay 1)
  {264, 6},   // 15M (relay 6)
  {232, 6}    // 12M/10M (relay 6)
};

const uint8_t FILTER_TABLE_SIZE = sizeof(FILTER_TABLE) / sizeof(FilterMap);

// D2-D8: one-hot filter outputs
const uint8_t BPF_PINS[] = {2, 3, 4, 5, 6, 7, 8};
const uint8_t BPF_PIN_COUNT = 7;

const uint8_t PTT_OUT_PIN = 9;
const uint8_t PTT_IN_PIN = 10;

volatile bool genericMode = false;
int currentBand = 0;

void setup() {
  Serial.begin(115200);

  for (uint8_t i = 0; i < BPF_PIN_COUNT; i++) {
    pinMode(BPF_PINS[i], OUTPUT);
    digitalWrite(BPF_PINS[i], LOW);
  }

  pinMode(PTT_IN_PIN, INPUT);
  pinMode(PTT_OUT_PIN, OUTPUT);
  digitalWrite(PTT_OUT_PIN, LOW);

  Wire.begin(I2C_ADDRESS);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);
}

inline void setFilter(uint8_t filterNumber) {
  for (uint8_t i = 0; i < BPF_PIN_COUNT; i++) {
    digitalWrite(BPF_PINS[i], (i + 1) == filterNumber ? HIGH : LOW);
  }
}

uint8_t getFilterValue(int command) {
  for (uint8_t i = 0; i < FILTER_TABLE_SIZE; i++) {
    if (pgm_read_word(&FILTER_TABLE[i].command) == command) {
      return pgm_read_byte(&FILTER_TABLE[i].bpfValue);
    }
  }
  return 0;
}

void requestEvent() {
  Wire.write(0);
}

void loop() {
  digitalWrite(PTT_OUT_PIN, digitalRead(PTT_IN_PIN));
  delay(1);
}

void receiveEvent(int bytes) {
  if (bytes < 3) return;

  uint8_t byte1 = Wire.read();
  uint8_t byte2 = Wire.read();
  uint8_t byte3 = Wire.read();

  while (Wire.available()) Wire.read();

  if (byte1 == 2 && byte2 == 2 && byte3 == 3) {
    genericMode = true;
    return;
  }

  int command = (byte2 * 100) + byte3;

  if (byte1 != 4 && command != 0 && command != currentBand) {
    processCommand(command);
  }
}

void processCommand(int command) {
  currentBand = command;
  uint8_t filterValue = getFilterValue(command);

  setFilter(filterValue);

  Serial.print("Band: ");
  Serial.print(command);
  Serial.print(" -> Filter: ");
  Serial.println(filterValue);
}