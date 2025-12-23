# GravadorLeitorEpromFlash
Dispositivo gravador de memórias EPROM e flash paralela.

Este programa e dispositivo utiliza uma Arduino Mega 2560 como base para um gravador de memórias com endereçamento e dados paralelos. Neste projeto foi utilizado um conector ZIF de 40 pinos encaixado em forma de shield. O dispositivo consegue ler uma vasta gama de memórias com a ligacão pino a pino conforme código fonte, no entanto, para gravação e apagamento das memórias flash é necessário utilização de um circuito adicional para fornecer mais potência, pois usando-se as GPIOs para alimentar a memória, pode ser pouca potência para VCC e GND. No caso de desejar gravar EPROM, dependendo do modelo será necessário entre 12 a 23 volts no pino VPP, assim é necessário fonte externa.

É possível ampliar o número de memórias compatíveis para um número muito grande, para isso é só criar um mapeamento dos pinos, e acrescentar so selectedDevice na função de Loop, como por exemplo:

const Pinout AT29C010A = {
  32, 8, 17, 
  { PIN_13, PIN_14, PIN_15, PIN_25, PIN_26, PIN_27, PIN_28, PIN_29 }, 
  { PIN_12, PIN_11, PIN_10, PIN_09, PIN_08, PIN_07, PIN_06, PIN_05, PIN_35, PIN_34, PIN_31, PIN_33, PIN_04, PIN_36, PIN_37, PIN_03, PIN_02 }, 
  PIN_40, PIN_16, PIN_39, PIN_32, PIN_30, -1, YES, NO
};

onde, 

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

Observar que dependendo do tipo de flash, os "magic bytes" podem possuir outra sequencia, neste caso será necessário verificar a subrotina magicWrite.

Ajude a fazer o mapeamento para outras memórias, assim como sugerir melhorias no circuito. 
