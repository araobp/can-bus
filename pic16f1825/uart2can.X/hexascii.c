#include "hexascii.h"
#include <stdio.h>

/*
 * ASCII code 
 */
#define ZERO 0x30u
#define NINE 0x39u
#define CAPITAL_A 0x41u
#define CAPITAL_F 0x46u
#define SMALL_A 0x61u
#define SMALL_F 0x66u

/**
 * Ascii byte to uint8_t value
 * @param ascii
 * @return value in uint8_t
 */
uint8_t atoui8(uint8_t ascii) {
    uint8_t minus = 0;
    if (ascii >= ZERO && ascii <= NINE) {
        minus = ZERO;
    } else if (ascii >= CAPITAL_A && ascii <= CAPITAL_F) {
        minus = CAPITAL_A - 10;
    } else if (ascii >= SMALL_A && ascii <= SMALL_F) {
        minus = SMALL_A - 10;
                
    }
    return (uint8_t)(ascii - minus);
}

/**
 * Ascii SID to uint16_t value
 * @param ascii
 * @return SID in uint16_t 
 */
uint16_t atosid(uint8_t *ascii) 
{
    uint16_t a = (uint16_t)(atoui8(ascii[0]));
    uint16_t b = (uint16_t)(atoui8(ascii[1]));
    uint16_t c = (uint16_t)(atoui8(ascii[2]));
    uint16_t id = a * 0x100u + b * 0x10u + c;
    return id;
}
