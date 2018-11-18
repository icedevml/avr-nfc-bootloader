MFRC522 Bootloader for AVR
==========================

Based on optiboot, works with CV520 and MFRC522.

## Why?

Because sometimes you don't have physical access to the device but you still want to be able to reprogram it.

## Commands I was using:

Build with:
```
make atmega164p SOFT_UART=1 UART=0 AVR_FREQ=8000000L BAUD_RATE=115200 LED_START_FLASHES=3 LED=A1 LED_DATA_FLASH=1
```

Erase device flash:
```
avrdude -v -patmega164p -cusbasp -e -B10
```

Program optiboot and fuses:
```
avrdude -v -patmega164p -cusbasp "-Uflash:w:optiboot_atmega164p_UART0_115200_8000000L.hex:i" -U lfuse:w:0xE2:m -U hfuse:w:0xD8:m -U lock:w:0xCC:m -B10
```

## Android app to interact with bootloader
See: https://github.com/icedevml/avr-nfc-bootloader-app

## Notes
Tested with ATmega-164PA, should work with any other microprocessor which is supported by optiboot, but you need to adjust `Makefile.MCUdude` manually for your microprocessor.
