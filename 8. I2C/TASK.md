1. Read pin outs of various I2C devices / break out boards 	- DONE
2. Hook up AVRs with DS1338 & SL018 						- DONE
3. Check can flash code 									- DONE
3a Copy example (Arduino) code for DS1338 					- DONE
3b Copy example (AVR) code for SL018 						- DONE
3c Handwrite AVR pure code for DS1338 						- DONE
4. Get basic result working with RTC 						- DONE
5. Use the arduino MPU 6050 code to get this board working basic. - DONE :)
6.

Overcome issues:
================
Q. Why wont MPUs responsd (hang on 1st read byte)? Address is correct?
NO - seems people / datasheets quote 7bit address. So we need to << 1 before adding the R/W bit.
0x68 --> 0xd0


- Read MPU6050 manual - DONE
For both Gyro and Accel sensors can change :
i) Full range
ii) sensitivity
? Really or is that connected?

8000Hz is the max data rate from sensors.
I2C can dump all registers at 400kHz

What setting do MWC have?

char is signed!!!!!!!!!!
