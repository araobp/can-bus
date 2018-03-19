/*
 * UART-CAN bridge with Microchip MCP2515
 * 
 * MCP2515 data sheet: http://ww1.microchip.com/downloads/en/DeviceDoc/21801d.pdf
 * 
 * Version 0.1  March 17, 2018
 * Version 0.11  March 19, 2018
 */

#include "mcc_generated_files/mcc.h"
#include "stdlib.h"
#include "mcp2515.h"

#define VERSION "0.11  March 19, 2018"

#define LED LATCbits.LATC3
#define ON 1  // Note: this should be 0 and the circuit is not right. 
#define OFF 0

#define BUFSIZE 9
const uint8_t max_idx = BUFSIZE - 2;  // append '\0' at the tail

struct {
    bool with_sid;
    bool hex_output;
} output_format;

/*
 * Receive message from CAN bus
 */
void receive_handler(uint8_t sid, uint8_t *pbuf, uint8_t dlc) {
    uint8_t i;
    if (output_format.with_sid) {
        printf("%d,%s\n", sid, pbuf);            
    } else if (output_format.hex_output) {
        printf("%02x:", sid);
        for(i=0; i<dlc; i++) {
            printf(" %02x", pbuf[i]);
        }
        printf("\n");
    } else {
        printf("%s\n", pbuf);
    }
}

void main(void)
{
    LED = ON;
    
    uint8_t buf[BUFSIZE];  // UART read buffer
    bool echo_back = false;  // UART echo back
    uint8_t c, idx;

    uint8_t cmd, sid, n, bpr;
    uint16_t mask;

    SYSTEM_Initialize();

    //INTERRUPT_GlobalInterruptEnable();
    //INTERRUPT_PeripheralInterruptEnable();
    
    output_format.with_sid = false;
    output_format.hex_output = false;
    can_init(receive_handler);
    
    idx = 0;
    
    while (1)
    {
        bool status = can_status_check();
        if (status) {
            LED = OFF;  // Alarm off
        } else {
            LED = ON;  // Alaram on
        }
        if (EUSART_DataReady) {
            c = EUSART_Read();
            if (echo_back) printf("%c", c);  // echo back
            
            buf[idx] = c;
            if (c == '\n') {
                buf[idx] = '\0';
                if (buf[0] == '@') {  // command entered
                    cmd = buf[1];
                    switch(cmd) {
                        case 'i':  // Set standard identifier
                            sid = (uint8_t)atoi(&buf[2]);
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
                                output_format.with_sid = false;
                                output_format.hex_output = false;
                            } else if (buf[3] == 's' && buf[4] == '\0') { 
                                output_format.with_sid = true;
                                output_format.hex_output = false;
                            } else if (buf[3] == 'h' && buf[4] == '\0') {
                                output_format.with_sid = false;
                                output_format.hex_output = true;
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
                        case 'd':  // Dump register
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
                            printf("     with SID: @ols or @ons, in hex format: @olh or @onh\n");
                            printf("[Set mask] @m<n><mask(SID10 ~ SID0)>\n"); 
                            printf("[Set filter] @f<n><filter(SID10 ~ SID0)>\n");
                            printf("[Set baud rate] @b<bpr>\n");
                            printf("[Abort all pending transmissions] @a\n");
                            printf("[Dump masks and filters] @F\n");
                            printf("[Dump registers] @d\n");
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
                
            } else if (++idx > max_idx) {
                buf[idx] = '\0';
                can_send(buf, idx);
                idx = 0;
            }
        }
    }
}
