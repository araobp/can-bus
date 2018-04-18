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
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "Expansion board -- thermistor"
Date "2018-04-18"
Rev "v0.1"
Comp "https://github.com/araobp"
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L +5V #PWR?
U 1 1 5AD6958B
P 5150 1500
F 0 "#PWR?" H 5150 1350 50  0001 C CNN
F 1 "+5V" H 5150 1640 50  0000 C CNN
F 2 "" H 5150 1500 50  0001 C CNN
F 3 "" H 5150 1500 50  0001 C CNN
	1    5150 1500
	1    0    0    -1  
$EndComp
$Comp
L R R?
U 1 1 5AD695C2
P 5400 2500
F 0 "R?" V 5480 2500 50  0000 C CNN
F 1 "R" V 5400 2500 50  0000 C CNN
F 2 "" V 5330 2500 50  0001 C CNN
F 3 "" H 5400 2500 50  0001 C CNN
	1    5400 2500
	1    0    0    -1  
$EndComp
$Comp
L Thermistor TH?
U 1 1 5AD695FB
P 5400 3100
F 0 "TH?" V 5500 3150 50  0000 C CNN
F 1 "Thermistor" V 5300 3100 50  0000 C BNN
F 2 "" H 5400 3100 50  0001 C CNN
F 3 "" H 5400 3100 50  0001 C CNN
	1    5400 3100
	1    0    0    -1  
$EndComp
$Comp
L Conn_01x04_Male J?
U 1 1 5AD696BC
P 5500 1650
F 0 "J?" H 5500 1850 50  0000 C CNN
F 1 "Conn_01x04_Male" H 5500 1350 50  0000 C CNN
F 2 "" H 5500 1650 50  0001 C CNN
F 3 "" H 5500 1650 50  0001 C CNN
	1    5500 1650
	0    -1   1    0   
$EndComp
$Comp
L Conn_01x04_Male J?
U 1 1 5AD696F6
P 5600 3900
F 0 "J?" H 5600 4100 50  0000 C CNN
F 1 "Conn_01x04_Male" H 5600 3600 50  0000 C CNN
F 2 "" H 5600 3900 50  0001 C CNN
F 3 "" H 5600 3900 50  0001 C CNN
	1    5600 3900
	0    1    -1   0   
$EndComp
$Comp
L GND #PWR?
U 1 1 5AD6B029
P 5900 3550
F 0 "#PWR?" H 5900 3300 50  0001 C CNN
F 1 "GND" H 5900 3400 50  0000 C CNN
F 2 "" H 5900 3550 50  0001 C CNN
F 3 "" H 5900 3550 50  0001 C CNN
	1    5900 3550
	1    0    0    -1  
$EndComp
Wire Wire Line
	5500 1850 5500 2050
Wire Wire Line
	5500 2050 5900 2050
Wire Wire Line
	5900 2050 5900 3350
Wire Wire Line
	5900 3350 5900 3450
Wire Wire Line
	5900 3450 5900 3550
Wire Wire Line
	5150 1500 5150 1800
Wire Wire Line
	5150 1800 5150 2000
Wire Wire Line
	5150 2000 5400 2000
Wire Wire Line
	5400 1850 5400 2000
Wire Wire Line
	5400 2000 5400 2350
$Comp
L PWR_FLAG #FLG?
U 1 1 5AD6B0C5
P 4850 1650
F 0 "#FLG?" H 4850 1725 50  0001 C CNN
F 1 "PWR_FLAG" H 4850 1800 50  0000 C CNN
F 2 "" H 4850 1650 50  0001 C CNN
F 3 "" H 4850 1650 50  0001 C CNN
	1    4850 1650
	1    0    0    -1  
$EndComp
$Comp
L PWR_FLAG #FLG?
U 1 1 5AD6B0E1
P 6200 3250
F 0 "#FLG?" H 6200 3325 50  0001 C CNN
F 1 "PWR_FLAG" H 6200 3400 50  0000 C CNN
F 2 "" H 6200 3250 50  0001 C CNN
F 3 "" H 6200 3250 50  0001 C CNN
	1    6200 3250
	1    0    0    -1  
$EndComp
Wire Wire Line
	4850 1650 4850 1800
Wire Wire Line
	4850 1800 5150 1800
Connection ~ 5150 1800
Wire Wire Line
	5900 3350 6200 3350
Wire Wire Line
	6200 3250 6200 3350
Connection ~ 5900 3350
Connection ~ 5400 2000
Wire Wire Line
	5400 2650 5400 2800
Wire Wire Line
	5400 2800 5400 2900
Wire Wire Line
	5700 1850 5700 2800
Wire Wire Line
	5700 2800 5400 2800
Connection ~ 5400 2800
Wire Wire Line
	5400 3300 5400 3450
Wire Wire Line
	5400 3450 5900 3450
Connection ~ 5900 3450
$EndSCHEMATC
