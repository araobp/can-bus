/* 
 * MCP2515 data sheet: http://ww1.microchip.com/downloads/en/DeviceDoc/21801d.pdf
 */

#include <stdbool.h>
#include <stdio.h>
#include "mcp2515.h"
#include "spi.h"

/*
 *  TXBnSIDH and TXBnSIDL on MCP2515
 */
struct {
    uint8_t sidh;
    uint8_t sidl;
} sid;

struct {
    bool debug;
    bool verbose;
} mode;

uint8_t operation_mode;
void (*handler)(uint8_t sid, uint8_t *pbuf, uint8_t dlc) = NULL;

void can_logging_mode(bool debug, bool verbose) {
    mode.debug = debug;
    mode.verbose = verbose;
}

/*
 * Set CAN message ID (11bit length)
 * 
 * SIDH: [SID10|SID9|SID8|SID7|SID6|SID5|SID4|SID3]
 * SIDL: [SID2|SID1|SID0| - |EXIDE| - |EID17|EID16]
 */
void can_set_sid (uint16_t can_node) {
    uint8_t sidh = (can_node >> 3) & 0x00ff;   // SID3 ~ SID10
    uint8_t sidl = (can_node << 5) & 0x00e0;   // SID0 ~ SID2
    sid.sidh = sidh;
    sid.sidl = sidl;
}

/*
 * Initialize this program and MCP2515 with default config 
 */
void can_init(void *receive_handler) {
    can_set_sid(0);
    
    handler = receive_handler;

    can_ope_mode(CONFIGURATION_MODE);    
    uint8_t cnf_buf[5] = {WRITE, CNF3, CNF3_VALUE, CNF2_VALUE, CNF1_VALUE};
    SPI_exchange(cnf_buf, 5);
    
    mode.debug = false;
    mode.verbose = false;
    can_ope_mode(NORMAL_MODE);
}

/*
 * P58 Set MCP2515 to normal mode
 */
bool can_ope_mode(uint8_t ope_mode) {
    uint8_t can_ctrl_buf[3];
    can_ctrl_buf[0] = WRITE;
    can_ctrl_buf[1] = CANCTRL;
    can_ctrl_buf[2] = ope_mode;
    uint8_t bytes_written = SPI_exchange(can_ctrl_buf, 3);

    if (ope_mode != CONFIGURATION_MODE) operation_mode = ope_mode;
    
    if (bytes_written == 3) {
        can_ctrl_buf[0] = READ;
        can_ctrl_buf[1] = CANSTAT;
        bytes_written = SPI_exchange(can_ctrl_buf, 3);
        if (mode.debug) printf("CANSTAT: %02x\n", can_ctrl_buf[2]);
        return true;
    } else {
        return false;
    }
}

/*
 * Set mask and filter
 */
void can_set_mask(uint8_t cmd, uint8_t n, uint8_t mask) {
    uint8_t mask_buf[4];

    can_ope_mode(CONFIGURATION_MODE);  // P33
    uint8_t mask_sidh = (uint8_t)((mask >> 3) & 0x00ff);
    uint8_t mask_sidl = (uint8_t)(((mask & 0x0007) << 5) & 0x00ff);
    if (mode.debug) {
        if (cmd == 'm') {
            printf("mask(%d): %02x %02x\n", n, mask_sidh, mask_sidl);
        } else {
            printf("filter(%d): %02x %02x\n", n, mask_sidh, mask_sidl);                                    
        }
    }
    mask_buf[0] = WRITE;
    mask_buf[1] = (cmd == 'm')? rxmnsidh[n]: rxfnsidh[n];
    mask_buf[2] = mask_sidh;
    mask_buf[3] = mask_sidl;
    SPI_exchange(mask_buf, 4);

    if (mode.debug) {
        mask_buf[0] = READ;
        mask_buf[1] = (cmd == 'm')? rxmnsidh[n]: rxfnsidh[n];
        mask_buf[2] = 0x00;
        mask_buf[3] = 0x00;
        SPI_exchange(mask_buf, 4);
        if (cmd == 'm') {
            printf("RXM%dSIDH: %02x\n", n, mask_buf[2]);
            printf("RXM%dSIDL: %02x\n", n, mask_buf[3]);
        } else {
            printf("RXF%dSIDH: %02x\n", n, mask_buf[2]);
            printf("RXF%dSIDL: %02x\n", n, mask_buf[3]);
        }
    }

    can_ope_mode(operation_mode);   

}

/*
 * P51 Clear transmit buffer empty interrupt flag
 */
bool txf_clear(uint8_t n) {
    uint8_t mask = 0b00000001 << (n + 2);
    uint8_t can_int_flag[4];
    can_int_flag[0] =BIT_MODIFY;
    can_int_flag[1] = CANINTF;
    can_int_flag[2] = mask;
    can_int_flag[3] = 0x00;
    uint8_t bytes_written = SPI_exchange(can_int_flag, 4);
    if (bytes_written == 4) {
        return true;
    } else {
        return false;
    }
}

/*
 * Receive CAN message
 */
void receive(uint8_t n) {
    
    // n: 0 ~ 1
    // RXBnSDIH, RXBnSDIL, RXBnEID8, RXBnEID0, RXBnDLC, RXBnDm (m: 0 ~ 7)
    uint8_t rx_buf[15];  // Append '\0' at the tail

    // P65
    rx_buf[0] = READ_RX_BUFFER + (nm[n] << 1);
    uint8_t bytes_read = SPI_exchange(rx_buf, 14);
    uint8_t dlc = rx_buf[5];
    uint16_t sid = ((uint16_t)rx_buf[1] << 3) + ((uint16_t)rx_buf[2] >> 5);
    sid = sid & 0b0000011111111111;
    rx_buf[6+dlc] = '\0';
    
    if (mode.debug) {
        printf("RXB%dSIDH: %02x\n", n, rx_buf[1]);
        printf("RXB%dSIDL: %02x\n", n, rx_buf[2]);    
        printf("RXB%dDLC: %02x\n", n, rx_buf[5]);    
        printf("RXB%dD0: %02x\n", n, rx_buf[6]);
    }

    // Output the received message from CAN bus
    uint8_t *pbuf = &rx_buf[6];
    handler(sid, pbuf, dlc);
}

/*
 * Send CAN message
 */
bool can_send(uint8_t *buf, uint8_t dlc) {
    uint8_t i;
    uint8_t n = 0;
    
    uint8_t can_status_buf[2] = {READ_STATUS, 0x00}; 
    uint8_t bytes_written = SPI_exchange(can_status_buf, 2);
    uint8_t status = can_status_buf[1];
    if ((status & TXB0_TXREQ) == 0) {
        if (mode.debug) printf("TXB0 is idle\n");
        n = 0;
    } else if ((status & TXB1_TXREQ) == 0) {
        if (mode.debug) printf("TXB1 is idle\n");
        n = 1;
    } else if ((status & TXB2_TXREQ) > 0) {
        if (mode.debug) printf("TXB2 is idle\n");
        n = 2;
    }
    
    // n: 0 ~ 2
    // TXBnSDIH, TXBnSDIL, TXBnEID8, TXBnEID0, TXBnDLC, TXBnDm (m: 0 ~ 7)
    uint8_t tx_buf[14];

    // P66
    tx_buf[0] = LOAD_TX_BUFFER + abc[n];
    // P19 TXBnSIDH
    tx_buf[1] = sid.sidh;
    // P20 TXBnSIDL
    tx_buf[2] = sid.sidl;
    // P20 TXBnEID8
    tx_buf[3] = 0;
    // P20 TXBnEID0
    tx_buf[4] = 0;
    // P21 TXBnDLC
    tx_buf[5] = dlc;

    // Copy buffer
    for(i=0; i<dlc; i++) {
        tx_buf[6+i] = buf[i];
        if (mode.debug) printf("Copying buffer: %c\n", tx_buf[6+i]);
    }

    // P66 Load TX buffer
    uint8_t len = 6 + dlc;
    uint8_t bytes_written = SPI_exchange(tx_buf, len);

    uint8_t debug_buf[3];
    debug_buf[0] = READ;
    debug_buf[1] = txbnsidh[n];
    debug_buf[2] = 0x00;
    SPI_exchange(debug_buf, 3);
    
    if (mode.debug) printf("TXB%dSIDH: %02x\n", n, debug_buf[2]);
    debug_buf[0] = READ;
    debug_buf[1] = txbnsidh[n] + 1;  //TXBnSIDL
    debug_buf[2] = 0x00;
    SPI_exchange(debug_buf, 3);
   
    if (mode.debug) printf("TXB%dSIDL: %02x\n", n, debug_buf[2]);
    
    if (bytes_written == len) {
        // P64, P66 RTS
        uint8_t txb_ctrl_buf[1];
        txb_ctrl_buf[0] = RTS + (0b00000001 << n);
        bytes_written = SPI_exchange(txb_ctrl_buf, 1);
        if (bytes_written == 1) {
            if (mode.debug) printf("Message sent: %s\n", buf);
            return true;
        }
    }
    
    return false;
}

/*
 * Check CAN status
 */
void can_status_check(void) {
    // P67
    uint8_t can_status_buf[2] = {READ_STATUS, 0x00}; 
    uint8_t bytes_written = SPI_exchange(can_status_buf, 2);
    uint8_t status = can_status_buf[1];
    if (status == 0x00) {
        return;
    } else if ((status & RX0IF_MASK) > 0) {
        if (mode.debug) printf("RX0IF is on\n");
        receive(0);
    } else if ((status & RX1IF_MASK) > 0) {
        if (mode.debug) printf("RX1IF is on\n");
        receive(1);
    } else if ((status & TX0IF_MASK) > 0) {
        txf_clear(0);
    } else if ((status & TX1IF_MASK) > 0) {
        txf_clear(1);        
    } else if ((status & TX2IF_MASK) > 0) {
        txf_clear(2);
    }
    return;
}

bool can_baudrate(uint8_t bpr) {
    can_ope_mode(CONFIGURATION_MODE);
    // P42
    uint8_t mask = 0b00111111;
    uint8_t cnf1_buf[4];
    cnf1_buf[0] = BIT_MODIFY;
    cnf1_buf[1] = CNF1;
    cnf1_buf[2] = mask;
    cnf1_buf[3] = bpr;
    uint8_t bytes_written = SPI_exchange(cnf1_buf, 4);
    can_ope_mode(operation_mode);
    if (bytes_written == 4) {
        return true;
    } else {
        return false;
    }
}

uint8_t read_register(uint8_t addr) {
    uint8_t read_buf[3];
    read_buf[0] = READ;
    read_buf[1] = addr;
    read_buf[2] = 0x00;
    SPI_exchange(read_buf, 3);
    return read_buf[2];
}

/*
 * Dump registers
 */
void can_dump_registers(void) {
    printf("---\n");
    printf("SIDH: %02x\n", sid.sidh);
    printf("SIDL: %02x\n", sid.sidl);
    printf("\n");
    
    printf("CNF1: %02x\n", read_register(CNF1));
    printf("CNF2: %02x\n", read_register(CNF2));
    printf("CNF3: %02x\n", read_register(CNF3));    
    printf("\n");
    
    uint8_t n;
    uint8_t i;
    uint8_t mask_sidh;
    uint8_t mask_sidl;
    for (n=0; n<2; n++) {
        mask_sidh = read_register(rxmnsidh[n]);
        mask_sidl = read_register(rxmnsidh[n]+1);
        printf("RXM%dSIDH RXM%dSIDL: %02x %02x\n", n, n, mask_sidh, mask_sidl);
    }
    for (n=0; n<6; n++) {
        mask_sidh = read_register(rxfnsidh[n]);
        mask_sidl = read_register(rxfnsidh[n]+1);
        printf("RXF%dSIDH RXF%dSIDL: %02x %02x\n", n, n, mask_sidh, mask_sidl);
    }
    printf("\n");
    
    printf("CANCTRL: %02x\n", read_register(CANCTRL));
    printf("CANSTAT: %02x\n", read_register(CANSTAT));
    printf("---\n");    
}
