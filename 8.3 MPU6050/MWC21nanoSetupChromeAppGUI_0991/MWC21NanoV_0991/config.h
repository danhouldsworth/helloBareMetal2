#define I2C_SPEED 100000L

/******                Serial com speed    *********************************/
/* This is the speed of the serial interface */
#define SERIAL_COM_SPEED 115200
/* This is the speed of the serial 1 interface.. if active you can use both 0 or 1 on the Mega or USB or serial 1 on a ATmega32u4 */
#define SERIAL1_COM_SPEED 115200

/* interleaving delay in micro seconds between 2 readings WMP/NK in a WMP+NK config
   if the ACC calibration time is very long (20 or 30s), try to increase this delay up to 4000
   it is relevent only for a conf with NK */
#define INTERLEAVING_DELAY 3000

/* when there is an error on I2C bus, we neutralize the values during a short time. expressed in microseconds
   it is relevent only for a conf with at least a WMP */
#define NEUTRALIZE_DELAY 100000



