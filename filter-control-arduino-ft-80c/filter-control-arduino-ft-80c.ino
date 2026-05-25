#include <Wire.h>

const uint8_t I2C_ADDRESS = 0x21;
const uint8_t PCA9555_OUTPUT_PORT_0 = 0x02;
const uint8_t PCA9555_OUTPUT_PORT_1 = 0x03;
const uint8_t PCA9555_CONFIGURATION_PORT_0 = 0x06;
const uint8_t PCA9555_CONFIGURATION_PORT_1 = 0x07;
const uint8_t PCA9555_REGISTER_COUNT = 8;
const uint8_t I2C_BUFFER_SIZE = 8;

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
const uint8_t RB_PTT_PIN = A6;
const int ANALOG_THRESHOLD_ON = 600;
const int ANALOG_THRESHOLD_OFF = 400;

volatile bool transmit = false;
volatile bool i2cReady = false;
volatile bool i2cPacketPending = false;
volatile uint8_t i2cPacketLength = 0;
volatile uint8_t i2cPacket[I2C_BUFFER_SIZE];
volatile uint8_t pca9555Registers[PCA9555_REGISTER_COUNT] = {0};
volatile uint8_t pca9555ReadRegister = 0;
bool rbPttState = false;
int currentBand = 0;
uint8_t lastPttState = HIGH;
unsigned long lastPttCheck = 0;
const unsigned long PTT_DEBOUNCE = 5; // 5ms debounce

void setup() {
  Wire.begin(I2C_ADDRESS);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);

  Serial.begin(115200);

  for (uint8_t i = 0; i < RELAY_COUNT; i++) {
    pinMode(RELAY_PINS[i], OUTPUT);
    digitalWrite(RELAY_PINS[i], LOW);
  }

  pinMode(PTT_PIN, INPUT_PULLUP);
  pinMode(TX_PIN, OUTPUT);
  pinMode(PA_PIN, OUTPUT);
  digitalWrite(TX_PIN, HIGH);
  digitalWrite(PA_PIN, LOW);
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
  Wire.write(pca9555Registers[pca9555ReadRegister & (PCA9555_REGISTER_COUNT - 1)]);
  pca9555ReadRegister = (pca9555ReadRegister + 1) & (PCA9555_REGISTER_COUNT - 1);
}

void processCommand(int command);

static void applyPca9555Outputs() {
  int command = (pca9555Registers[PCA9555_OUTPUT_PORT_0] * 100) + pca9555Registers[PCA9555_OUTPUT_PORT_1];

  if (command != currentBand) {
    processCommand(command);
  }
}

static void handlePca9555Write(const uint8_t* data, uint8_t length) {
  uint8_t reg = data[0] & (PCA9555_REGISTER_COUNT - 1);
  bool outputsChanged = false;

  pca9555ReadRegister = reg;

  for (uint8_t i = 1; i < length && reg < PCA9555_REGISTER_COUNT; i++, reg++) {
    pca9555Registers[reg] = data[i];
    if (reg == PCA9555_OUTPUT_PORT_0 || reg == PCA9555_OUTPUT_PORT_1) {
      outputsChanged = true;
    }
  }

  pca9555ReadRegister = reg & (PCA9555_REGISTER_COUNT - 1);

  if (outputsChanged
      && pca9555Registers[PCA9555_CONFIGURATION_PORT_0] == 0x00
      && pca9555Registers[PCA9555_CONFIGURATION_PORT_1] == 0x00) {
    applyPca9555Outputs();
  }
}

static void processPendingI2cPacket() {
  if (!i2cPacketPending) {
    return;
  }

  uint8_t packet[I2C_BUFFER_SIZE];
  uint8_t length = 0;

  noInterrupts();
  length = i2cPacketLength;
  for (uint8_t i = 0; i < length; i++) {
    packet[i] = i2cPacket[i];
  }
  i2cPacketPending = false;
  interrupts();

  if (length == 0) {
    return;
  }

  if (length == 1) {
    pca9555ReadRegister = packet[0] & (PCA9555_REGISTER_COUNT - 1);
    return;
  }

  handlePca9555Write(packet, length);
}

void loop() {
  processPendingI2cPacket();

  unsigned long currentTime = millis();
  if (currentTime - lastPttCheck >= PTT_DEBOUNCE) {
    uint8_t pttState = digitalRead(PTT_PIN);

    if (i2cReady) {
      int rbPttRead = analogRead(RB_PTT_PIN);
      if (rbPttState && rbPttRead < ANALOG_THRESHOLD_OFF) {
        rbPttState = false;
      } else if (!rbPttState && rbPttRead > ANALOG_THRESHOLD_ON) {
        rbPttState = true;
      }
    }

    if (pttState != lastPttState) {
      transmit = (pttState == LOW);
      digitalWrite(TX_PIN, pttState);
      lastPttState = pttState;
    }

    digitalWrite(PA_PIN, transmit || rbPttState);
    lastPttCheck = currentTime;
  }

  delay(1);
}

void receiveEvent(int bytes) {
  (void)bytes;
  i2cReady = true;
  uint8_t length = 0;

  while (Wire.available() && length < I2C_BUFFER_SIZE) {
    i2cPacket[length++] = Wire.read();
  }

  while (Wire.available()) {
    Wire.read();
  }

  i2cPacketLength = length;
  i2cPacketPending = true;
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
