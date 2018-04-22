EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:switches
LIBS:relays
LIBS:motors
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:can-bus
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Pyroelectric_sensor U1
U 1 1 5ADD148B
P 5850 3600
F 0 "U1" H 5400 3800 60  0000 C CNN
F 1 "Pyroelectric_sensor" H 5850 3200 60  0000 C CNN
F 2 "" H 5950 4250 60  0001 C CNN
F 3 "" H 5950 4250 60  0001 C CNN
	1    5850 3600
	1    0    0    -1  
$EndComp
$Comp
L Conn_01x04_Male J1
U 1 1 5ADD154A
P 5700 2100
F 0 "J1" H 5700 2300 50  0000 C CNN
F 1 "Conn_01x04_Male" H 5700 1800 50  0000 C CNN
F 2 "" H 5700 2100 50  0001 C CNN
F 3 "" H 5700 2100 50  0001 C CNN
	1    5700 2100
	0    -1   1    0   
$EndComp
$Comp
L Conn_01x04_Male J2
U 1 1 5ADD1580
P 5800 4850
F 0 "J2" H 5800 5050 50  0000 C CNN
F 1 "Conn_01x04_Male" H 5800 4550 50  0000 C CNN
F 2 "" H 5800 4850 50  0001 C CNN
F 3 "" H 5800 4850 50  0001 C CNN
	1    5800 4850
	0    1    -1   0   
$EndComp
Wire Wire Line
	5600 2300 5600 3150
Wire Wire Line
	5700 2300 5700 2850
Wire Wire Line
	5700 2850 6100 2850
Wire Wire Line
	6100 2850 6100 3150
Wire Wire Line
	5850 3150 5850 3050
Wire Wire Line
	5850 3050 6350 3050
Wire Wire Line
	6350 3050 6350 4300
Wire Wire Line
	6350 4300 5800 4300
Wire Wire Line
	5800 4300 5800 4650
NoConn ~ 5600 4650
NoConn ~ 5700 4650
NoConn ~ 5900 4650
NoConn ~ 5800 2300
NoConn ~ 5900 2300
$Comp
L +5V #PWR01
U 1 1 5ADD19F9
P 5300 2150
F 0 "#PWR01" H 5300 2000 50  0001 C CNN
F 1 "+5V" H 5300 2290 50  0000 C CNN
F 2 "" H 5300 2150 50  0001 C CNN
F 3 "" H 5300 2150 50  0001 C CNN
	1    5300 2150
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR02
U 1 1 5ADD1A13
P 6550 2850
F 0 "#PWR02" H 6550 2600 50  0001 C CNN
F 1 "GND" H 6550 2700 50  0000 C CNN
F 2 "" H 6550 2850 50  0001 C CNN
F 3 "" H 6550 2850 50  0001 C CNN
	1    6550 2850
	1    0    0    -1  
$EndComp
Wire Wire Line
	5300 2150 5300 2600
Wire Wire Line
	5300 2600 5600 2600
Connection ~ 5600 2600
Wire Wire Line
	5700 2600 6550 2600
Wire Wire Line
	6550 2600 6550 2850
Connection ~ 5700 2600
$Comp
L PWR_FLAG #FLG03
U 1 1 5ADD1A54
P 4950 2400
F 0 "#FLG03" H 4950 2475 50  0001 C CNN
F 1 "PWR_FLAG" H 4950 2550 50  0000 C CNN
F 2 "" H 4950 2400 50  0001 C CNN
F 3 "" H 4950 2400 50  0001 C CNN
	1    4950 2400
	1    0    0    -1  
$EndComp
$Comp
L PWR_FLAG #FLG04
U 1 1 5ADD1A6E
P 6850 2500
F 0 "#FLG04" H 6850 2575 50  0001 C CNN
F 1 "PWR_FLAG" H 6850 2650 50  0000 C CNN
F 2 "" H 6850 2500 50  0001 C CNN
F 3 "" H 6850 2500 50  0001 C CNN
	1    6850 2500
	1    0    0    -1  
$EndComp
Wire Wire Line
	4950 2400 4950 2500
Wire Wire Line
	4950 2500 5300 2500
Connection ~ 5300 2500
Wire Wire Line
	6550 2700 6850 2700
Wire Wire Line
	6850 2700 6850 2500
Connection ~ 6550 2700
$EndSCHEMATC
