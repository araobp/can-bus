/*
 * UART-CAN bridge with Microchip MCP2515
 * 
 * MCP2515 data sheet: http://ww1.microchip.com/downloads/en/DeviceDoc/21801d.pdf
 */

#include "mcc_generated_files/mcc.h"
#include "stdlib.h"
#include "mcp2515.h"

#define LED LATCbits.LATC3

#define BUFSIZE 9
const uint8_t max_idx = BUFSIZE - 2;  // append '\0' at the tail

SID sid;

uint8_t buf[BUFSIZE];  // UART read buffer
uint8_t c;
uint8_t idx;

typedef struct {
    bool debug;
    bool verbose;
    uint8_t ope_mode;
    bool with_sid;
} MODE;

MODE mode;

/*
 * Set CAN message ID (11bit length)
 * 
 * SIDH: [SID10|SID9|SID8|SID7|SID6|SID5|SID4|SID3]
 * SIDL: [SID2|SID1|SID0| - |EXIDE| - |EID17|EID16]
 */
void set_sid (uint16_t can_node) {
    uint8_t sidh = (can_node >> 3) & 0x00ff;   // SID3 ~ SID10
    uint8_t sidl = (can_node << 5) & 0x00e0;   // SID0 ~ SID2
    sid.sidh = sidh;
    sid.sidl = sidl;
    if (mode.debug) printf("SID set: %02x %02x\n", sidh, sidl);
}

uint8_t SPI_exchange(uint8_t *buf, uint8_t len) {
    LATAbits.LATA4 = 0;  // Lower CS pin on MCP2515
    uint8_t bytes_written = SPI_Exchange8bitBuffer(buf, len, buf);
    LATAbits.LATA4 = 1;  // Raise CS pin on MCP2515
    return bytes_written;
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
 * P51 Clear transmit buffer empty interrupt flag
 */
bool can_txf_clear(uint8_t n) {
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
void can_receive(uint8_t n) {
    
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

    // P51
    // This is unnecessary? See P63
    /*
    uint8_t mask = 0b00000001 << n;  //RXnIF interrupt pending
    uint8_t can_int_flag[4];
    can_int_flag[0] = BIT_MODIFY;
    can_int_flag[1] = CANINTF;
    can_int_flag[2] = mask;
    can_int_flag[3] = 0x00;
    uint8_t bytes_written = SPI_exchange(can_int_flag, 4, can_int_flag);
    */
    
    // Output the received message from CAN bus
    uint8_t *rx_buf_ptr = &rx_buf[6];
    if (mode.verbose) {
        if (mode.with_sid) {
            printf("Message received: %d,%s\n", sid, rx_buf_ptr);        
        } else {
            printf("Message received: %s\n", rx_buf_ptr);
        }
    } else {
        if (mode.with_sid) {
            printf("%d,%s\n", sid, rx_buf_ptr);            
        } else {
            printf("%s\n", rx_buf_ptr);
        }
    }
    LED = !LED;
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
            LED = !LED;
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
        can_receive(0);
    } else if ((status & RX1IF_MASK) > 0) {
        if (mode.debug) printf("RX1IF is on\n");
        can_receive(1);
    } else if ((status & TX0IF_MASK) > 0) {
        can_txf_clear(0);
    } else if ((status & TX1IF_MASK) > 0) {
        can_txf_clear(1);        
    } else if ((status & TX2IF_MASK) > 0) {
        can_txf_clear(2);
    }
    return;
}

void main(void)
{
    SYSTEM_Initialize();

    //INTERRUPT_GlobalInterruptEnable();
    //INTERRUPT_PeripheralInterruptEnable();
    
    set_sid(0);
    
    mode.debug = false;
    mode.verbose = false;
    mode.ope_mode = NORMAL_MODE;
    can_ope_mode(NORMAL_MODE);
    mode.with_sid = false;
    
    idx = 0;
    
    while (1)
    {
        can_status_check();
        
        if (EUSART_DataReady) {
            c = EUSART_Read();
            if (mode.verbose) printf("%c", c);  // echo back
            //LED = !LED;
            
            buf[idx] = c;
            if (c == '\n') {
                buf[idx] = '\0';
                if (buf[0] == '@') {  // command entered
                    uint8_t cmd = buf[1];
                    uint8_t sid;
                    uint8_t n;
                    uint16_t mask;
                    uint8_t mask_sidh;
                    uint8_t mask_sidl;
                    uint8_t mask_buf[4];
                    switch(cmd) {
                        case 'i':  // Set standard identifier
                            sid = atoi(&buf[2]);
                            set_sid(sid);
                            break;
                        case 'v':  // Set verbosity
                            if (buf[2] == 'd') {  //  debug mode 
                                mode.debug = true;
                                mode.verbose = true;
                            } else if (buf[2] == 'v') {  //  verbose mode
                                mode.debug = false;
                                mode.verbose = true;
                            } else if (buf[2] == 'n') {  // normal mode
                                mode.debug = false;
                                mode.verbose = false;
                            }
                            break;
                        case 'o':  // Set operation mode
                            if (buf[2] == 'l') {
                                mode.ope_mode = LOOPBACK_MODE;
                                can_ope_mode(LOOPBACK_MODE);
                            } else if (buf[2] == 'n') {
                                mode.ope_mode = NORMAL_MODE;
                                can_ope_mode(NORMAL_MODE);
                            }
                            if (buf[3] != '\0') {
                                if (buf[3] == 'n') {
                                    mode.with_sid = false;
                                } else if (buf[3] == 's') {
                                    mode.with_sid = true;
                                }
                            }
                            break;
                        case 'm':  // Set mask
                        case 'f':  // Set filter
                            can_ope_mode(CONFIGURATION_MODE);  // P33
                            n = buf[2] - 0x30;
                            mask = atoi(&buf[3]);
                            mask_sidh = (uint8_t)((mask >> 3) & 0x00ff);
                            mask_sidl = (uint8_t)(((mask & 0x0007) << 5) & 0x00ff);
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
                            
                            can_ope_mode(mode.ope_mode);
                            break;
                        case 'h':  // Show help
                            printf("--- UART2CAN HELP ---\n");
                            printf("[Set standard identifier] @i<Standard Identifier>\n");
                            printf("[Set output mode] {debug: @vd, verbose: @vv, normal: @vn}\n");
                            printf("[Enable operation mode] {loopback: @ol, normal: @on}\n");
                            printf("[Set mask] @m<n><mask(SID10 ~ SID0)>\n"); 
                            printf("[Set filter] @f<n><filter(SID10 ~ SID0)>\n");
                            printf("[Send message] <message>\n");
                            printf("[Receive message] <message> will be output\n");
                            printf("[Show this help]: @h\n");
                            break;
                    }
                } else {
                    bool rc = can_send(buf, idx);
                    if (rc) {
                        if (mode.verbose) printf("Message sent: %s\n", buf);
                    } else {
                        if (mode.verbose) printf("Unable to send message\n");
                    }
                }
                idx = 0;
            } else if (++idx > max_idx) {
                buf[idx] = '\0';
                bool rc = can_send(buf, idx);
                if (rc) {
                    if (mode.verbose) printf("\nMessage sent: %s\n", buf);
                } else {
                    if (mode.verbose) printf("\nUnable to send message\n");                    
                }
                idx = 0;
            }
        }
    }
}
