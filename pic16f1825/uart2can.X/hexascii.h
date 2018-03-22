/* 
 * File:   hexascii.h
 * Author: shiny
 *
 * Created on 2018/03/22, 12:38
 */

#ifndef HEXASCII_H
#define	HEXASCII_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>

uint8_t atoui8(uint8_t ascii);
uint16_t atosid(uint8_t *ascii);

#ifdef	__cplusplus
}
#endif

#endif	/* HEXASCII_H */

