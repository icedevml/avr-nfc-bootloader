MFRC522 Bootloader for AVR
==========================

Based on optiboot, works with CV520 and MFRC522. Check [optiboot.c#722](https://github.com/icedevml/avr-nfc-bootloader/blob/660e79389fe0a9c1189acb1623ef8ab1f8a84fdc/optiboot.c#L722) for actual differences between normal UART bootloader and this MFRC522 NFC bootloader.

## Why?

Because sometimes you don't have physical access to the device's debug connectors but you still want to be able to reprogram it.

## Demo
See: https://www.youtube.com/watch?v=Uo2WDgB-wPI

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

Please remember that values in `secret.h.dist` and `pinout.h.dist` are exemplary and you need to adjust them to match your setup.
