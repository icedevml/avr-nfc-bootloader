uint8_t spi_transfer(uint8_t data)
{
  uint8_t readData;
  uint8_t writeData;
  uint8_t builtReturn;
  uint8_t mask;

  mask = 0x80;
  builtReturn = 0;
  readData = 0;

  for (uint8_t count=0; count<8; count++)
  {
    if (data & mask)
    {
      writeData = 1;
    }
    else
    {
      writeData = 0;
    }

    // CPHA=0, miso/mosi changing state now
    PIN_SET(PIN_MOSI_CV520, writeData);
    PIN_OFF(PIN_SCK_CV520);
    _delay_us(1);

    // CPHA=0, miso/mosi stable now
    readData = PIN_READ(PIN_MISO_CV520) ? 1 : 0;
    PIN_ON(PIN_SCK_CV520);
    _delay_us(1);

    mask >>= 1;
    builtReturn |= (readData << (7 - count));
  }

  PIN_OFF(PIN_SCK_CV520);

  return builtReturn;
}

void spi_begin()
{
  PIN_OFF(PIN_SCK_CV520);
}

void spi_write(uint8_t addr, uint8_t val) {
  PIN_OFF(PIN_NSS_CV520);
  spi_transfer(addr << 1);
  spi_transfer(val);
  PIN_ON(PIN_NSS_CV520);
}

uint8_t spi_read(uint8_t addr) {
  PIN_OFF(PIN_NSS_CV520);
  spi_transfer(0x80 | (addr << 1));
  uint8_t out = spi_transfer(0);
  PIN_ON(PIN_NSS_CV520);

  return out;
}

uint8_t spi_poll(uint8_t addr, uint8_t byte_mask, int max_tries) {
  PIN_OFF(PIN_NSS_CV520);
  uint8_t cmd = 0x80 | (addr << 1);
  spi_transfer(cmd);
  uint8_t tries = 0;

  while (max_tries > tries) {
    tries++;
    uint8_t val = spi_transfer(cmd);

    if ((val & byte_mask) == byte_mask) {
      return 1;
    }

    _delay_ms(1);
  }

  PIN_ON(PIN_NSS_CV520);
  return 0;
}

void spi_batch(const uint8_t *data, size_t len) {
  while (len > 0) {
    uint8_t reg = *data++;
    uint8_t val = *data++;
    spi_write(reg, val);
    len -= 2;
  }
}
