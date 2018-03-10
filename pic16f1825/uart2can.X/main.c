#include "mcc_generated_files/mcc.h"
#include "stdlib.h"

#define LED LATCbits.LATC3

#define BUFSIZE 64

const uint8_t max_char = BUFSIZE - 1;

typedef struct {
    uint8_t sidh;
    uint8_t sidl;
} SID;  // TXBnSIDH and TXBnSIDL at MCP2515

uint8_t buf[BUFSIZE];
uint8_t c;
uint8_t cnt = 0;
SID sid;
    
/*
 * Set CAN message ID (11bit length)
 * 
 * SIDH: [SID10|SID9|SID8|SID7|SID6|SID5|SID4|SID3]
 * SIDL: [SID2|SID1|SID0| - |EXIDE| - |EID17|EID16]
 */
void set_sid (uint16_t can_node) {
    sid.sidh = (can_node >> 8) & 0x00ff;   // SID3 ~ SID10
    sid.sidl = (can_node << 5) & 0x00e0;   // SID0 ~ SID2 
}

/*
 * Send CAN message
 */
void can_send(uint8_t *buf, uint8_t cnt) {
}

void main(void)
{
    SYSTEM_Initialize();

    //INTERRUPT_GlobalInterruptEnable();
    //INTERRUPT_PeripheralInterruptEnable();
    
    set_sid(0);
    
    while (1)
    {
        if (EUSART_DataReady) {
            c = EUSART_Read();
            printf("%c", c);
            LED = !LED;
            
            buf[cnt] = c;
            if (c == '\n') {
                buf[cnt] = '\0';
                if (buf[0] == '@') {
                    uint16_t sid = atoi(&buf[1]);
                    set_sid(atoi(sid));
                    printf("Message ID set: %d\n", sid);
                } else {
                    can_send(buf, cnt);
                    printf("CAN message sent: %s\n", buf);
                }
                cnt = 0;
            } else if (++cnt > max_char) {
                buf[cnt] = '\0';
                can_send(buf, cnt);
                printf("\nCAN message sent: %s\n", buf);
                cnt = 0;
            }
            
        }
        // Add your application code
    }
}
