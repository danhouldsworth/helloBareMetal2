#include <avr/wdt.h>
#define UART_NUMBER 2
#define RX_BUFFER_SIZE 64
#define TX_BUFFER_SIZE 128
#define INBUF_SIZE 64

static volatile uint8_t serialHeadRX[UART_NUMBER],serialTailRX[UART_NUMBER];
static uint8_t serialBufferRX[RX_BUFFER_SIZE][UART_NUMBER];
static volatile uint8_t headTX,tailTX;
static uint8_t bufTX[TX_BUFFER_SIZE];
static uint8_t inBuf[INBUF_SIZE];
uint8_t PWM_PIN[8] = {9,10,5,6,11,13};   //for a quad+: rear,right,left,front

#define MSP_VERSION              0

//to multiwii developpers/committers : do not add new MSP messages without a proper argumentation/agreement on the forum
#define MSP_IDENT                100   //out message         multitype + multiwii version + protocol version + capability variable
#define MSP_STATUS               101   //out message         cycletime & errors_count & sensor present & box activation & current setting number
#define MSP_RAW_IMU              102   //out message         9 DOF
#define MSP_SERVO                103   //out message         8 servos
#define MSP_MOTOR                104   //out message         8 motors
#define MSP_RC                   105   //out message         8 rc chan and more
#define MSP_RAW_GPS              106   //out message         fix, numsat, lat, lon, alt, speed, ground course
#define MSP_COMP_GPS             107   //out message         distance home, direction home
#define MSP_ATTITUDE             108   //out message         2 angles 1 heading
#define MSP_ALTITUDE             109   //out message         altitude, variometer
#define MSP_ANALOG               110   //out message         vbat, powermetersum, rssi if available on RX
#define MSP_RC_TUNING            111   //out message         rc rate, rc expo, rollpitch rate, yaw rate, dyn throttle PID
#define MSP_PID                  112   //out message         P I D coeff (9 are used currently)
#define MSP_BOX                  113   //out message         BOX setup (number is dependant of your setup)
#define MSP_MISC                 114   //out message         powermeter trig
#define MSP_MOTOR_PINS           115   //out message         which pins are in use for motors & servos, for GUI 
#define MSP_BOXNAMES             116   //out message         the aux switch names
#define MSP_PIDNAMES             117   //out message         the PID names
#define MSP_WP                   118   //out message         get a WP, WP# is in the payload, returns (WP#, lat, lon, alt, flags) WP#0-home, WP#16-poshold
#define MSP_BOXIDS               119   //out message         get the permanent IDs associated to BOXes

#define MSP_SET_RAW_RC           200   //in message          8 rc chan
#define MSP_SET_RAW_GPS          201   //in message          fix, numsat, lat, lon, alt, speed
#define MSP_SET_PID              202   //in message          P I D coeff (9 are used currently)
#define MSP_SET_BOX              203   //in message          BOX setup (number is dependant of your setup)
#define MSP_SET_RC_TUNING        204   //in message          rc rate, rc expo, rollpitch rate, yaw rate, dyn throttle PID
#define MSP_ACC_CALIBRATION      205   //in message          no param
#define MSP_MAG_CALIBRATION      206   //in message          no param
#define MSP_SET_MISC             207   //in message          powermeter trig + 8 free for future use
#define MSP_RESET_CONF           208   //in message          no param
#define MSP_SET_WP               209   //in message          sets a given WP (WP#,lat, lon, alt, flags)
#define MSP_SELECT_SETTING       210   //in message          Select Setting Number (0-2)
#define MSP_SET_HEAD             211   //in message          define a new heading hold direction

#define MSP_BIND                 240   //in message          no param

#define MSP_EEPROM_WRITE         250   //in message          no param

#define MSP_DEBUGMSG             253   //out message         debug string buffer
#define MSP_DEBUG                254   //out message         debug1,debug2,debug3,debug4

static uint8_t checksum;
static uint8_t indRX;
static uint8_t cmdMSP = 0;

uint32_t read32() {
  uint32_t t = read16();
  t+= (uint32_t)read16()<<16;
  return t;
}
uint16_t read16() {
  uint16_t t = read8();
  t+= (uint16_t)read8()<<8;
  return t;
}
uint8_t read8()  {
  return inBuf[indRX++]&0xff;
}

void headSerialResponse(uint8_t err, uint8_t s) {
  serialize8('$');
  serialize8('M');
  serialize8(err ? '!' : '>');
  checksum = 0; // start calculating a new checksum
  serialize8(s);
  serialize8(cmdMSP);
}

void headSerialReply(uint8_t s) {
  headSerialResponse(0, s);
}

void inline headSerialError(uint8_t s) {
  headSerialResponse(1, s);
}

void tailSerialReply() {
  serialize8(checksum);UartSendData();
}

void serializeNames(PGM_P s) {
  for (PGM_P c = s; pgm_read_byte(c); c++) {
    serialize8(pgm_read_byte(c));
  }
}

void checkSetup(){
   static uint8_t reciving = 0;
   static uint8_t setupRecBuf[32];
   static uint8_t setupRecCount = 0;
   while(SerialAvailable(0)){
     
     if(reciving == 1){
        setupRecBuf[setupRecCount++] = SerialRead(replayTo);
        if(setupRecCount >= 31){
          conf.F3D            = setupRecBuf[0]; // ESC's are set to 3D Mode
          conf.MIDDLEDEADBAND = setupRecBuf[1]; //nur für 3D
          
          conf.sOneShot       = setupRecBuf[2]; //0=normaler betrieb (4xxhz) 1 ist oneshot 125 
          
          conf.copterType     = setupRecBuf[3]; //0=Bi,1=Tri,2=QUADP,3=QUADX,4=Y4,5=Y6,6=H6P,7=H6X,8=Vtail4
          
          conf.RxType         = setupRecBuf[4]; //0StandardRX,1sat1024,2sat2048,3PPMGrSp,4PPMRobHiFu,5PPMHiSanOthers
          
          conf.MINTHROTTLE   = (setupRecBuf[5]<<8) | setupRecBuf[6];
          conf.MAXTHROTTLE   = (setupRecBuf[7]<<8) | setupRecBuf[8];
          conf.MINCOMMAND    = (setupRecBuf[9]<<8) | setupRecBuf[10];
          conf.MIDRC         = (setupRecBuf[11]<<8) | setupRecBuf[12];
          conf.MINCHECK      = (setupRecBuf[13]<<8) | setupRecBuf[14];
          conf.MAXCHECK      = (setupRecBuf[15]<<8) | setupRecBuf[16];
          
          conf.BILeftMiddle  = (setupRecBuf[17]<<8) | setupRecBuf[18];
          conf.BIRightMiddle = (setupRecBuf[19]<<8) | setupRecBuf[20];
          conf.TriYawMiddle  = (setupRecBuf[21]<<8) | setupRecBuf[22];
          if(setupRecBuf[23] == 0)conf.YAW_DIRECTION = 1; else conf.YAW_DIRECTION =-1;
          if(setupRecBuf[24] == 0)conf.BiLeftDir = 1; else conf.BiLeftDir =-1;
          if(setupRecBuf[25] == 0)conf.BiRightDir = 1; else conf.BiRightDir =-1;
          
          conf.DigiServo = setupRecBuf[26];
          conf.ArmRoll       = setupRecBuf[27];  // arm und disarm über roll statt yaw
          
          conf.MPU6050_DLPF_CFG = setupRecBuf[28]; //0=256(aus),1=188,2=98,3=42,4=20,5=10,6=5 
          conf.s3DMIDDLE = (setupRecBuf[29]<<8) | setupRecBuf[30];
          
          writeParams(1);
          delay(30);
          // setup WDT
          WDTCSR = 0;
          wdt_reset();
          //set up WDT reset
          WDTCSR = (1<<WDCE)|(1<<WDE);
          //Start watchdog timer with 0,125s prescaller
          WDTCSR = (1<<WDE)|(1<<WDP0)|(1<<WDP1);
          while(1);
        }
     }else if(SerialRead(replayTo) == 'A'){
        reciving = 1;
     }else if(SerialRead(replayTo) == 'B'){
        serialize8('B');UartSendData();
        serialize8(conf.F3D);UartSendData();
        serialize8(conf.MIDDLEDEADBAND);UartSendData();
        
        serialize8(conf.sOneShot);UartSendData();
        
        serialize8(conf.copterType);UartSendData();
        
        serialize8(conf.RxType); UartSendData();
        
        serialize8(conf.MINTHROTTLE&0xff);UartSendData(); serialize8(conf.MINTHROTTLE>>8);UartSendData();
        serialize8(conf.MAXTHROTTLE&0xff);UartSendData(); serialize8(conf.MAXTHROTTLE>>8);UartSendData();
        serialize8(conf.MINCOMMAND&0xff);UartSendData(); serialize8(conf.MINCOMMAND>>8);UartSendData(); 
        serialize8(conf.MIDRC&0xff);UartSendData(); serialize8(conf.MIDRC>>8);UartSendData();
        serialize8(conf.MINCHECK&0xff);UartSendData(); serialize8(conf.MINCHECK>>8);UartSendData(); 
        serialize8(conf.MAXCHECK&0xff);UartSendData(); serialize8(conf.MAXCHECK>>8);UartSendData();
        
        serialize8(conf.BILeftMiddle&0xff);UartSendData(); serialize8(conf.BILeftMiddle>>8);UartSendData();
        serialize8(conf.BIRightMiddle&0xff);UartSendData(); serialize8(conf.BIRightMiddle>>8);UartSendData();
        serialize8(conf.TriYawMiddle&0xff);UartSendData(); serialize8(conf.TriYawMiddle>>8);UartSendData();
        if(conf.YAW_DIRECTION == 1){ serialize8(0);UartSendData(); }else{  serialize8(1);UartSendData();}
        if(conf.BiLeftDir == 1){ serialize8(0);UartSendData(); }else{  serialize8(1);UartSendData();}
        if(conf.BiRightDir == 1){ serialize8(0);UartSendData(); }else{  serialize8(1);UartSendData();}
        serialize8(conf.DigiServo);UartSendData();
        
        serialize8(conf.ArmRoll);UartSendData();
        
        serialize8(conf.MPU6050_DLPF_CFG);UartSendData();
        serialize8(conf.s3DMIDDLE&0xff);UartSendData(); serialize8(conf.s3DMIDDLE>>8);UartSendData();
        serialize8(';');UartSendData();
        #if (ARDUINO >= 100)
          USB_Flush(USB_CDC_TX);
        #endif
     }else if(SerialRead(replayTo) == 'X'){
        SetupMode = 0;
     }
  }
}





void serialCom() {
  uint8_t c;  
  static uint8_t offset;
  static uint8_t dataSize;
  static enum _serial_state {
    IDLE,
    HEADER_START,
    HEADER_M,
    HEADER_ARROW,
    HEADER_SIZE,
    HEADER_CMD,
  } c_state = IDLE;
  
  while (SerialAvailable(0)) {
    
    uint8_t bytesTXBuff = ((uint8_t)(headTX-tailTX))%TX_BUFFER_SIZE; // indicates the number of occupied bytes in TX buffer
    if (bytesTXBuff > TX_BUFFER_SIZE - 40 ) return; // ensure there is enough free TX buffer to go further (40 bytes margin)
    c = SerialRead(replayTo);
    
    if(c == '#' && SetupMode == 0 && offset == 0 && indRX == 0 && checksum == 0 && cmdMSP != '#' && replayTo == 0){
      SetupMode = 1;
      return;
    }
    
    if (c_state == IDLE) {
      c_state = (c=='$') ? HEADER_START : IDLE;
      if (c_state == IDLE) evaluateOtherData(c); // evaluate all other incoming serial data
    } else if (c_state == HEADER_START) {
      c_state = (c=='M') ? HEADER_M : IDLE;
    } else if (c_state == HEADER_M) {
      c_state = (c=='<') ? HEADER_ARROW : IDLE;
    } else if (c_state == HEADER_ARROW) {
      if (c > INBUF_SIZE) {  // now we are expecting the payload size
        c_state = IDLE;
        continue;
      }
      dataSize = c;
      offset = 0;
      checksum = 0;
      indRX = 0;
      checksum ^= c;
      c_state = HEADER_SIZE;  // the command is to follow
    } else if (c_state == HEADER_SIZE) {
      cmdMSP = c;
      checksum ^= c;
      c_state = HEADER_CMD;
    } else if (c_state == HEADER_CMD && offset < dataSize) {
      checksum ^= c;
      inBuf[offset++] = c;
    } else if (c_state == HEADER_CMD && offset >= dataSize) {
      if (checksum == c) {  // compare calculated and transferred checksum
        evaluateCommand();  // we got a valid packet, evaluate it
      }
      c_state = IDLE;
    }
  }
}

void evaluateCommand() {
  switch(cmdMSP) {
   case MSP_SET_RAW_RC:
     for(uint8_t i=0;i<8;i++) {
       rcData[i] = read16();
     }
     headSerialReply(0);
     break;
   case MSP_SET_PID:
     for(uint8_t i=0;i<PIDITEMS;i++) {
       conf.P8[i]=read8();
       conf.I8[i]=read8();
       conf.D8[i]=read8();
     }
     headSerialReply(0);
     break;
   case MSP_SET_BOX:
     for(uint8_t i=0;i<CHECKBOXITEMS;i++) {
       conf.activate[i]=read16();
     }
     headSerialReply(0);
     break;
   case MSP_SET_RC_TUNING:
     conf.rcRate8 = read8();
     conf.rcExpo8 = read8();
     conf.rollPitchRate = read8();
     conf.yawRate = read8();
     conf.dynThrPID = read8();
     conf.thrMid8 = read8();
     conf.thrExpo8 = read8();
     headSerialReply(0);
     break;
   case MSP_SET_MISC:
     headSerialReply(0);
     break;
 
   case MSP_IDENT:
     headSerialReply(7);
     serialize8(VERSION);   // multiwii version
     serialize8(MULTITYPE); // type of multicopter
     serialize8(MSP_VERSION);         // MultiWii Serial Protocol Version
     serialize32(0);        // "capability"
     break;
   case MSP_STATUS:
     headSerialReply(11);
     serialize16(cycleTime);
     serialize16(i2c_errors_count);
     serialize16(ACC|BARO<<1|MAG<<2|GPS<<3|SONAR<<4);
     serialize32(f.ACC_MODE<<BOXACC|f.HORIZON_MODE<<BOXHORIZON|f.ARMED<<BOXARM|f.FSBEEP<<BOXBEEP|s3D<<BOX3D);
       serialize8(0);   // current setting
     break;
   case MSP_RAW_IMU:
     headSerialReply(18);
     for(uint8_t i=0;i<3;i++) serialize16(accSmooth[i]);
     for(uint8_t i=0;i<3;i++) serialize16(gyroData[i]);
     for(uint8_t i=0;i<3;i++) serialize16(magADC[i]);
     break;
   case MSP_SERVO:
     headSerialReply(16);
     for(uint8_t i=0;i<8;i++)
       serialize16(servo[i]);
     break;
   case MSP_MOTOR:
     headSerialReply(16);
     for(uint8_t i=0;i<8;i++) {
       serialize16( (i < NUMBER_MOTOR) ? motor[i] : 0 );
     }
     break;
   case MSP_RC:
     headSerialReply(8 * 2);
     for(uint8_t i=0;i<8;i++) serialize16(rcData[i]);
     break;
   case MSP_ATTITUDE:
     headSerialReply(8);
     for(uint8_t i=0;i<2;i++) serialize16(angle[i]);
     serialize16(heading);
     serialize16(headFreeModeHold);
     break;
   case MSP_ALTITUDE:
     headSerialReply(6);
     serialize32(EstAlt);
     serialize16(0);                  // added since r1172
     break;
   case MSP_RC_TUNING:
     headSerialReply(7);
     serialize8(conf.rcRate8);
     serialize8(conf.rcExpo8);
     serialize8(conf.rollPitchRate);
     serialize8(conf.yawRate);
     serialize8(conf.dynThrPID);
     serialize8(conf.thrMid8);
     serialize8(conf.thrExpo8);
     break;
   case MSP_PID:
     headSerialReply(3*PIDITEMS);
     for(uint8_t i=0;i<PIDITEMS;i++) {
       serialize8(conf.P8[i]);
       serialize8(conf.I8[i]);
       serialize8(conf.D8[i]);
     }
     break;
   case MSP_PIDNAMES:
     headSerialReply(strlen_P(pidnames));
     serializeNames(pidnames);
     break;
   case MSP_BOX:
     headSerialReply(2*CHECKBOXITEMS);
     for(uint8_t i=0;i<CHECKBOXITEMS;i++) {
       serialize16(conf.activate[i]);
     }
     break;
   case MSP_BOXNAMES:
     headSerialReply(strlen_P(boxnames));
     serializeNames(boxnames);
     break;
   case MSP_BOXIDS:
     headSerialReply(CHECKBOXITEMS);
     for(uint8_t i=0;i<CHECKBOXITEMS;i++) {
       serialize8(pgm_read_byte(&(boxids[i])));
     }
     break;
   case MSP_MISC:
     headSerialReply(2);
     serialize16(0);
     break;
   case MSP_MOTOR_PINS:
     headSerialReply(8);
     for(uint8_t i=0;i<8;i++) {
       serialize8(PWM_PIN[i]);
     }
     break;
   case MSP_RESET_CONF:
     conf.checkNewConf++;
     checkFirstTime(1);
     headSerialReply(0);
     break;
   case MSP_ACC_CALIBRATION:
     if(!f.ARMED) calibratingA=512;
     headSerialReply(0);
     break;
   case MSP_MAG_CALIBRATION:
     headSerialReply(0);
     break;
   case MSP_EEPROM_WRITE:
     writeParams(0);
     headSerialReply(0);
     break;
   case MSP_DEBUG:
     headSerialReply(8);
     for(uint8_t i=0;i<4;i++) {
       serialize16(debug[i]); // 4 variables are here for general monitoring purpose
     }
     break;
   default:  // we do not know how to handle the (valid) message, indicate error MSP $M!
     headSerialError(0);
     break;
  }
  tailSerialReply();
}

// evaluate all other incoming serial data
void evaluateOtherData(uint8_t sr) {
  switch (sr) {
  // Note: we may receive weird characters here which could trigger unwanted features during flight.
  //       this could lead to a crash easily.
  //       Please use if (!f.ARMED) where neccessary
   
  }
}


// *******************************************************
// Interrupt driven UART transmitter - using a ring buffer
// *******************************************************

void serialize32(uint32_t a) {
  serialize8((a    ) & 0xFF);
  serialize8((a>> 8) & 0xFF);
  serialize8((a>>16) & 0xFF);
  serialize8((a>>24) & 0xFF);
}

void serialize16(int16_t a) {
  serialize8((a   ) & 0xFF);
  serialize8((a>>8) & 0xFF);
}

void serialize8(uint8_t a) {
  uint8_t t = headTX;
  if (++t >= TX_BUFFER_SIZE) t = 0;
  bufTX[t] = a;
  checksum ^= a;
  headTX = t;
}


ISR(USART1_UDRE_vect){
  uint8_t t = tailTX;
  if (headTX != t) {
    if (++t >= TX_BUFFER_SIZE) t = 0;
    UDR1 = bufTX[t];  // Transmit next byte in the ring
    tailTX = t;
  }
  if (t == headTX) UCSR1B &= ~(1<<UDRIE1); // Check if all data is transmitted . if yes disable transmitter UDRE interrupt
}


void UartSendData() {
  if(replayTo == 0 ){
      while(headTX != tailTX) {
        if (++tailTX >= TX_BUFFER_SIZE) tailTX = 0;
        uint8_t* p = bufTX+tailTX;
        USB_Send(USB_CDC_TX,p,1);
      }
  }else if(conf.RxType != 1 && conf.RxType != 2) UCSR1B |= (1<<UDRIE1); // enable transmitter UDRE1 interrupt if deactivacted 
}


static void inline SerialOpen(uint8_t port, uint32_t baud) {
  uint8_t h = ((F_CPU  / 4 / baud -1) / 2) >> 8;
  uint8_t l = ((F_CPU  / 4 / baud -1) / 2);
  switch (port) {
      #if (ARDUINO >= 100) && !defined(TEENSY20)
        case 0:UDIEN &= ~(1<<SOFE);  break;// disable the USB frame interrupt of arduino (it causes strong jitter and we dont need it)
      #endif
    case 1: UCSR1A  = (1<<U2X1); UBRR1H = h; UBRR1L = l; UCSR1B |= (1<<RXEN1)|(1<<TXEN1)|(1<<RXCIE1); break;
  }
}

static void inline SerialEnd(uint8_t port) {
  switch (port) {
    case 1: UCSR1B &= ~((1<<RXEN1)|(1<<TXEN1)|(1<<RXCIE1)); break;
  }
}

static void inline store_uart_in_buf(uint8_t data, uint8_t portnum) {
  uint8_t h = serialHeadRX[portnum];
  if (++h >= RX_BUFFER_SIZE) h = 0;
  if (h == serialTailRX[portnum]) return; // we did not bite our own tail?
  serialBufferRX[serialHeadRX[portnum]][portnum] = data;  
  serialHeadRX[portnum] = h;
}


ISR(USART1_RX_vect)  { 
  if(conf.RxType == 1 || conf.RxType == 2){
    SpektrumISR();
  }else{
    store_uart_in_buf(UDR1, 1); 
  }
}


uint8_t SerialRead(uint8_t port) {
  #if (ARDUINO >= 100)
    USB_Flush(USB_CDC_TX);
  #endif
  if(port == 0) return USB_Recv(USB_CDC_RX);      

  uint8_t t = serialTailRX[port];
  uint8_t c = serialBufferRX[t][port];
  if (serialHeadRX[port] != t) {
    if (++t >= RX_BUFFER_SIZE) t = 0;
    serialTailRX[port] = t;
  }
  return c;
}


uint8_t SerialAvailable(uint8_t port) {
    uint8_t c = 0;
    if(port == 0) c = USB_Available(USB_CDC_RX);
    if(port == 0)
      if(c == 0 && conf.RxType != 1 && conf.RxType != 2 ){
        replayTo = 1;
        port = 1;
      }else{
        replayTo = 0;
        return c;
      }
  return (serialHeadRX[port] - serialTailRX[port])%RX_BUFFER_SIZE;
}

void SerialWrite(uint8_t port,uint8_t c){
 switch (port) {
    case 0: serialize8(c);UartSendData(); break;                 // Serial0 TX is driven via a buffer and a background intterupt
    case 1: while (!(UCSR1A & (1 << UDRE1))) ; UDR1 = c; break;  // Serial1 Serial2 and Serial3 TX are not driven via interrupts
  }
}

void debugmsg_append_str(const char *str) {};
