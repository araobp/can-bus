/*
 * UART-CAN bridge with Microchip MCP2515
 * 
 * MCP2515 data sheet: http://ww1.microchip.com/downloads/en/DeviceDoc/21801d.pdf
 * 
 * Version 0.1  March 17, 2018
 * Version 0.11  March 19, 2018
 * Version 0.12  March 22, 2018
 */

#include "mcc_generated_files/mcc.h"
#include "stdlib.h"
#include "mcp2515.h"
#include "hexascii.h"
#include <stdbool.h>
#include <string.h>

#define VERSION "0.13  March 23, 2018"

#define LED LATCbits.LATC3
#define ON 1  // Note: this should be 0 (open drain). 
#define OFF 0

#define BUFSIZE 16
#define MAX_IDX 7  // append '\0' at the tail

bool with_sid;

struct {
    uint8_t buf[15];
    bool printing;
} receive_buf;

/*
 * Receive message from CAN bus
 */
void receive_handler(uint16_t sid, uint8_t *buf, uint8_t dlc) {
    uint16_t sid0, sid1, sid2, sid3, sida, sidb;
    if (receive_buf.printing) {
        // Discard the data, unfortunately
    } else {

        if (with_sid) {

            // Convert SID into ASCII char array
            sid0 = sid / 1000u;
            sida = sid % 1000u;
            sid1 = sida / 100u;
            sidb = sida % 100u;
            sid2 = sidb / 10u;
            sid3 = sidb % 10u;
            receive_buf.buf[0] = (uint8_t)(sid0 + 0x30u);
            receive_buf.buf[1] = (uint8_t)(sid1 + 0x30u);
            receive_buf.buf[2] = (uint8_t)(sid2 + 0x30u);
            receive_buf.buf[3] = (uint8_t)(sid3 + 0x30u);
            receive_buf.buf[4] = ',';

            memcpy(&receive_buf.buf[5], buf, dlc);
            receive_buf.buf[5+dlc] = '\n';
            receive_buf.printing = true;
        } else {
            memcpy(receive_buf.buf, buf, dlc);
            receive_buf.buf[dlc] = '\n';
            receive_buf.printing = true;
        }
    }
}

void main(void)
{
    LED = ON;
    
    uint8_t buf[BUFSIZE];  // UART read buffer
    bool echo_back = false;  // UART echo back
    uint8_t c, idx;

    uint8_t cmd, n, bpr;
    uint16_t sid, mask;
    
    uint8_t pos;
    uint8_t putc_data;
    
    __delay_ms(100);

    SYSTEM_Initialize();

    //INTERRUPT_GlobalInterruptEnable();
    //INTERRUPT_PeripheralInterruptEnable();
    
    pos = 0;
    receive_buf.printing = false;
    
    idx = 0;

    with_sid = false;
    can_init(receive_handler);
        
    while (1)
    {
        bool status = can_status_check();
        if (status) {
            LED = OFF;  // Alarm off
        } else {
            LED = ON;  // Alarm on
        }
        
        if (receive_buf.printing) {
            putc_data = receive_buf.buf[pos++];
            putch(putc_data);
            if (putc_data == '\n') {
                receive_buf.printing = false;
                pos = 0;
            }
        }
        
        if (EUSART_DataReady) {
            c = EUSART_Read();
            if (echo_back) printf("%c", c);  // echo back
            
            buf[idx] = c;
                  
            // Command parser
            if (c == '\n') {
                buf[idx] = '\0';
                if (buf[0] == '@') {  // command entered
                    cmd = buf[1];
                    switch(cmd) {
                        case 'i':  // Set standard identifier
                            sid = (uint16_t)atoi(&buf[2]);
                            can_set_sid(sid);
                            break;
                        case 'v':  // Set verbosity
                            if (buf[2] == 'd') {  //  debug mode
                                can_logging_mode(true, true);
                                echo_back = true;
                            } else if (buf[2] == 'v') {  //  verbose mode
                                can_logging_mode(false, true);
                                echo_back = true;
                            } else if (buf[2] == 'n') {  // normal mode
                                can_logging_mode(false, false);
                                echo_back = false;
                            }
                            break;
                        case 'o':  // Set operation mode
                            if (buf[2] == 'l') {
                                can_ope_mode(LOOPBACK_MODE);
                            } else if (buf[2] == 'n') {
                                can_ope_mode(NORMAL_MODE);                                
                            }
                            if (buf[3] == '\0') {
                                with_sid = false;
                            } else if (buf[3] == 's' && buf[4] == '\0') { 
                                with_sid = true;
                            }
                            break;
                        case 'b':  // Set Baud Rate Prescaller (BPR + 1)
                            can_abort();  // Abort before changing baud rate
                            bpr = (uint8_t)atoi(&buf[2]);
                            can_baudrate(bpr);
                            break;
                        case 'm':  // Set mask
                        case 'f':  // Set filter
                            n = buf[2] - 0x30u;
                            mask = (uint16_t)atoi(&buf[3]);
                            if (cmd == 'm') {
                                can_set_mask(SET_MASK, n, mask);
                            } else if (cmd == 'f') {
                                can_set_mask(SET_FILTER, n, mask);                                
                            }
                            break;
                        case 'a':  // Abort all pending transmissions
                            can_abort();
                            break;
                        case 'F':  // Dump masks and filters
                            can_dump_masks_and_filters();
                            break;
                        case 'D':  // Dump register
                            can_dump_registers();
                            break;
                        case '@':  // Data begining with '@' character
                            can_send(&buf[1], idx - 1u);
                            break;
                        case 'h':  // Show help
                            printf("/// UART2CAN HELP (version %s) ///\n", VERSION);
                            printf("[Set standard identifier] @i<standard identifier>\n");
                            printf("[Set output mode] {debug: @vd, verbose: @vv, normal: @vn}\n");
                            printf("[Enable operation mode] {loopback: @ol, normal: @on}\n");
                            printf("     with SID: @ols or @ons\n");
                            printf("[Set mask] @m<n><mask(SID10 ~ SID0)>\n"); 
                            printf("[Set filter] @f<n><filter(SID10 ~ SID0)>\n");
                            printf("[Set baud rate] @b<bpr>\n");
                            printf("[Abort all pending transmissions] @a\n");
                            printf("[Dump masks and filters] @F\n");
                            printf("[Dump registers] @D\n");
                            printf("[Send message] <message>\n");
                            printf("[Send message beginning with \'@\' character] @<@message>\n");
                            printf("[Receive message] <message> will be output\n");
                            printf("[Show this help]: @h\n");
                            break;
                    }
                } else {
                    can_send(buf, idx);
                }
                idx = 0;
                
            } else if (++idx > MAX_IDX) {
                buf[idx] = '\0';
                can_send(buf, idx);
                idx = 0;
            }
        }
    }
}
