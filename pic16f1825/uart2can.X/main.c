#include "mcc_generated_files/mcc.h"
#include "stdlib.h"

#define LED LATCbits.LATC3

#define BUFSIZE 9
const uint8_t max_char = BUFSIZE - 1;  // append '\0' at the tail

// P18 TXB0CTRL
#define TXB0CTRL 0x30
#define TRANSMIT 0b00001011  // TXBnCTRL transmit request with highest priority

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
#define TX0IF_MASK 0b00001000
#define TX1IF_MASK 0b00100000
#define TX2IF_MASK 0b10000000

/*
 *  TXBnSIDH and TXBnSIDL on MCP2515
 */
typedef struct {
    uint8_t sidh;
    uint8_t sidl;
} SID;

SID sid;

uint8_t buf[BUFSIZE];  // UART read buffer
uint8_t c;
uint8_t cnt = 0;

/*
 *  n: 0 ~ 2
 * TXBnSDIH, TXBnSDIL, TXBnEID8, TXBnEID0, TXBnDLC, TXBnDm (m: 0 ~ 7)
 */
uint8_t tx_buf[14];

/*
 * n: 0 ~ 1
 * RXBnSDIH, RXBnSDIL, RXBnEID8, RXBnEID0, RXBnDLC, RXBnDm (m: 0 ~ 7)
 */
uint8_t rx_buf[15];  // Append '\0' at the tail

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
    printf("SID set: %02x %02x\n", sidh, sidl);
}

uint8_t SPI_send(uint8_t *sbuf, uint8_t len, uint8_t *rbuf) {
    LATAbits.LATA4 = 0;  // Lower CS pin on MCP2515
    uint8_t bytes_written = SPI_Exchange8bitBuffer(sbuf, len, rbuf);
    LATAbits.LATA4 = 1;  // Raise CS pin on MCP2515
    return bytes_written;
}

/*
 * P58 Set MCP2515 to normal mode
 */
bool can_start(void) {
    // uint8_t can_ctrl_buf[3] = {WRITE, CANCTRL, NORMAL_MODE}; 
    uint8_t can_ctrl_buf[3] = {WRITE, CANCTRL, LOOPBACK_MODE}; 
    uint8_t bytes_written = SPI_send(can_ctrl_buf, 3, can_ctrl_buf);
    if (bytes_written == 3) {
        can_ctrl_buf[0] = READ;
        can_ctrl_buf[1] = CANSTAT;
        bytes_written = SPI_send(can_ctrl_buf, 3, can_ctrl_buf);
        printf("CANSTAT: %02x\n", can_ctrl_buf[2]);
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
    uint8_t bytes_written = SPI_send(can_int_flag, 4, can_int_flag);
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
    // P65
    rx_buf[0] = READ_RX_BUFFER + (nm[n] << 1);
    uint8_t bytes_read = SPI_send(rx_buf, 14, rx_buf);
    rx_buf[bytes_read] = '\0';
    
    /*** debug ***/
    printf("RXB0SIDH: %02x\n", rx_buf[1]);
    printf("RXB0SIDL: %02x\n", rx_buf[2]);    
    printf("RXB0DLC: %02x\n", rx_buf[5]);    
    
    // P51
    uint8_t mask = 0b00000001 << n;  //RXnIF interrupt pending
    uint8_t can_int_flag[4];
    can_int_flag[0] = BIT_MODIFY;
    can_int_flag[1] = CANINTF;
    can_int_flag[2] = mask;
    can_int_flag[3] = 0x00;
    uint8_t bytes_written = SPI_send(can_int_flag, 4, can_int_flag);
    
    // Output the received message from CAN bus
    printf("Message received: %s\n", rx_buf[6]);
}

/*
 * Send CAN message
 */
bool can_send(uint8_t n, uint8_t *buf, uint8_t cnt) {
    uint8_t i;
    
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
    tx_buf[5] = cnt;

    // Copy buffer
    for(i=0; i++; i<cnt) {
        tx_buf[6+i] = buf[i];
    }

    // P66 Load TX buffer
    uint8_t len = 6 + cnt;
    uint8_t bytes_written = SPI_send(tx_buf, len, tx_buf);

    //*** Debug ***
    uint8_t debug[3] = {READ, 0x31, 0x00};  //TXB0SIDH
    SPI_send(debug, 3, debug);
    printf("TXB0SIDH: %02x\n", debug[2]);
    uint8_t debug[3] = {READ, 0x32, 0x00};  //TXB0SIDL
    SPI_send(debug, 3, debug);
    printf("TXB0SIDL: %02x\n", debug[2]);
    
    if (bytes_written == len) {
        // P64, P66 RTS
        uint8_t txb_ctrl_buf[1];
        txb_ctrl_buf[0] = RTS + (0b00000001 << n);
        bytes_written = SPI_send(txb_ctrl_buf, 1, txb_ctrl_buf);
        if (bytes_written == 1) {
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
    uint8_t bytes_written = SPI_send(can_status_buf, 2, can_status_buf);
    uint8_t status = can_status_buf[1];
    if (status == 0x00) {
        return;
    } else if ((status & RX0IF_MASK) > 0) {
        printf("RX0IF is on\n");
        can_receive(0);
    } else if ((status & RX1IF_MASK) > 0) {
        printf("RX1IF is on\n");
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
    can_start();
    
    while (1)
    {
        can_status_check();
        
        if (EUSART_DataReady) {
            c = EUSART_Read();
            printf("%c", c);
            //LED = !LED;
            
            buf[cnt] = c;
            if (c == '\n') {
                buf[cnt] = '\0';
                if (buf[0] == '@') {  // SID
                    uint16_t sid = atoi(&buf[1]);
                    set_sid(sid);
                } else {
                    bool rc = can_send(0, buf, cnt);
                    if (rc) {
                        printf("CAN message sent: %s\n", buf);
                    } else {
                        printf("Unable to send message\n");
                    }
                }
                cnt = 0;
            } else if (++cnt > max_char) {
                buf[cnt] = '\0';
                bool rc = can_send(0, buf, cnt);
                if (rc) {
                    printf("\nCAN message sent: %s\n", buf);
                } else {
                        printf("\nUnable to send message\n");                    
                }
                cnt = 0;
            }
            
        }
    }
}
