#include "spi.h"
#include "mcc_generated_files/spi.h"

uint8_t SPI_exchange(uint8_t *buf, uint8_t len) {
    CS_PIN = 0;  // Lower CS pin on MCP2515
    uint8_t bytes_written = SPI_Exchange8bitBuffer(buf, len, buf);
    CS_PIN = 1;  // Raise CS pin on MCP2515
    return bytes_written;
}
