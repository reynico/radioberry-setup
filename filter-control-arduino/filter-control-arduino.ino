#include <Wire.h>

const uint8_t I2C_ADDRESS = 0x21;
const uint8_t COMMAND_ON_OFF = 0x01;
const uint8_t COMMAND_SPEED = 0x02;

struct FilterMap {
  int command;
  uint8_t bpfValue;
};

const FilterMap FILTER_TABLE[] PROGMEM = {
  {1608, 1},  // 160M
  {1604, 2},  // 80M
  {1602, 3},  // 60M
  {802, 4},   // 40M
  {401, 5},   // 30M
  {101, 6},   // 20M
  {164, 6},   // 17M
  {264, 7},   // 15M
  {4, 7}      // 12M/10M
};

const uint8_t FILTER_TABLE_SIZE = sizeof(FILTER_TABLE) / sizeof(FilterMap);

// BPF Vertex (TC4028BP)
const uint8_t BPF_PINS[] = {9, 10, 11}; // A, B, C
const uint8_t BPF_PIN_COUNT = 3;

// Control pins
const uint8_t PTT_PIN = 8;
const uint8_t TX_PIN = 7;
const uint8_t PA_PIN = 6;

volatile uint8_t currentCW = 0;
volatile bool genericMode = false;
volatile bool transmit = false;
int currentBand = 0;
uint8_t lastPttState = LOW;
unsigned long lastPttCheck = 0;
const unsigned long PTT_DEBOUNCE = 5; // 5ms debounce

void setup() {
  Serial.begin(115200);
  
  for (uint8_t i = 0; i < BPF_PIN_COUNT; i++) {
    pinMode(BPF_PINS[i], OUTPUT);
  }
  setBPFVertex(0);
  
  pinMode(PTT_PIN, INPUT);
  pinMode(TX_PIN, OUTPUT);
  pinMode(PA_PIN, OUTPUT);
  digitalWrite(TX_PIN, LOW);
  digitalWrite(PA_PIN, LOW);
  
  Wire.begin(I2C_ADDRESS);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);
}

inline void setBPFVertex(uint8_t filterNumber) {
  for (uint8_t i = 0; i < BPF_PIN_COUNT; i++) {
    digitalWrite(BPF_PINS[i], (filterNumber >> i) & 1);
  }
}

uint8_t getFilterValue(int command) {
  for (uint8_t i = 0; i < FILTER_TABLE_SIZE; i++) {
    if (pgm_read_word(&FILTER_TABLE[i].command) == command) {
      return pgm_read_byte(&FILTER_TABLE[i].bpfValue);
    }
  }
  return 0; // OFF by default
}

void requestEvent() {
  Wire.write(0);
}

void loop() {
  unsigned long currentTime = millis();
  if (currentTime - lastPttCheck >= PTT_DEBOUNCE) {
    uint8_t pttState = digitalRead(PTT_PIN);
    
    if (pttState != lastPttState && currentCW == 1) {
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
  uint8_t pttTrig = digitalRead(PTT_PIN);
  
  if (byte1 == 2 && byte2 == 2 && byte3 == 3) {
    genericMode = true;
    return;
  }
  
  if (!genericMode) {
    command = (byte2 * 100) + byte3;
  } else {
    command = (byte2 * 100) + byte3;
  }
  
  if (byte1 == 3) {
    currentCW = byte3;
    if (pttTrig == HIGH) {
      processPTT(byte2 > 0 ? byte2 : byte3);
    } else {
      processPTT(0);
    }
  } else {
    currentCW = 0;
    processPTT(0);
  }
  
  if (byte1 != 4 && command != 0 && command != currentBand) {
    processCommand(command);
  }
}

inline void processPTT(uint8_t state) {
  transmit = (state > 0);
  digitalWrite(TX_PIN, transmit);
  digitalWrite(PA_PIN, transmit);
}

void processCommand(int command) {
  currentBand = command;
  uint8_t filterValue = getFilterValue(command);
  
  setBPFVertex(filterValue);
  
  Serial.print("Band: ");
  Serial.print(command);
  Serial.print(" -> Filter: ");
  Serial.println(filterValue);
}