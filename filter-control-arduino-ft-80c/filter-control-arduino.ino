#include <Wire.h>

const uint8_t I2C_ADDRESS = 0x21;
const uint8_t COMMAND_ON_OFF = 0x01;
const uint8_t COMMAND_SPEED = 0x02;

struct FilterMap {
  int command;
  uint8_t bpfValue;
};

const FilterMap FILTER_TABLE[] PROGMEM = {
  {3208, 1},  // 2200M
  {1608, 1},  // 160M
  {1604, 2},  // 80M
  {1602, 3},  // 60M
  {802, 3},   // 40M
  {401, 4},   // 30M
  {101, 4},   // 20M
  {164, 5},   // 17M
  {264, 5},   // 15M
  {232, 6}    // 12M/10M
};

const uint8_t FILTER_TABLE_SIZE = sizeof(FILTER_TABLE) / sizeof(FilterMap);

const uint8_t RELAY_PINS[] = {8, 7, 6, 5, 4, 3};
const uint8_t RELAY_COUNT = sizeof(RELAY_PINS);

// Control pins
const uint8_t PTT_PIN = 10;
const uint8_t TX_PIN = 13;
const uint8_t PA_PIN = 9;

volatile uint8_t currentCW = 0;
volatile bool genericMode = false;
volatile bool transmit = false;
int currentBand = 0;
uint8_t lastPttState = HIGH;
unsigned long lastPttCheck = 0;
const unsigned long PTT_DEBOUNCE = 5; // 5ms debounce

void setup() {
  Serial.begin(115200);
  
  for (uint8_t i = 0; i < RELAY_COUNT; i++) {
    pinMode(RELAY_PINS[i], OUTPUT);
    digitalWrite(RELAY_PINS[i], LOW);
  }
  
  pinMode(PTT_PIN, INPUT_PULLUP);
  pinMode(TX_PIN, OUTPUT);
  pinMode(PA_PIN, OUTPUT);
  digitalWrite(TX_PIN, LOW);
  digitalWrite(PA_PIN, LOW);
  
  Wire.begin(I2C_ADDRESS);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);
}

inline void setBPF(uint8_t filterNumber) {
  for (uint8_t i = 0; i < RELAY_COUNT; i++) {
    digitalWrite(RELAY_PINS[i], LOW);
  }

  if (filterNumber >= 1 && filterNumber <= RELAY_COUNT) {
    digitalWrite(RELAY_PINS[filterNumber - 1], HIGH);
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
  unsigned long currentTime = millis();
  if (currentTime - lastPttCheck >= PTT_DEBOUNCE) {
    uint8_t pttState = digitalRead(PTT_PIN);

    if (pttState != lastPttState) {
      processPTT(pttState);
      lastPttState = pttState;
    }
    lastPttCheck = currentTime;
  }

  delay(1);
}

void receiveEvent(int bytes) {
  if (bytes < 3) return;
  
  uint8_t byte1 = Wire.read();
  uint8_t byte2 = Wire.read();
  uint8_t byte3 = Wire.read();
  
  while (Wire.available()) {
    Wire.read();
  }
  
  int command = 0;

  if (byte1 == 2 && byte2 == 2 && byte3 == 3) {
    genericMode = true;
    return;
  }

  command = (byte2 * 100) + byte3;

  if (byte1 == 3) {
    currentCW = byte3;
  } else {
    currentCW = 0;
  }

  if (byte1 != 4 && command != 0 && command != currentBand) {
    processCommand(command);
  }
}

inline void processPTT(uint8_t state) {
  transmit = (state == LOW);
  digitalWrite(TX_PIN, state);
  digitalWrite(PA_PIN, transmit);
}

void processCommand(int command) {
  currentBand = command;
  uint8_t filterValue = getFilterValue(command);
  
  setBPF(filterValue);
  
  Serial.print("Band: ");
  Serial.print(command);
  Serial.print(" -> Filter: ");
  Serial.println(filterValue);
}
