# CAN adapter to UART

![daisy_chain](./doc/daisy_chain.jpg)

## Motivation

(1) I want to connect PIC-MCU-based sensor/actuator blocks to home/office controller(RasPi or OpenWrt) over CAN, since CAN is very cheap and supports daisy-chain network topology.

(2) I already developed I2C-based network in [this "sensor-network" project](https://github.com/araobp/sensor-network), but I2C is for inter-board or inter-IC communications -- short-range (within 1 meter). I2C is master-slave, so it requires some scheduler to use its bandwidth efficiently. I developed such a scheduler in "sensor-network" project, but it made things complicated.

## Parts

Approximately, the total cost is 500 yen ( < five dollors) per board.

- Microchip PIC16F1825 (150 yen)
- Microchip MCP2515 (160 yen)
- Microchip MCP2561 (90 yen)
- Murata Ceralock 8MHz (35 yen)
- Resistor: 51k, 10k and 120 ohm (1 yen * 3)
- Capacitor: 0.1 micro F (10 yen * 3)
- IC sockets (10 yen * 3)
- Universal board (30yen)
- Pin headers, jumper pins etc.

## Specification

|Parameter           |Value                       |Note
|--------------------|----------------------------|---------------------------|
|PIC16F1825 MCU clock|32MHz(Internal OSC 8MHz * 4 PLL)                        |
|UART baud rate      |9600bps (fixed)             |Full-duplex wire-rate is not supported|
|SPI clock           |2MHz (fixed)                |                           |
|CAN speed           |Max. 250kHz (default 125kHz)|                           |
|Oscillator for CAN controller|8MHz (ceramic)     |Murata Ceralock            |

## Development plan and progress

### Version 0.1 (completed on March 17, 2018)

![board](./doc/board.jpg)

- Purchased CAN adaptors from Amazon.
- Evaluate CAN ==> [EVALUATION](./doc/EVALUATION.md).
- Developed my original ASCII-based protocol over serial for MCP2515. The protocol is mainly for evaluating MCP2515.

### Version 0.2 plan

- Purchase three pairs of MCP2515 and MCP2561 in Akihabara.
- Develop my original CAN adaptor board.
- Develop CAN-MQTT gateway on RasPi ~~and OpenWrt~~.
- Develop a 3D-printed DIN rail enclosure for the board by using [FreeCAD](https://www.freecadweb.org/).
- ~~Add [SLCAN](https://elixir.bootlin.com/linux/v3.4/source/drivers/net/can/slcan.c) that is ASCII-based protocol for SocketCAN.~~

### Version 0.3 plan

~~- Purchase [a cheap ARM mbed board](http://akizukidenshi.com/catalog/g/gK-12144/).~~
~~- Develop CAN-MQTT gateway on mbed.~~

~~### Version 0.4 plan~~

- Use KiCAD to redesign the circuit for PIC16F1829 that support two MSSPs(i.e., two SPI/I2C interfaces).
- Adopt moduler architecture: base board and sensor/actuator building blocks.
- Support sensors and actuators: doppler sensor, CdS, servo motor etc.

## CAN adaptor to UART

### With PC/Android(USB-OTG)

```
[PC or Android]-USB-[FTDI]-UART-[PIC16F1825]-SPI-[MCP2515]-[TJA1050]-- CAN bus

```

### Use it as CAN library for PIC 16F1829 MCU

```
[PIC16F1XXX]-SPI-[MCP2515]-[TJA1050]-- CAN bus                   
```

## Development environment

Microchip [MPLAB-X IDE](http://www.microchip.com/mplab/mplab-x-ide) with MCC plugin.

## Command (UART/USB)

```
/// UART2CAN HELP (version 0.11  March 19, 2018) ///
[Set standard identifier] @i<standard identifier>
[Set output mode] {debug: @vd, verbose: @vv, normal: @vn}
[Enable operation mode] {loopback: @ol, normal: @on}
     with SID: @ols or @ons
[Set mask] @m<n><mask(SID10 ~ SID0)>
[Set filter] @f<n><filter(SID10 ~ SID0)>
[Set baud rate] @b<bpr>
[Abort all pending transmissions] @a
[Dump masks and filters] @F
[Dump registers] @D
[Send message] <message>
[Send message beginning with '@' character] @<@message>
[Receive message] <message> will be output
[Show this help]: @h
```

## Mask and filters

For example, to receive messages with SID 5, 10 and 15:
```
@m02047  --> RXM0 0b11111111111
@f05     --> RXF0 0b00000000101 (SID 5 message to RXB0)
@f110    --> RXF1 0b00000001010 (SID 10 message to RXB0)
@m12047  --> RXM1 0b11111111111
@f215    --> RXF2 0b00000001111 (SID 15 message to RXB1)
```

## Standard Identifier format

This implementation supports CAN Standard Frame only (does not support Extended Frame). For home networking, 11bit Standard Identifier suffices.

=> [FORMAT](./doc/FORMAT.md)

## CAN adaptor board

At first, I am going to use [this universal board](http://akizukidenshi.com/catalog/g/gP-08241/) to make a prototype of the CAN adaptor:

![universal board](./doc/universal_board.jpg)

I have finished the first prototype (Marth 25-27, 2018). I have made three boards, and it took eight hours.

![front](./doc/universal_board_front.jpg)

![back](./doc/universal_board_back.jpg)

Later on, I am going to try [KiCAD](http://kicad-pcb.org/) to make my original printed board.

## Reference

### Datasheet (Microchip/NXP/Murata)

#### Microchip
- [PIC16F1825](http://ww1.microchip.com/downloads/en/DeviceDoc/41440A.pdf)
- [MCP2515](http://ww1.microchip.com/downloads/en/DeviceDoc/21801d.pdf)
- [MCP2561](http://ww1.microchip.com/downloads/en/DeviceDoc/20005167C.pdf)
- [MCP2515DM-BM](http://www.microchip.com/Developmenttools/ProductDetails.aspx?PartNO=MCP2515DM-BM)

#### NXP
- [TJA1050](https://www.nxp.com/docs/en/data-sheet/TJA1050.pdf)

#### Murata
- [Ceralock](https://www.murata.com/~/media/webrenewal/support/library/catalog/products/timingdevice/ceralock/p17e.ashx)
