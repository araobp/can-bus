/* 
 * Created on March 13, 2018.
 * 
 * MCP2515 data sheet: http://ww1.microchip.com/downloads/en/DeviceDoc/21801d.pdf
 */

#ifndef MCP2515_H
#define	MCP2515_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>
    
#define SIDH(mask) ((uint8_t)((mask >> 3) & 0x00ffu))
#define SIDL(mask) ((uint8_t)(((mask & 0x0007u) << 5) & 0x00ffu))
#define SID_MAX 0b0000011111111111u

// P18 TXB0CTRL
#define TXB0CTRL 0x30u
#define TXB1CTRL 0x40u
#define TXB2CTRL 0x50u
#define TRANSMIT 0b00001011u  // TXBnCTRL transmit request with highest priority
#define TXERR 0b00010000u
    
// P27 RXB0CTRL
#define RXB0CTRL 0x60u
// P28 RXB1CTRL    
#define RXB1CTRL 0x70u
// Enable rollover to RXB1
#define RXB0CTRL_VALUE 0b00100100u
#define RXB1CTRL_VALUE 0b00100000u
    
// P34 mask and filters    
const uint8_t rxfnsidh[6] = {0x00u, 0x04u, 0x08u, 0x10u, 0x14u, 0x18u};
const uint8_t rxmnsidh[2] = {0x20u, 0x24u};

// P42 CNF1, CNF2 and CNF3
#define CNF1 0x2au
#define CNF2 0x29u
#define CNF3 0x28u
// FOSQ 8MHz, baudrate 125kbps
#define CNF1_VALUE 0b01000001u
#define CNF2_VALUE 0b11110001u
#define CNF3_VALUE 0b00000101u

// P57 Trl
#define T_RL 2  // 2 micro sec
#define T_RL_PLUS 48  // 48 micro sec
#define DELAY T_RL+T_RL_PLUS

// P58 CANCTRL
#define CANCTRL 0x0fu
#define NORMAL_MODE 0b00000000u
#define LOOPBACK_MODE 0b01000000u
#define CONFIGURATION_MODE 0b10000000u
#define ABAT 0b00010000u

// P59 CANSTAT
#define CANSTAT 0x0e

// P51 CANINTF
#define CANINTF 0x2c

// P64 SPI instructions
#define RESET_ 0b11000000u
#define READ 0b00000011u
#define READ_RX_BUFFER 0b10010000u
const uint8_t nm[2] = {0b00u, 0b10u};  // RXB0SIDH, RXB1SIDH
#define WRITE 0b00000010u
// P66 SPI instructions
#define LOAD_TX_BUFFER 0b01000000u
const uint8_t abc[3] = {0b000u, 0b010u, 0b100u};  // TXB0SIDH, TXB1SIDH, TXB2SIDH
#define RTS 0b10000000u
#define BIT_MODIFY 0b00000101u
// P67 SPI instructions
#define READ_STATUS 0b10100000u
#define RX0IF_MASK 0b00000001u
#define RX1IF_MASK 0b00000010u
#define TXB0_TXREQ 0b00000100u
#define TX0IF_MASK 0b00001000u
#define TXB1_TXREQ 0b00010000u
#define TX1IF_MASK 0b00100000u
#define TXB2_TXREQ 0b01000000u
#define TX2IF_MASK 0b10000000u
const uint8_t txbnsidh[3] = {0x31u, 0x41u, 0x51u};

#define SET_MASK 0u
#define SET_FILTER 1u

void can_logging_mode(bool debug, bool verbose);
void can_set_sid (uint16_t can_node);
void can_init(void *receive_handler);
bool can_ope_mode(uint8_t ope_mode);
void can_set_mask(uint8_t cmd, uint8_t n, uint16_t mask);
bool can_send(uint8_t *data_buf, uint8_t dlc);
bool can_status_check(void);
bool can_baudrate(uint8_t bpr);
bool can_abort(void);
void can_dump_masks_and_filters(void);
void can_dump_registers(void);

#ifdef	__cplusplus
}
#endif

#endif	/* MCP2515_H */

