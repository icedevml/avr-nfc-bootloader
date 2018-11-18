const uint8_t CMD_WAKEUP[]        = {0x52};
const uint8_t CMD_ANTICOL[]       = {0x93, 0x20};
const uint8_t CMD_REQSAK[]        = {0xE0, 0x50};
const uint8_t CMD_DESEL[]         = {0xC2};
const uint8_t CMD_APDU_SEL_STK[]  = {0x00, 0xA4, 0x04, 0x00, 0x07, 0xF0, 0xC0, 0xFF, 0xEE, 0x13, 0x37, 0x00};
const uint8_t CMD_APDU_NEXT_CMD_RES_OK[]  = {0x00, 0xC2, 0x00, 0x00, 0x01, 0xAF, 0x00, 0x00};
const uint8_t CMD_APDU_NEXT_CMD_RES_ERR[] = {0x00, 0xC2, 0x00, 0x00, 0x01, 0xAF, 0x01, 0x00};

const uint8_t init_cmds[] = {
    REG_01_CommandReg, 0x07,
    REG_11_ModeReg, 0x3d,
    REG_2d_TReloadReg, 0x1e,
    REG_2c_TReloadReg, 0x00,
    REG_2a_TModeReg, 0x8d,
    REG_2b_TPrescalerReg, 0x3e,
    REG_15_TxASKReg, 0x40,
    REG_08_Status2Reg, 0x00,
    REG_0d_BitFramingReg, 0x07,
    REG_14_TxControlReg, 0x83,
    REG_02_ComlEnReg, 0xf7,
    REG_04_ComIrqReg, 0x14,
    REG_01_CommandReg, 0x00
};

const uint8_t transceive_cmds[] = {
    REG_0c_ControlReg, 0x90,
    REG_01_CommandReg, 0x00,
    REG_08_Status2Reg, 0x00,
    REG_0d_BitFramingReg, 0x00,
    REG_0e_CollReg, 0x20,
    REG_02_ComlEnReg, 0xf7,
    REG_04_ComIrqReg, 0x64,
    REG_01_CommandReg, 0x00,
    REG_0a_FIFOLevelReg, 0x80
};

const uint8_t transceive_cmds2[] = {
    REG_0c_ControlReg, 0x90,
    REG_01_CommandReg, 0x00,
    REG_0e_CollReg, 0xa0
};

uint8_t current_pcb = 0x02;

uint8_t exchange_pcb(uint8_t pcb) {
  return pcb ^ 0x01;
}

void uart_puts(char *str);

void iso14443a_crc(const uint8_t* pbtData, size_t szLen, uint8_t *crclsb, uint8_t *crcmsb, uint8_t with_pcb) {
  uint8_t bt;
  uint32_t wCrc = 0x6363;

  do {
    if (with_pcb) {
      szLen++;
      with_pcb = 0;
      bt = current_pcb;
    } else {
      bt = *pbtData++;
    }
    bt = (bt^(uint8_t)(wCrc & 0x00FF));
    bt = (bt^(bt<<4));
    wCrc = (wCrc >> 8)^((uint32_t)bt << 8)^((uint32_t)bt<<3)^((uint32_t)bt>>4);
  } while (--szLen);

  *crclsb = (uint8_t) (wCrc & 0xFF);
  *crcmsb = (uint8_t) ((wCrc >> 8) & 0xFF);
}

void nfc_init() {
  spi_batch(init_cmds, sizeof(init_cmds));
}

int nfc_transceive(const uint8_t *buf, size_t buf_size, uint8_t with_crc, uint8_t with_pcb, uint8_t *out_buf, size_t out_buf_size) {
  uint8_t crclsb;
  uint8_t crcmsb;

  PIN_ON(PIN_LED_B);

  if (with_crc) {
    iso14443a_crc(buf, buf_size, &crclsb, &crcmsb, with_pcb);
  }

  spi_batch(transceive_cmds, sizeof(transceive_cmds));

  PIN_OFF(PIN_NSS_CV520);
  spi_transfer(0x12); // write fifo

  if (with_pcb) {
    spi_transfer(current_pcb);
    current_pcb = exchange_pcb(current_pcb);
  }

  for (int i = 0; i < buf_size; i++) {
    spi_transfer(buf[i]);
  }

  if (with_crc) {
    spi_transfer(crclsb);
    spi_transfer(crcmsb);
  }

  PIN_ON(PIN_NSS_CV520);

  spi_write(REG_01_CommandReg, 0x0c);      // resp 0ch 00001100

  if (buf == CMD_WAKEUP) {
    spi_write(REG_0d_BitFramingReg, 0x87);   // resp 80h 10000000
  } else {
    spi_write(REG_0d_BitFramingReg, 0x80);   // resp 80h 10000000
  }

  PIN_OFF(PIN_LED_B);

  if (!spi_poll(REG_04_ComIrqReg, 1 << 5, 100)) {
    return -2;
  }

  PIN_ON(PIN_LED_R);

  spi_write(REG_0d_BitFramingReg, 0x00);   // resp 00h 00000000

  uint8_t fifo_bytes = (spi_read(REG_0a_FIFOLevelReg) & 0x7F);
  uint8_t out_buf_too_small = 0;

  if (fifo_bytes > 0) {
    PIN_OFF(PIN_NSS_CV520);
    spi_transfer(0x92);

    for (int i = 0; i < fifo_bytes; i++) {
      if (i > out_buf_size) {
        out_buf_too_small = 1;
        break;
      }

      out_buf[i] = spi_transfer(0x92);
    }

    PIN_ON(PIN_NSS_CV520);
  }

  spi_batch(transceive_cmds2, sizeof(transceive_cmds2));

  PIN_OFF(PIN_LED_R);

  if (out_buf_too_small) {
    return -3;
  }

  return fifo_bytes;
}

int nfc_transceive_apdu(const uint8_t *buf, size_t buf_size, uint8_t *out_buf, size_t out_buf_size) {
  int read_bytes = nfc_transceive(buf, buf_size, 1, 1, out_buf, out_buf_size);
  uint8_t retries = 15;

  if (read_bytes <= 0) {
    return read_bytes;
  }

  while (out_buf[0] == 0xF2) {
    _delay_ms(5);
    read_bytes = nfc_transceive(out_buf, read_bytes, 0, 0, out_buf, out_buf_size);

    if (read_bytes <= 0) {
      return read_bytes;
    }

    retries--;

    if (retries == 0) {
      return -4;
    }
  }

  return read_bytes;
}

