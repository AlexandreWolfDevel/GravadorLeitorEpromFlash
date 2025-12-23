/* ========================================================================================
   Autor________: Alexandre Stürmer Wolf
   Projeto______: Gravador de EPROM e memórias FLASH
   Dispositivo__: Arduino Mega 2560
   Importante___: Se utilizado VPP, externamente possui transistores chaveando 12 V,
                  ainda não implementado tensões maiores, também limitado a determinados
                  pinos, futuramente será expandido.
   Novos CIs____: Fazer o mapeamento e depois colocar no if selectedDevice no loop.
   Limitações___: Características e limitações
                  Atualmente é limitado ao 40 pinos (limitação conector ZIF);
                  A tensão para VCC e GND é proveniente de GPIO (pode não ser ideal);
  ======================================================================================== */

#define PIN_01 52
#define PIN_02 50
#define PIN_03 48
#define PIN_04 46
#define PIN_05 44
#define PIN_06 42
#define PIN_07 40
#define PIN_08 A0
#define PIN_09 A1
#define PIN_10 A2
#define PIN_11 A3
#define PIN_12 A4
#define PIN_13 A5
#define PIN_14 A6
#define PIN_15 A7
#define PIN_16 38
#define PIN_17 A8
#define PIN_18 A9
#define PIN_19 A10
#define PIN_20 A11

#define PIN_21 19
#define PIN_22 18
#define PIN_23 17
#define PIN_24 16
#define PIN_25 15
#define PIN_26 14
#define PIN_27 28
#define PIN_28 26
#define PIN_29 24
#define PIN_30 2
#define PIN_31 3
#define PIN_32 4
#define PIN_33 5
#define PIN_34 6
#define PIN_35 7
#define PIN_36 22
#define PIN_37 8
#define PIN_38 9
#define PIN_39 10
#define PIN_40 11

#define MAX_PARAMS 10
#define YES 1
#define NO 0

typedef struct {
  uint8_t numPins;                  // Number of pins in CI
  uint8_t numBitsData;              // Number of data pins
  uint8_t numBitsAddress;           // Number of address pins
  uint8_t arrayData[40];            // Until 40 pin data, normal 8 or 16, but...
  uint8_t arrayAddress[40];         // Until 40 pin address
  uint8_t pinVcc;                   // VCC pin
  uint8_t pinGnd;                   // GND pin
  uint8_t pinWr;                    // Write pin
  uint8_t pinOe;                    // Read - Output Enable pin
  uint8_t pinCe;                    // Chip Select pin
  uint8_t pinVpp;                   // VPP -> HIGH VOLTAGE enable
  uint8_t flagMagicWriteErase : 1;  // Uses Magic bytes to program
  uint8_t flagEraseAfterWrite : 1;  // Need erase after write (write protected)
  uint8_t flagToFuture : 6;         // Future use
} Pinout;

const Pinout AT29C010A = {
  32, 8, 17, { PIN_13, PIN_14, PIN_15, PIN_25, PIN_26, PIN_27, PIN_28, PIN_29 }, { PIN_12, PIN_11, PIN_10, PIN_09, PIN_08, PIN_07, PIN_06, PIN_05, PIN_35, PIN_34, PIN_31, PIN_33, PIN_04, PIN_36, PIN_37, PIN_03, PIN_02 }, PIN_40, PIN_16, PIN_39, PIN_32, PIN_30, -1, YES, NO
};

const Pinout GENERIC = {
  22, 8, 9, { PIN_13, PIN_14, PIN_15, PIN_16, PIN_17, PIN_18, PIN_19, PIN_20 }, { PIN_01, PIN_02, PIN_03, PIN_04, PIN_05, PIN_06, PIN_07, PIN_08, PIN_09 }, PIN_21, PIN_22, PIN_10, PIN_11, PIN_12, -1, NO, NO
};

Pinout selectedDevice;
String params[MAX_PARAMS];

void pinWrite(uint8_t pin, uint8_t val) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, val);
}

int StrToHex(String s) {
  return strtol(s.c_str(), NULL, 16);
}

void rawWrite(uint32_t addr, uint8_t data) {
  for (uint8_t i = 0; i < selectedDevice.numBitsAddress; i++) {
    pinWrite(selectedDevice.arrayAddress[i], (addr >> i) & 1);
  }

  for (uint8_t i = 0; i < selectedDevice.numBitsData; i++) {
    pinWrite(selectedDevice.arrayData[i], (data >> i) & 1);
  }

  pinWrite(selectedDevice.pinCe, LOW);
  pinWrite(selectedDevice.pinOe, HIGH);
  pinWrite(selectedDevice.pinWr, LOW);

  delayMicroseconds(1);

  pinWrite(selectedDevice.pinWr, HIGH);
  pinWrite(selectedDevice.pinCe, HIGH);

  delayMicroseconds(1);
}

void magicWrite(uint32_t addr, uint8_t data) {
  rawWrite(0x5555, 0xAA);
  rawWrite(0x2AAA, 0x55);
  rawWrite(0x5555, 0xA0);
  rawWrite(addr, data);
  delayMicroseconds(300);
}

void writeData(uint32_t addr, uint8_t data) {
  if (selectedDevice.flagMagicWriteErase) {
    magicWrite(addr, data);
  } else {
    rawWrite(addr, data);
  }
}

uint8_t readData(uint32_t addr) {
  for (uint8_t i = 0; i < selectedDevice.numBitsAddress; i++) {
    pinWrite(selectedDevice.arrayAddress[i], (addr >> i) & 1);
  }

  for (uint8_t i = 0; i < selectedDevice.numBitsData; i++) {
    pinMode(selectedDevice.arrayData[i], INPUT);
  }

  pinWrite(selectedDevice.pinWr, HIGH);
  pinWrite(selectedDevice.pinCe, LOW);
  pinWrite(selectedDevice.pinOe, LOW);

  delayMicroseconds(1);

  uint8_t val = 0;
  for (uint8_t i = 0; i < selectedDevice.numBitsData; i++) {
    if (digitalRead(selectedDevice.arrayData[i])) {
      val |= (1 << i);
    }
  }

  pinWrite(selectedDevice.pinOe, HIGH);
  pinWrite(selectedDevice.pinCe, HIGH);

  return val;
}

void writeChip() {
  pinWrite(selectedDevice.pinVcc, HIGH);
  pinWrite(selectedDevice.pinGnd, LOW);

  uint32_t addr = StrToHex(parametersGet("-a"));
  uint32_t n = parametersGet("-n").toInt();
  String data = parametersGet("-v");

  for (uint32_t i = 0; i < n; i++) {
    uint8_t value = StrToHex(data.substring(i * 2, i * 2 + 2));
    writeData(addr + i, value);
  }

  pinMode(selectedDevice.pinVcc, INPUT);
  pinMode(selectedDevice.pinGnd, INPUT);
}

void readChip() {
  pinWrite(selectedDevice.pinVcc, HIGH);
  pinWrite(selectedDevice.pinGnd, LOW);

  uint32_t addr = StrToHex(parametersGet("-a"));
  uint32_t n = parametersGet("-n").toInt();

  for (uint32_t i = 0; i < n; i++) {
    uint8_t value = readData(addr + i);
    if (value < 0x10) {
      Serial.print("0");
    }
    Serial.print(value, HEX);
  }
  pinMode(selectedDevice.pinVcc, INPUT);
  pinMode(selectedDevice.pinGnd, INPUT);
}

void eraseChip() {
  pinWrite(selectedDevice.pinVcc, HIGH);
  pinWrite(selectedDevice.pinGnd, LOW);

  if (selectedDevice.flagMagicWriteErase) {
    rawWrite(0x5555, 0xAA);
    rawWrite(0x2AAA, 0x55);
    rawWrite(0x5555, 0x80);
    rawWrite(0x5555, 0xAA);
    rawWrite(0x2AAA, 0x55);
    rawWrite(0x5555, 0x10);
    delay(200);
  }

  pinMode(selectedDevice.pinVcc, INPUT);
  pinMode(selectedDevice.pinGnd, INPUT);
}

void testConnections() {
  uint8_t pins[] = {
    PIN_01, PIN_02, PIN_03, PIN_04, PIN_05, PIN_06, PIN_07, PIN_08, PIN_09, PIN_10,
    PIN_11, PIN_12, PIN_13, PIN_14, PIN_15, PIN_16, PIN_17, PIN_18, PIN_19, PIN_20,
    PIN_21, PIN_22, PIN_23, PIN_24, PIN_25, PIN_26, PIN_27, PIN_28, PIN_29, PIN_30,
    PIN_31, PIN_32, PIN_33, PIN_34, PIN_35, PIN_36, PIN_37, PIN_38, PIN_39, PIN_40
  };

  for (uint8_t pin = 0; pin < 40; pin++) {
    Serial.print("Testing pin: ");
    Serial.println(pin+1);
    pinWrite(pins[pin], HIGH);
    delay(500);
    pinWrite(pins[pin], LOW);
    delay(500);
    pinMode(pins[pin], INPUT);
  }
}

void extractParameters(String cmd) {
  uint8_t pos = 0, s = 0;
  for (uint8_t i = 0; i < cmd.length(); i++) {
    if (cmd[i] == ' ') {
      params[pos++] = cmd.substring(s, i);
      s = i + 1;
    }
  }
  params[pos] = cmd.substring(s);
}

String parametersGet(String param) {
  String resp = "";
  for (uint8_t i = 0; i < MAX_PARAMS; i++) {
    if (params[i].startsWith(param)) {
      resp = params[i].substring(2);
      break;
    }
  }
  return resp;
}

void allInput() {
  for (uint8_t pin = A0; pin <= A15; pin++) {
    pinMode(pin, INPUT);
  }
  for (uint8_t pin = 2; pin <= 53; pin++) {
    pinMode(pin, INPUT);
  }
}

void setup() {
  allInput();
  Serial.begin(115200);
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readString();

    extractParameters(cmd);  // Global String array

    String device = parametersGet("-d");
    String operation = parametersGet("-o");

    if (device.equals("AT29C010A") || device.equals("W29EE01")) {
      selectedDevice = AT29C010A;
    } else if (device.equals("GENERIC")) {
      selectedDevice = GENERIC;
    } else {
      Serial.println("ERROR");
      return;
    }

    if (operation.equals("w")) {
      writeChip();
      Serial.println("OK");
    } else if (operation.equals("r")) {
      readChip();
      Serial.println("OK");
    } else if (operation.equals("e")) {
      eraseChip();
      Serial.println("OK");
    } else if (operation.equals("t")) {
      testConnections();
      Serial.println("OK");
    } else {
      Serial.println("ERROR");
    }
    allInput();
  }

  delay(10);
}
