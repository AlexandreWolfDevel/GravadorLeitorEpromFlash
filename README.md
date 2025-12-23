# GravadorLeitorEpromFlash
Dispositivo gravador de memórias EPROM e flash paralela.

Este programa e dispositivo utiliza uma Arduino Mega 2560 como base para um gravador de memórias com endereçamento e dados paralelos. Neste projeto foi utilizado um conector ZIF de 40 pinos encaixado em forma de shield. O dispositivo consegue ler uma vasta gama de memórias com a ligacão pino a pino conforme código fonte, no entanto, para gravação e apagamento das memórias flash é necessário utilização de um circuito adicional para fornecer mais potência, pois usando-se as GPIOs para alimentar a memória, pode ser pouca potência para VCC e GND.
