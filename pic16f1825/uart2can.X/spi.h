/* 
 * Created on 2018/03/15, 8:19
 */

#ifndef SPI_H
#define	SPI_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>
#include<xc.h>

#define CS_PIN LATAbits.LATA2

/**
 * SPI data exchange
 * @param buf
 * @param len
 * @return bytes written
 */
uint8_t SPI_exchange(uint8_t *buf, uint8_t len);

#ifdef	__cplusplus
}
#endif

#endif	/* SPI_H */

