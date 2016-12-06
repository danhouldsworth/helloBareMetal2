/**************************************************************************************/
/***************             Global RX related variables           ********************/
/**************************************************************************************/
#define SPEK_MAX_CHANNEL 7
#define SPEK_FRAME_SIZE 16

//RAW RC values will be store here
volatile uint16_t rcValue[8] = {1502, 1502, 1502, 1502, 1502, 1502, 1502, 1502}; // interval [1000;2000]

static uint8_t rcChannel[8]  = {ROLLPIN, PITCHPIN, YAWPIN, THROTTLEPIN, AUX1PIN,AUX2PIN,AUX3PIN,AUX4PIN};

static uint8_t PCInt_RX_Pins[PCINT_PIN_COUNT] = {PCINT_RX_BITS}; // if this slowes the PCINT readings we can switch to a define for each pcint bit

volatile uint8_t spekFrame[SPEK_FRAME_SIZE];

static uint8_t SPEK_CHAN_SHIFT = 2;
static uint8_t SPEK_CHAN_MASK = 0x03;

/**************************************************************************************/
/***************                   RX Pin Setup                    ********************/
/**************************************************************************************/
void configureReceiver() {
  if(conf.RxType == 2){
    SPEK_CHAN_SHIFT = 3;
    SPEK_CHAN_MASK = 0x07;
  }
  
  if(conf.RxType == 3){
    rcChannel[0] = PITCH;
    rcChannel[1] = YAW;
    rcChannel[2] = THROTTLE;
    rcChannel[3] = ROLL;
    rcChannel[4] = AUX1;
    rcChannel[5] = AUX2;
    rcChannel[6] = AUX3;
    rcChannel[7] = AUX4;
  }
  if(conf.RxType == 4){
    rcChannel[0] = ROLL;
    rcChannel[1] = PITCH;
    rcChannel[2] = THROTTLE;
    rcChannel[3] = YAW;
    rcChannel[4] = AUX1;
    rcChannel[5] = AUX2;
    rcChannel[6] = AUX3;
    rcChannel[7] = AUX4;
  }
  if(conf.RxType == 5){
    rcChannel[0] = PITCH;
    rcChannel[1] = ROLL;
    rcChannel[2] = THROTTLE;
    rcChannel[3] = YAW;
    rcChannel[4] = AUX1;
    rcChannel[5] = AUX2;
    rcChannel[6] = AUX3;
    rcChannel[7] = AUX4;
  }
  
  
  
  /******************    Configure each rc pin for PCINT    ***************************/
  if(conf.RxType == 0){
    // PCINT activation
    for(uint8_t i = 0; i < PCINT_PIN_COUNT; i++){ // i think a for loop is ok for the init.
      PCINT_RX_PORT |= PCInt_RX_Pins[i];
      PCINT_RX_MASK |= PCInt_RX_Pins[i];
    }
    PCICR = PCIR_PORT_BIT;
    
    
    /***************   atmega32u4's Specific RX Pin Setup   **********************/
      //Trottle on pin 7
      DDRE &= ~(1 << 6); // pin 7 to input
      PORTE |= (1 << 6); // enable pullups
      EIMSK |= (1 << INT6); // enable interuppt
      EICRB |= (1 << ISC60);
      
      DDRB &= ~(1 << 0); // set D17 to input 
  }  
  /*************************   Special RX Setup   ********************************/

  // Init PPM SUM RX
  if(conf.RxType > 2){
    PPM_PIN_INTERRUPT; 
  }
  // Init Sektrum Satellite RX
  if(conf.RxType == 1 || conf.RxType == 2){
    SerialOpen(1,115200);
  }
}

/**************************************************************************************/
/***************               Standard RX Pins reading            ********************/
/**************************************************************************************/
  // predefined PC pin block (thanks to lianj)
  #define RX_PIN_CHECK(pin_pos, rc_value_pos)                                                        \
    if (mask & PCInt_RX_Pins[pin_pos]) {                                                             \
      if (!(pin & PCInt_RX_Pins[pin_pos])) {                                                         \
        dTime = cTime-edgeTime[pin_pos]; if (900<dTime && dTime<2200) rcValue[rc_value_pos] = dTime; \
      } else edgeTime[pin_pos] = cTime;                                                              \
    }
  // port change Interrupt
  ISR(RX_PC_INTERRUPT) { //this ISR is common to every receiver channel, it is call everytime a change state occurs on a RX input pin
    uint8_t mask;
    uint8_t pin;
    uint16_t cTime,dTime;
    static uint16_t edgeTime[8];
    static uint8_t PCintLast;
  
    pin = RX_PCINT_PIN_PORT; // RX_PCINT_PIN_PORT indicates the state of each PIN for the arduino port dealing with Ports digital pins
   
    mask = pin ^ PCintLast;   // doing a ^ between the current interruption and the last one indicates wich pin changed
    sei();                    // re enable other interrupts at this point, the rest of this interrupt is not so time critical and can be interrupted safely
    PCintLast = pin;          // we memorize the current state of all PINs [D0-D7]
  
    cTime = micros();         // micros() return a uint32_t, but it is not usefull to keep the whole bits => we keep only 16 bits
    
    #if (PCINT_PIN_COUNT > 0)
      RX_PIN_CHECK(0,2);
    #endif
    #if (PCINT_PIN_COUNT > 1)
      RX_PIN_CHECK(1,4);
    #endif
    #if (PCINT_PIN_COUNT > 2)
      RX_PIN_CHECK(2,5);
    #endif
    #if (PCINT_PIN_COUNT > 3)
      RX_PIN_CHECK(3,6);
    #endif
    #if (PCINT_PIN_COUNT > 4)
      RX_PIN_CHECK(4,7);
    #endif
    #if (PCINT_PIN_COUNT > 5)
      RX_PIN_CHECK(5,0);
    #endif
    #if (PCINT_PIN_COUNT > 6)
      RX_PIN_CHECK(6,1);
    #endif
    #if (PCINT_PIN_COUNT > 7)
      RX_PIN_CHECK(7,3);
    #endif
  }
  
/****************      atmega32u4's Throttle & PPM Pin      *******************/
// throttle || PPM 
ISR(INT6_vect){ 
  static uint16_t now,diff;
  static uint16_t last = 0;
  if(conf.RxType > 2){
    rxInt();
  }else{
    now = micros();  
    if(!(PINE & (1<<6))){
	diff = now - last;
      if(900<diff && diff<2200){
        rcValue[3] = diff;
        #if defined(FAILSAFE)
          if(failsafeCnt > 20) failsafeCnt -= 20; else failsafeCnt = 0;   // If pulse present on THROTTLE pin (independent from ardu version), clear FailSafe counter  - added by MIS
        #endif 
      }
    }else last = now; 
  }
  failsave = 0;
}


/**************************************************************************************/
/***************                PPM SUM RX Pin reading             ********************/
/**************************************************************************************/
// attachInterrupt fix for promicro

// Read PPM SUM RX Data
void rxInt() {
  uint16_t now,diff;
  static uint16_t last = 0;
  static uint8_t chan = 0;

  now = micros();
  diff = now - last;
  last = now;
  if(diff>3000) chan = 0;
  else {
    if(900<diff && diff<2200 && chan<8 ) {   //Only if the signal is between these values it is valid, otherwise the failsafe counter should move up
      rcValue[chan] = diff;
    }
  chan++;
  }
}


/**************************************************************************************/
/***************            Spektrum Satellite RX Data             ********************/
/**************************************************************************************/
void SpektrumISR() {
  uint32_t spekTime;
  static uint32_t spekTimeLast, spekTimeInterval;
  static uint8_t  spekFramePosition;
  spekTime=micros();
  spekTimeInterval = spekTime - spekTimeLast;
  spekTimeLast = spekTime;
  if (spekTimeInterval > 5000) spekFramePosition = 0;
  spekFrame[spekFramePosition] = SPEK_DATA_REG;
  if (spekFramePosition == SPEK_FRAME_SIZE - 1) {
    rcFrameComplete = 1;
  } else {
    spekFramePosition++;
  }
  failsave = 0;
}

/**************************************************************************************/
/***************          combine and sort the RX Datas            ********************/
/**************************************************************************************/
uint16_t readRawRC(uint8_t chan) {
  static uint32_t spekChannelData[SPEK_MAX_CHANNEL];
  uint16_t data;
  uint8_t oldSREG;
  oldSREG = SREG; cli(); // Let's disable interrupts
  data = rcValue[rcChannel[chan]]; // Let's copy the data Atomically
  if(conf.RxType == 1 || conf.RxType == 2){
    if (rcFrameComplete) {
      for (uint8_t b = 3; b < SPEK_FRAME_SIZE; b += 2) {
        uint8_t spekChannel = 0x0F & (spekFrame[b - 1] >> SPEK_CHAN_SHIFT);
        if (spekChannel < SPEK_MAX_CHANNEL) spekChannelData[spekChannel] = ((uint32_t)(spekFrame[b - 1] & SPEK_CHAN_MASK) << 8) + spekFrame[b];
      }
      rcFrameComplete = 0;
    }
  }
  SREG = oldSREG; sei();// Let's enable the interrupts
  if(conf.RxType == 1 || conf.RxType == 2){
    static uint8_t spekRcChannelMap[SPEK_MAX_CHANNEL] = {1,2,3,0,4,5,6};
    if (chan >= SPEK_MAX_CHANNEL) {
      data = 1500;
    } else {
      if(conf.RxType == 1){
        data = 988 + spekChannelData[spekRcChannelMap[chan]];          // 1024 mode
      }
      if(conf.RxType == 2){
        data = 988 + (spekChannelData[spekRcChannelMap[chan]] >> 1);   // 2048 mode
      }
    }
  }
  return data; // We return the value correctly copied when the IRQ's where disabled
}

/**************************************************************************************/
/***************          compute and Filter the RX data           ********************/
/**************************************************************************************/
void computeRC() {
  static int16_t rcData4Values[8][4], rcDataMean[8];
  static uint8_t rc4ValuesIndex = 0;
  uint8_t chan,a;
    rc4ValuesIndex++;
    for (chan = 0; chan < 8; chan++) {
      rcData4Values[chan][rc4ValuesIndex%4] = readRawRC(chan);
      rcDataMean[chan] = 0;
      for (a=0;a<4;a++) rcDataMean[chan] += rcData4Values[chan][a];
      rcDataMean[chan]= (rcDataMean[chan]+2)/4;
      if ( rcDataMean[chan] < rcData[chan] -3)  rcData[chan] = rcDataMean[chan]+2;
      if ( rcDataMean[chan] > rcData[chan] +3)  rcData[chan] = rcDataMean[chan]-2;
    }
    if(conf.RxType == 1 || conf.RxType == 2){
      for (chan = 0; chan < 8; chan++) {
        if(rcData[chan] <= 900){
          
        }
      }
    }
    
}

