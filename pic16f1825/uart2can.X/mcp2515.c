/* 
 * MCP2515 data sheet: http://ww1.microchip.com/downloads/en/DeviceDoc/21801d.pdf
 */

#include <stdbool.h>
#include <stdio.h>
#include "mcc_generated_files/mcc.h"
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
void (*handler)(uint16_t, uint8_t *, uint8_t) = NULL;

uint16_t save_rxmn[2] = {0u, 0u};
uint16_t save_rxfn[6] = {0u, 0u, 0u, 0u, 0u, 0u};

void can_logging_mode(bool debug, bool verbose) {
    mode.debug = debug;
    mode.verbose = verbose;
}

uint8_t buf[16];

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
    __delay_ms(100);
    
    can_set_sid(0);   
    handler = receive_handler;
    
    //can_ope_mode(CONFIGURATION_MODE);    
    buf[0] = RESET_;
    SPI_exchange(buf, 1);
    
    __delay_us(DELAY);

    buf[0] = WRITE;
    buf[1] = CNF3;
    buf[2] = CNF3_VALUE;
    buf[3] = CNF2_VALUE;
    buf[4] = CNF1_VALUE;
    SPI_exchange(buf, 5);
          
    buf[0] = WRITE;
    buf[1] = RXB0CTRL;
    buf[2] = RXB0CTRL_VALUE;
    SPI_exchange(buf, 3);

    buf[0] = WRITE;
    buf[1] = RXB1CTRL;
    buf[2] = RXB1CTRL_VALUE;
    SPI_exchange(buf, 3);
    
    mode.debug = false;
    mode.verbose = false;

    can_ope_mode(NORMAL_MODE);
}

/*
 * P58 Set MCP2515 to specific mode
 */
bool can_ope_mode(uint8_t ope_mode) {
    buf[0] = WRITE;
    buf[1] = CANCTRL;
    buf[2] = ope_mode;
    uint8_t bytes_written = SPI_exchange(buf, 3);

    if (ope_mode != CONFIGURATION_MODE) operation_mode = ope_mode;
    
    if (bytes_written == 3) {
        buf[0] = READ;
        buf[1] = CANSTAT;
        bytes_written = SPI_exchange(buf, 3);
        if (mode.debug) printf("CANSTAT: %02x\n", buf[2]);
        return true;
    } else {
        return false;
    }
}

uint8_t abort_pending_transmissions(void) {
    buf[0] = BIT_MODIFY;
    buf[1] = CANCTRL;
    buf[2] = ABAT;
    buf[3] = ABAT;
    uint8_t bytes_written = SPI_exchange(buf, 4);
    return bytes_written;
}

/*
 * Set mask and filter
 */
void can_set_mask(uint8_t cmd, uint8_t n, uint16_t mask) {
    can_ope_mode(CONFIGURATION_MODE);  // P33
    abort_pending_transmissions();  // P16
    mask = (mask > SID_MAX)? SID_MAX: mask; 
    uint8_t mask_sidh = SIDH(mask);
    uint8_t mask_sidl = SIDL(mask);

    if (cmd == SET_MASK) {
        save_rxmn[n] = mask;
        if (mode.debug) printf("mask(%d): %02x %02x\n", n, mask_sidh, mask_sidl);
    } else {
        save_rxfn[n] = mask;
        if (mode.debug) printf("filter(%d): %02x %02x\n", n, mask_sidh, mask_sidl);                                    
    }
    
    buf[0] = WRITE;
    buf[1] = (cmd == SET_MASK)? rxmnsidh[n]: rxfnsidh[n];
    buf[2] = mask_sidh;
    buf[3] = mask_sidl;
    SPI_exchange(buf, 4);
    
    can_ope_mode(operation_mode);   
}

/*
 * P51 Clear transmit buffer empty interrupt flag
 */
bool txf_clear(uint8_t n) {
    uint8_t mask = 0b00000001u << (n + 2);
    buf[0] =BIT_MODIFY;
    buf[1] = CANINTF;
    buf[2] = mask;
    buf[3] = 0x00;
    uint8_t bytes_written = SPI_exchange(buf, 4);
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
    uint16_t id = ((uint16_t)rx_buf[1] << 3) + ((uint16_t)rx_buf[2] >> 5);
    id = id & 0b0000011111111111u;
    rx_buf[6u+dlc] = (uint8_t)'\0';
    
    if (mode.debug) {
        printf("RXB%dSIDH: %02x\n", n, rx_buf[1]);
        printf("RXB%dSIDL: %02x\n", n, rx_buf[2]);    
        printf("RXB%dDLC: %02x\n", n, rx_buf[5]);    
        printf("RXB%dD0: %02x\n", n, rx_buf[6]);
    }

    // Output the received message from CAN bus
    uint8_t *pbuf = &rx_buf[6];
    handler(id, pbuf, dlc);
}

/*
 * Send CAN message
 */
bool can_send(uint8_t *data_buf, uint8_t dlc) {
    uint8_t i;
    uint8_t n = 0;
    
    buf[0] = READ_STATUS;
    buf[1] = 0x00;
    uint8_t bytes_written = SPI_exchange(buf, 2);
    uint8_t status = buf[1];
    if ((status & TXB2_TXREQ) == 0) {
        if (mode.debug) printf("TXB2 is idle\n");
        n = 2;
    } else if ((status & TXB1_TXREQ) == 0) {
        if (mode.debug) printf("TXB1 is idle\n");
        n = 1;
    } else if ((status & TXB0_TXREQ) == 0) {
        if (mode.debug) printf("TXB0 is idle\n");
        n = 0;
    }
    
    // n: 0 ~ 2
    // TXBnSDIH, TXBnSDIL, TXBnEID8, TXBnEID0, TXBnDLC, TXBnDm (m: 0 ~ 7)

    // P66
    buf[0] = LOAD_TX_BUFFER + abc[n];
    // P19 TXBnSIDH
    buf[1] = sid.sidh;
    // P20 TXBnSIDL
    buf[2] = sid.sidl;
    // P20 TXBnEID8
    buf[3] = 0;
    // P20 TXBnEID0
    buf[4] = 0;
    // P21 TXBnDLC
    buf[5] = dlc;

    // Copy buffer
    for(i=0; i<dlc; i++) {
        buf[6u+i] = data_buf[i];
        if (mode.debug) printf("Copying buffer: %c\n", data_buf[i]);
    }

    // P66 Load TX buffer
    uint8_t len = 6u + dlc;
    uint8_t bytes_written = SPI_exchange(buf, len);

    buf[0] = READ;
    buf[1] = txbnsidh[n];
    buf[2] = 0x00;
    SPI_exchange(buf, 3);
    
    if (mode.debug) printf("TXB%dSIDH: %02x\n", n, buf[2]);
    buf[0] = READ;
    buf[1] = txbnsidh[n] + 1u;  //TXBnSIDL
    buf[2] = 0x00;
    SPI_exchange(buf, 3);
   
    if (mode.debug) printf("TXB%dSIDL: %02x\n", n, buf[2]);
    
    if (bytes_written == len) {
        // P64, P66 RTS
        buf[0] = RTS + (0b00000001u << n);
        bytes_written = SPI_exchange(buf, 1);
        if (bytes_written == 1) {
            if (mode.debug) printf("Message sent: %s\n", data_buf);
            return true;
        }
    }
    
    return false;
}

/*
 * Check CAN status
 */
bool can_status_check(void) {
    // P67
    buf[0] = READ_STATUS;
    buf[1] = 0x00;
    uint8_t bytes_written = SPI_exchange(buf, 2);
    uint8_t status = buf[1];
    if (status == 0x00) {
        return true;
    } else if ((status & RX1IF_MASK) > 0) {
        if (mode.debug) printf("RX1IF is on\n");
        receive(1);
    } else if ((status & RX0IF_MASK) > 0) {
        if (mode.debug) printf("RX0IF is on\n");
        receive(0);
    } else if ((status & TX0IF_MASK) > 0) {
        txf_clear(0);
    } else if ((status & TX1IF_MASK) > 0) {
        txf_clear(1);        
    } else if ((status & TX2IF_MASK) > 0) {
        txf_clear(2);
    }
    if ((status & TXERR) > 0) {
        return false;
    } else {
        return true;
    }
}

bool can_baudrate(uint8_t bpr) {
    can_ope_mode(CONFIGURATION_MODE);
    abort_pending_transmissions;
    // P42
    uint8_t mask = 0b00111111;
    buf[0] = BIT_MODIFY;
    buf[1] = CNF1;
    buf[2] = mask;
    buf[3] = bpr;
    uint8_t bytes_written = SPI_exchange(buf, 4);
    can_ope_mode(operation_mode);
    if (bytes_written == 4) {
        return true;
    } else {
        return false;
    }
}

bool can_abort(void) {
    can_ope_mode(CONFIGURATION_MODE);    
    uint8_t bytes_written = abort_pending_transmissions();
    can_ope_mode(operation_mode);
    if (bytes_written == 4) {
        return true;
    } else {
        return false;
    }    
}

uint8_t read_register(uint8_t addr) {
    buf[0] = READ;
    buf[1] = addr;
    buf[2] = 0x00;
    SPI_exchange(buf, 3);
    return buf[2];
}

void can_dump_masks_and_filters(void) {
    uint8_t i, h, l;
    uint16_t m;
    printf("---\n");
    for(i=0; i<2; i++) {
        m = save_rxmn[i];
        h = SIDH(m);
        l = SIDL(m);
        printf("RXM%d RXM%dSIDH RXM%dSIDL: %4d %02Xh %02Xh\n", i, i, i, m, h, l);
    }
    for(i=0; i<6; i++) {
        m = save_rxfn[i];
        h = SIDH(m);
        l = SIDL(m);
        printf("RXF%d RXF%dSIDH RXF%dSIDL: %4d %02Xh %02Xh\n", i, i, i, m, h, l);
    }    
    printf("---\n");
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

    printf("TXB0CTRL: %02x\n", read_register(TXB0CTRL));
    printf("TXB1CTRL: %02x\n", read_register(TXB1CTRL));
    printf("TXB2CTRL: %02x\n", read_register(TXB2CTRL));
    printf("\n");

    printf("RXB0CTRL: %02x\n", read_register(RXB0CTRL));
    printf("RXB1CTRL: %02x\n", read_register(RXB1CTRL));
    printf("\n");
    
    printf("CANCTRL: %02x\n", read_register(CANCTRL));
    printf("CANSTAT: %02x\n", read_register(CANSTAT));
    printf("---\n");    
}
