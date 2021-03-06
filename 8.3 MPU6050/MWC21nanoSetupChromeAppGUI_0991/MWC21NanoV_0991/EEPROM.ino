#include <avr/eeprom.h>
#include <EEPROM.h>

#define EEPROM_CONF_VERSION 99

void readEEPROM() {
  uint8_t i;

  eeprom_read_block((void*)&conf, (void*)0, sizeof(conf));
  for(i=0;i<6;i++) {
    lookupPitchRollRC[i] = (2500+conf.rcExpo8*(i*i-25))*i*(int32_t)conf.rcRate8/2500;
  }
  for(i=0;i<11;i++) {
    int16_t tmp = 10*i-conf.thrMid8;
    uint8_t y = 1;
    if (tmp>0) y = 100-conf.thrMid8;
    if (tmp<0) y = conf.thrMid8;
    lookupThrottleRC[i] = 10*conf.thrMid8 + tmp*( 100-conf.thrExpo8+(int32_t)conf.thrExpo8*(tmp*tmp)/(y*y) )/10; // [0;1000]
    lookupThrottleRC[i] = conf.MINTHROTTLE + (int32_t)(conf.MAXTHROTTLE-conf.MINTHROTTLE)* lookupThrottleRC[i]/1000;            // [0;1000] -> [MINTHROTTLE;MAXTHROTTLE]
  }

}

void writeParams(uint8_t b) {
  conf.checkNewConf = EEPROM_CONF_VERSION; // make sure we write the current version into eeprom
  eeprom_write_block((const void*)&conf, (void*)0, sizeof(conf));
  readEEPROM();
  if (b == 1) blinkLED(15,20,1);
}

void checkFirstTime(uint8_t guiReset) {
  if (EEPROM_CONF_VERSION == conf.checkNewConf) return;
  for (uint16_t y = 0; y < 512; y++)
    EEPROM.write(y, 0);  
    
  conf.P8[ROLL]  = 40;  conf.I8[ROLL] = 30; conf.D8[ROLL]  = 23;
  conf.P8[PITCH] = 40; conf.I8[PITCH] = 30; conf.D8[PITCH] = 23;
  conf.P8[YAW]   = 85;  conf.I8[YAW]  = 45;  conf.D8[YAW]  = 0;
  conf.P8[PIDALT]   = 16; conf.I8[PIDALT]   = 15; conf.D8[PIDALT]   = 7;
  
  conf.P8[PIDPOS]  = 0;     conf.I8[PIDPOS]    = 0;       conf.D8[PIDPOS]    = 0;
  conf.P8[PIDPOSR] = 0; conf.I8[PIDPOSR]   = 0;  conf.D8[PIDPOSR]   = 0;
  conf.P8[PIDNAVR] = 0;          conf.I8[PIDNAVR]   = 0;           conf.D8[PIDNAVR]   = 0;

  conf.P8[PIDLEVEL] = 70; conf.I8[PIDLEVEL] = 10; conf.D8[PIDLEVEL] = 100;
  conf.P8[PIDMAG] = 40;
  
  conf.P8[PIDVEL] = 0;  conf.I8[PIDVEL] = 0;  conf.D8[PIDVEL] = 0;
  
  conf.rcRate8 = 90; conf.rcExpo8 = 65;
  conf.rollPitchRate = 0;
  conf.yawRate = 0;
  conf.dynThrPID = 0;
  conf.thrMid8 = 50; conf.thrExpo8 = 0;
  for(uint8_t i=0;i<CHECKBOXITEMS;i++) {conf.activate[i] = 0;}
  conf.angleTrim[0] = 0; conf.angleTrim[1] = 0;
  
  if(guiReset == 0){
    conf.F3D            = 0; // ESC's are set to 3D Mode
    conf.MIDDLEDEADBAND = 40; //nur für 3D
    
    conf.sOneShot       = 0; //0=normaler betrieb (4xxhz) 1 ist oneshot 125 
    
    conf.copterType     = 3; //0=Bi,1=Tri,2=QUADP,3=QUADX,4=Y4,5=Y6,6=H6P,7=H6X,8=Vtail4
    
    conf.RxType         = 0; //0StandardRX,1sat1024,2sat2048,3PPMGrSp,4PPMRobHiFu,5PPMHiSanOthers
    
    conf.MINTHROTTLE   = 1150;
    conf.MAXTHROTTLE   = 2000;
    conf.MINCOMMAND    = 1000;
    conf.MIDRC         = 1500;
    conf.MINCHECK      = 1100;
    conf.MAXCHECK      = 1900;
    
    conf.BILeftMiddle  = 1500; 
    conf.BIRightMiddle = 1500;
    conf.TriYawMiddle  = 1500;
    conf.YAW_DIRECTION = 1;
    conf.BiLeftDir     = 1;
    conf.BiRightDir    = 1;
    conf.DigiServo     = 0;
    
    conf.ArmRoll       = 0;  // arm und disarm über roll statt yaw
    
    conf.MPU6050_DLPF_CFG = 3; //0=256(aus),1=188,2=98,3=42,4=20,5=10,6=5
    
    conf.s3DMIDDLE    = 1500; 
  }
  
  
  writeParams(0); // this will also (p)reset checkNewConf with the current version number again.
}

