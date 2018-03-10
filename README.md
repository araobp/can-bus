# CAN bus evaluation

![waveform](./doc/waveform.jpg)

## Motivation

I evaluated CAN on mbed LPC1768 boards with CAN tranceivers(MCP2561), but LPC1768 is expensive for IoT prototyping.

I have ever used Microchip PIC1825 MCU for IoT prototyping a lot of times since 2016. Altough the MCU's functionality is very limited, it is very cheap and satisify most of requirements as MCU for IoT prototyping.

I want to try out the combination of PIC16F1825 and CAN controller(MCP2515)/tranceiver(MCP2551 or MCP2561) this time.

![board](./doc/board.jpg)

## Configuration

``` 
                                          ---+------+-----+--- CAN bus
                                             |      |     |
[PC]-USB/UART-[PIC16F1825]-SPI-[MCP2515]-[MCP2561]| | [MCP2561]-[MCP2515]-SPI-[PIC16F1825]-USB/UART-[PC]
                                                    |
                                                [MCP2561]-[MCP2515]-SPI-[PIC16F1825]-USB/UART-[PC]                                                    
```

## My original PIC16F1825 evaluation board

![pico](https://docs.google.com/drawings/d/e/2PACX-1vTHoT0TZIyVhAgkDVHyuWkc1-_6oFHT2mF53g2q36bgH_qxplkvvRIkJ3PqJBNuTZauhhMmSiemMoZO/pub?w=480&h=360)

## Datasheet

- [PIC16F1825](http://ww1.microchip.com/downloads/en/DeviceDoc/41440A.pdf)
- [MCP2515](http://ww1.microchip.com/downloads/en/DeviceDoc/21801d.pdf)
- [MCP2551](http://ww1.microchip.com/downloads/en/DeviceDoc/21667E.pdf)
- [MCP2561](http://ww1.microchip.com/downloads/en/DeviceDoc/20005167C.pdf)
