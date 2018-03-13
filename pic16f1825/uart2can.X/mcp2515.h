/* 
 * Created on March 13, 2018.
 */

#ifndef MCP2515_H
#define	MCP2515_H

#ifdef	__cplusplus
extern "C" {
#endif

// P18 TXB0CTRL
#define TXB0CTRL 0x30
#define TRANSMIT 0b00001011  // TXBnCTRL transmit request with highest priority

// P34 mask and filters    
const uint8_t rxfnsidh[6] = {0x00, 0x04, 0x08, 0x10, 0x14, 0x18};
const uint8_t rxmnsidh[2] = {0x20, 0x24};
    
// P58 CANCTRL
#define CANCTRL 0x0f
#define NORMAL_MODE 0b00000000
#define LOOPBACK_MODE 0b01000000
#define CONFIGURATION_MODE 0b10000000

// P59 CANSTAT
#define CANSTAT 0x0e

// P51 CANINTF
#define CANINTF 0x2c

// P65 SPI instructions
#define READ 0b00000011
#define READ_RX_BUFFER 0b10010000
const uint8_t nm[2] = {0b00, 0b10};  // RXB0SIDH, RXB1SIDH
#define WRITE 0b00000010
// P66 SPI instructions
#define LOAD_TX_BUFFER 0b01000000
const uint8_t abc[3] = {0b000, 0b010, 0b100};  // TXB0SIDH, TXB1SIDH, TXB2SIDH
#define RTS 0b10000000
#define BIT_MODIFY 0b00000101
// P67 SPI instructions
#define READ_STATUS 0b10100000
#define RX0IF_MASK 0b00000001
#define RX1IF_MASK 0b00000010
#define TXB0_TXREQ 0b00000100
#define TX0IF_MASK 0b00001000
#define TXB1_TXREQ 0b00010000
#define TX1IF_MASK 0b00100000
#define TXB2_TXREQ 0b01000000
#define TX2IF_MASK 0b10000000
const uint8_t txbnsidh[3] = {0x31, 0x41, 0x51};

/*
 *  TXBnSIDH and TXBnSIDL on MCP2515
 */
typedef struct {
    uint8_t sidh;
    uint8_t sidl;
} SID;

#ifdef	__cplusplus
}
#endif

#endif	/* MCP2515_H */

