# CAN bus evaluation

## Motivation

I evaluated CAN on mbed LPC1768 boards with CAN tranceivers(MCP2561), but LPC1768 is expensive and some of libraries on mbed are unstable.

I want to use CAN with Microchip PIC MCUs and CAN controller(MCP2515) to see if the combination works OK.

## Configuration

``` 
                                          ---+------+-----+--- CAN bus
                                             |      |     |
[PC]-USB/UART-[PIC16F1829]-SPI-[MCP2515]-[MCP2561]| | [MCP2561]-[MCP2515]-SPI-[PIC16F1829]-USB/UART-[PC]
                                                    |
                                                [MCP2561]-[MCP2515]-SPI-[PIC16F1829]-USB/UART-[PC]                                                    
```
