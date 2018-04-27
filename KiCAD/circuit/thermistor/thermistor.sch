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
LIBS:thermistor-cache
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
L +5V #PWR01
U 1 1 5AD6958B
P 5200 2550
F 0 "#PWR01" H 5200 2400 50  0001 C CNN
F 1 "+5V" H 5200 2690 50  0000 C CNN
F 2 "" H 5200 2550 50  0001 C CNN
F 3 "" H 5200 2550 50  0001 C CNN
	1    5200 2550
	1    0    0    -1  
$EndComp
$Comp
L R R1
U 1 1 5AD695C2
P 5450 3550
F 0 "R1" V 5530 3550 50  0000 C CNN
F 1 "R" V 5450 3550 50  0000 C CNN
F 2 "can-bus:Resistor_FaithfulLink_CF25" V 5380 3550 50  0001 C CNN
F 3 "" H 5450 3550 50  0001 C CNN
	1    5450 3550
	1    0    0    -1  
$EndComp
$Comp
L Thermistor TH1
U 1 1 5AD695FB
P 5450 4150
F 0 "TH1" V 5550 4200 50  0000 C CNN
F 1 "Thermistor" V 5350 4150 50  0000 C BNN
F 2 "can-bus:Thermistor" H 5450 4150 50  0001 C CNN
F 3 "" H 5450 4150 50  0001 C CNN
	1    5450 4150
	1    0    0    -1  
$EndComp
$Comp
L Conn_01x04_Male J1
U 1 1 5AD696BC
P 5550 2700
F 0 "J1" H 5550 2900 50  0000 C CNN
F 1 "Conn_01x04_Male" H 5550 2400 50  0000 C CNN
F 2 "can-bus:PinHeader_1x4" H 5550 2700 50  0001 C CNN
F 3 "" H 5550 2700 50  0001 C CNN
	1    5550 2700
	0    -1   1    0   
$EndComp
$Comp
L Conn_01x04_Male J2
U 1 1 5AD696F6
P 5650 4950
F 0 "J2" H 5650 5150 50  0000 C CNN
F 1 "Conn_01x04_Male" H 5650 4650 50  0000 C CNN
F 2 "can-bus:PinHeader_1x4" H 5650 4950 50  0001 C CNN
F 3 "" H 5650 4950 50  0001 C CNN
	1    5650 4950
	0    1    -1   0   
$EndComp
$Comp
L GND #PWR02
U 1 1 5AD6B029
P 5950 4600
F 0 "#PWR02" H 5950 4350 50  0001 C CNN
F 1 "GND" H 5950 4450 50  0000 C CNN
F 2 "" H 5950 4600 50  0001 C CNN
F 3 "" H 5950 4600 50  0001 C CNN
	1    5950 4600
	1    0    0    -1  
$EndComp
Wire Wire Line
	5550 2900 5550 3100
Wire Wire Line
	5550 3100 5950 3100
Wire Wire Line
	5950 3100 5950 4600
Wire Wire Line
	5200 2550 5200 3050
Wire Wire Line
	5200 3050 5450 3050
Wire Wire Line
	5450 2900 5450 3400
$Comp
L PWR_FLAG #FLG03
U 1 1 5AD6B0C5
P 4900 2700
F 0 "#FLG03" H 4900 2775 50  0001 C CNN
F 1 "PWR_FLAG" H 4900 2850 50  0000 C CNN
F 2 "" H 4900 2700 50  0001 C CNN
F 3 "" H 4900 2700 50  0001 C CNN
	1    4900 2700
	1    0    0    -1  
$EndComp
$Comp
L PWR_FLAG #FLG04
U 1 1 5AD6B0E1
P 6250 4300
F 0 "#FLG04" H 6250 4375 50  0001 C CNN
F 1 "PWR_FLAG" H 6250 4450 50  0000 C CNN
F 2 "" H 6250 4300 50  0001 C CNN
F 3 "" H 6250 4300 50  0001 C CNN
	1    6250 4300
	1    0    0    -1  
$EndComp
Wire Wire Line
	4900 2700 4900 2850
Wire Wire Line
	4900 2850 5200 2850
Connection ~ 5200 2850
Wire Wire Line
	5950 4400 6250 4400
Wire Wire Line
	6250 4400 6250 4300
Connection ~ 5950 4400
Connection ~ 5450 3050
Wire Wire Line
	5450 3700 5450 3950
Wire Wire Line
	5750 2900 5750 3850
Wire Wire Line
	5750 3850 5450 3850
Connection ~ 5450 3850
Wire Wire Line
	5450 4350 5450 4500
Wire Wire Line
	5450 4500 5950 4500
Connection ~ 5950 4500
NoConn ~ 5650 2900
NoConn ~ 5450 4750
NoConn ~ 5550 4750
NoConn ~ 5650 4750
NoConn ~ 5750 4750
$EndSCHEMATC
