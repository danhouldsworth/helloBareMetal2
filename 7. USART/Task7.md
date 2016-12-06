==> Read the datasheet for USART contorl
==> Initiatlise and set for 115200

Send some bytes do our USART viewer

Poloul
======
White	5v
Black	0v
Purple	Rx
Grey 	Tx

OLED
----

27 	Tx
26	Rx


1284p
-----
35 	Tx


1) Can the Polulu be connected post OLED BOOT, and Screen boot and work
2) Can we flash an existing C code for USART and get working
3) How do we mod the above code to show up on Screen & OLED
4) What was the issue?

=> At baud 110 can see the Tx Pulsing the LED. True enough, chars with high content off ascii binary 1's leave led brighter (?~), than those with mainly 0's (@A) which clearly flash.

=> Driving the Poloul Rx line 5v / 0v does not crash the screen, or show a char. Still able to transmit.
=> Connecting it to the flash Tx0 line results in one invalid char. No crash
=> Rebooting the chip has no effect
=> Plugging in / out the Tx0 wire, creates more invalid chars.
=> Plugging OLED Rx to 5v/0v does not crash. Occasionally blank screen from moving cable is because is displays invalid chars as blanks.

1) Try 2x - helped
2) Try viewing OSCAL, then tweaking
3) Try tuning USART with OSCAL - no need

1) Add echo (char --> binary) on D4U to demonstrate UART Rx
2) Power the spek sat with 3.3v, and see if we can read it


http://forum.arduino.cc/index.php?topic=22266.0
void computeRC(){
    // if (satFrame[0] != 0x03) return;
// // uint8_t channel =0;
    for (uint8_t frameByte = 3; frameByte < 16; frameByte +=2){
        uint8_t channel = 0b111 & (satFrame[frameByte - 1] >> 3);
        // if (channel == 0){
        channelVals[channel] = ((uint32_t)(satFrame[frameByte - 1] & 0b111 ) << 8) + satFrame[frameByte];
        // }
    }

    // for (uint8_t channel = 0; channel < 8; channel++){
        // channelVals[channel] = satFrame[channel];
    // }
}


Screen doesn't support 76800
Careful flicking bits without reading first incase changed something else in other part program
Frame 	: 8 x 16bit chunks === 32bytes @ 22ms/frame OR 11ms/frame
Header 	: 00000011 00000001
ch1-7	: 00cccvvv vvvvvvvv 	[2-bit 0 header || 3-bit channel number || 11-bit value]

Throttle: 00000000 00000000 --> 00000111 11111111
Aileron	: 00001000 00000000 --> 00001111 11111111
Elevator: 00010000 00000000 --> 00010111 11111111
Rudder	: 00011000 00000000 --> 00011111 11111111
Gear	: 00100000 00000000 --> 00100111 11111111
Flap	: 00101000 00000000 --> 00101111 11111111
Ch 7	: 00110000 00000000 --> 00110111 11111111

Throttle: 0 - 2047 [+ 0x 2048]
Aileron : 0 - 2047 [+ 1x 2048]
Elevator: 0 - 2047 [+ 2x 2048]
Rudder	: 0 - 2047 [+ 3x 2048]
Gear	: 0 - 2047 [+ 4x 2048]
Flap 	: 0 - 2047 [+ 5x 2048]
Ch 7	: 0 - 2047 [+ 6x 2048]

name 		// frame order 	// byte label code
throttle 	// 0-1 			// HB: 00000vvv
aux2  		// 2-3 			// HB: 00110vvv
rudder 		// 4-5 			// HB: 00011vvv
gear 		// 5-6 			// HB: 00100vvv
elevator 	// 6-7 			// HB: 00010vvv
aux1(flaps)	// 7-8 			// HB: 00101vvv
ailieron	// 8-9 			// HB: 00001vvv

11ms DSMX 00000000 11111111
22ms DSMX 11100001 10100010

DSMX 11ms
=========
Tx reboot

Header Byte #2 Values:
(Estimate - cumulative missed frames?)
@ DSMX 22
00000000 / 0x00 /   0
00101101 / 0x2d /  45
01011010 / 0x5a /  90
10000111 / 0x87 / 135
10110100 / 0xb4 / 180
11100001 / 0xe1 / 225
11111111 / 0xff / 255

DSM2 22
00110000 48
01100000 96
10001100 140
10111001 185
11101001 233
11111111 255
11111011 ?
00101101 45
01011010 90
00101011 43



Rx reboot
00000000 (sometimes H2 is not)

Internal Mode, System value:

Header Byte #1 :
00000001 / 0x01 /   1 === 22ms 1024 DSM2
00010010 / 0x12 /  18 === 11ms 2048 DSM2
10100010 / 0xa2 / 162 === 22ms 2048 DSMX (AR6210)
10110010 / 0xb2 / 178 === 11ms 2048 DSMX (AR8000)
xxx..... : 000 == DSM2 / 101 == DSMX
......bb :  01 == 1024 /  10 == 2048
...s.... : 	 0 == 22ms /   1 == 11ms
....00..

01100010 0x62
01000010 0x42: 0 supplement frame?
01.00010 00000000
00010010 DSM2 11ms Correct!

Occasional bit errors?

*Orange has null header MSB(1st). LSB(2nd) stays constant for that connection (fade?). Fade = 0 on reboot of Sat (with Tx on)
*Each frame has alternate last (2 then 4) words = 0xFFFF
*Each valid word starts with 0 always
*Bit 16 === 0
*Bit 15 === Bit 14. Always alternate from previous fame

Servo data:
54321ddddddddddd
5===0
4===3 (alternate each servo word starting at 0)
    Spektrum not always. Discard if so, then reliably goes 0..1..0..1..0..1..0 Again discard if not
leaves 3-bit channel with 2 rules for discarding

Reliably have readings for channel numbered : 0-6

4==3 Holds for AR8000
Not strictly alternate across frame pairs
0..1..0..1..0
0..1..0..0..1..1

Then reliable as the others.
Still channels ccc :0-7 with quite a few repeated.


Multi-Wii
* Use 115,200 not 125,000
* Non-Blocking - interupt fires then reads all 16 bytes (but uses arduino serial - which puts in buffer)
* Discards header
* Detects delay, to indicate likely the start of new frame
* Can BIND! Uses 3 | 5 pulses to set DSM2 Internal 22ms | 11ms
* Simply >> 1 the 2048 precision.

BaseFlight
* As above, but includes bind 9 pulses for DSMX 11ms

Bench
* Cold start threw off clocks?
Q : Trim tabs - how much move?
