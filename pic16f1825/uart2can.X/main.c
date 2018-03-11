#include "mcc_generated_files/mcc.h"
#include "stdlib.h"

#define LED LATCbits.LATC3

#define BUFSIZE 64

const uint8_t max_char = BUFSIZE - 1;

#define TRANSMIT 0b00001011  // TXBnCTRL transmit request with highest priority


/*
 *  TXBnSIDH and TXBnSIDL on MCP2515
 */
typedef struct {
    uint8_t sidh;
    uint8_t sidl;
} SID;

uint8_t buf[BUFSIZE];
uint8_t c;
uint8_t cnt = 0;
SID sid;

#define TXB0CTRL 0x30
#define CANCTRL 0x0f
#define NORMAL_MODE 0x00
#define REGISTER_WRITE 0b00000010

const uint8_t txb_n_ctrl[3] = {TXB0CTRL, TXB0CTRL+0x10, TXB0CTRL+0x20};

/*
 *  n: 0 ~ 3
 * TXBnCTRL, TXBnSDIH, TXBnSDIL, TXBnEID8, TXBnEID0, TXBnDLC, TXBnDm (m: 0 ~ 7)
 */
uint8_t tx_buf[15];

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

/*
 * Set MCP2515 to normal mode
 */
bool can_start(void) {
    uint8_t can_ctrl_buf[3] = {REGISTER_WRITE, CANCTRL, NORMAL_MODE}; 
    uint8_t bytes_written = SPI_Exchange8bitBuffer(can_ctrl_buf, 3, can_ctrl_buf);
    if (bytes_written == 3) {
        return true;
    } else {
        return false;
    }
}

/*
 * Send CAN message
 */
bool can_send(uint8_t n, uint8_t *buf, uint8_t cnt) {

    uint8_t i;
    tx_buf[0] = REGISTER_WRITE;
    tx_buf[1] = txb_n_ctrl[n] + 1;
    tx_buf[2] = sid.sidh;  // TXBnSIDH
    tx_buf[3] = sid.sidl;  // TXBnSIDL
    tx_buf[4] = 0;         // TXBnEID8
    tx_buf[5] = 0;         // TXBnEID0
    tx_buf[6] = cnt;       // TXBnDLC
    for(i=0; i++; i<cnt) {
        tx_buf[7+i] = buf[i];
    }
    uint8_t bytes_written = 0;
    uint8_t len = 7 + cnt;
    bytes_written = SPI_Exchange8bitBuffer(tx_buf, len, tx_buf);
    if (bytes_written == len) {
        uint8_t txb_ctrl_buf[3];
        txb_ctrl_buf[0] = REGISTER_WRITE;
        txb_ctrl_buf[1] = txb_n_ctrl[n];
        txb_ctrl_buf[2] = TRANSMIT; 
        bytes_written = SPI_Exchange8bitBuffer(txb_ctrl_buf, 3, txb_ctrl_buf);
        if (bytes_written == 3) {
            return true;
        }
    }
    return false;
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
        if (EUSART_DataReady) {
            c = EUSART_Read();
            printf("%c", c);
            LED = !LED;
            
            buf[cnt] = c;
            if (c == '\n') {
                buf[cnt] = '\0';
                if (buf[0] == '@') {  // SID
                    uint16_t sid = atoi(&buf[1]);
                    set_sid(atoi(sid));
                } else {
                    can_send(0, buf, cnt);
                    printf("CAN message sent: %s\n", buf);
                }
                cnt = 0;
            } else if (++cnt > max_char) {
                buf[cnt] = '\0';
                can_send(0, buf, cnt);
                printf("\nCAN message sent: %s\n", buf);
                cnt = 0;
            }
            
        }
    }
}
