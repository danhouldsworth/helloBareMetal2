/*
OneShot PWM by Dieter Würtenberger (dieter@modfly.de)
3D MixTable By Felix Niessen (felix.niessen@googlemail.com)
*/

static uint8_t  cycleCounter = 0;


/**************************************************************************************/
/*****   Writes the Motors and Servos values to the Timer PWM compare registers   ******/
/**************************************************************************************/
void writeMotors() {
      uint16_t PWM_t; 
     // ################ first Update Motors (125us-250us or 1000us-2000us) #####################
          if(conf.sOneShot == 0){
              OCR1A = motor[0]<<4;   // Imp=1000.. 2000us @11Bit, Port B5 9
              OCR1B = motor[1]<<4;   // Imp=1000.. 2000us @11Bit, Port B6 10
              if(conf.copterType != 0) OCR1C = motor[4]<<4;   // Imp=1000.. 2000us @11Bit, Port B7 11
          }else{
              OCR1A = motor[0]<<1;   // Imp= 125.. 0250us @11Bit, Port B5 
              OCR1B = motor[1]<<1;   // Imp= 125.. 0250us @11Bit, Port B6 
              if(conf.copterType != 0) OCR1C = motor[4]<<1;   // Imp= 125.. 0250us @11Bit, Port B7
          }
          if(conf.sOneShot == 0){      // Timer3 A 
              if(conf.copterType > 1) OCR3A = motor[2]<<4;   // Imp=1000.. 2000us @11Bit, Port C6 5
          }else{
              if(conf.copterType > 1) OCR3A = motor[2]<<1;   // Imp= 125.. 0250us @11Bit, Port C6
          }
        // ###### Update-Reihenfolge bei Timer4:  beide Compare-Register besetzen ######
          PWM_t = 2047-motor[5];  TC4H = (PWM_t >> 8);  OCR4A = (unsigned char)(PWM_t);  // Port C7 13
          if(conf.copterType != 1){ ;
            PWM_t = 2047-motor[3];  TC4H = (PWM_t >> 8);  OCR4D = (unsigned char)(PWM_t);  // Port D7 6
          }else{                    
            PWM_t = 2047-motor[2];  TC4H = (PWM_t >> 8);  OCR4D = (unsigned char)(PWM_t); // Port D7 6
          } 
          if(conf.sOneShot == 0){    
              OCR3B = motor[6]<<4;   // Imp=1000.. 2000us @11Bit, Port variable (z.Zt. PF5)
              OCR3C = motor[7]<<4;   // Imp=1000.. 2000us @11Bit, Port variable (z.Zt. PF6)
          }else{
              OCR3B = motor[6]<<1;   // Imp= 125.. 250us @11Bit, Port variable 
              OCR3C = motor[7]<<1;   // Imp= 125.. 250us @11Bit, Port variable
          }
     // #####################  then Update Servos (fix 1000us-2000us) #############################
        if ((cycleCounter == 1 && conf.DigiServo == 1) || (cycleCounter == 5 && conf.DigiServo == 0)){ 
          if(conf.copterType == 0){
            OCR1C = servo[4]<<4;   // Imp=900.. 2100us @11Bit
            OCR3A = servo[5]<<4;   // Imp=900.. 2100us @11Bit
          }
          if(conf.copterType == 1){
            OCR3A = servo[5]<<4;   // Imp=900.. 2100us @11Bit
          }
          cycleCounter = 0;
        }else{
          cycleCounter++;
        }
    // ##########################  at last start AT32u4 ONESHOTS  ######################################
     // Nun bei allen 8 PWMs gleichzeitig Update erzwingen, ONESHOTS starten und Reset vorbereiten.
     TCNT1 = 0xFFFF;  OCR1A=0; OCR1B=0; OCR1C=0;  // die drei Timer1-ONESHOTS starten
     PORTF |= 1<<6;  PORTF |= 1<<7;               // set Ports PF6(ADC6), PF7(ADC7) for SW_Oneshots with Timer3
     TCNT3 = 0xFFFF;  OCR3A=0; OCR3B=0; OCR3C=0;  // die drei Timer3-ONESHOTS starten
     if(conf.sOneShot == 1){                   // die beiden Timer4-ONESHOTS starten
        TCCR4B = B00000011;  // Start Timer4 mit Prescaler= 4 / Imp= 125.. 250us @9Bit, Bit.7=PWM4X clear !!! (PWM Inversion Mode)
     }else{
        TCCR4B = B00000110;  // Start Timer4 mit Prescaler=32 / Imp=1000..2000us @9Bit, Bit.7=PWM4X clear !!! (PWM Inversion Mode)
     }
}


/**************************************************************************************/
/************        Initialize the PWM Timers and Registers         ******************/
/**************************************************************************************/
void initOutput() {
  
/****************            mark all PWM pins as Output             ******************/
  pinMode(9,OUTPUT);
  pinMode(10,OUTPUT);
  pinMode(5,OUTPUT);
  pinMode(6,OUTPUT);
  pinMode(11,OUTPUT);
  pinMode(13,OUTPUT);
  
  /******** Specific PWM Timers & Registers for the atmega32u4   ************/
  // ************** INIT Timer1 for 3-Channel HW-Oneshots with Mode= 14 on OC1A, OC1B, OC1C  ****************************************
  TCNT1  = 0;  TCCR1C = 0; TIMSK1 = 0;
  TCCR1A = B10101010;     // PWM_WaveformMode=14 -> Fast_PWM, TOP=ICRn, PWM_OutputMode=non-inverting
  TCCR1B = B00011001;     // Prescaler=clk/1 / Imp=125.. 250us @11Bit oder Imp=1000.. 2000us @14Bit
  ICR1 = 0xFFFF;          // set TOP TIMER1 to max
 // *************** INIT Timer3 for 3-Channel Oneshots with Mode= 14 on OC3A(pure HW) and OC3B, OC3C(both SW)   *********************
  TCNT3  = 0;  TCCR3C = 0; TIMSK3 = 0;
  TCCR3A = B10101010;     // PWM_WaveformMode=14 -> Fast_PWM, TOP=ICRn, PWM_OutputMode=non-inverting
  TCCR3B = B00011001;     // Prescaler=clk/1 / Imp=125.. 250us @11Bit oder Imp=1000.. 2000us @14Bit
  ICR3 = 0xFFFF;          // set TOP TIMER3 to max
  DDRF |= (1<<6)|(1<<7);	// set Port PF6=ADC6, PF7=ADC7 as output for SW-Oneshots
// *************** INIT Timer4 for 2-Channel HW-Oneshots with Compare Units OC4A, OC4D ********************************************
//  OPERATION: enhanced PWM Mode, Fast PWM , only Pins OC4x connected with inverted OCnx signals
  TCNT4 = 0; TCCR4B = 0; TCCR4D = 0; TCCR4E = 0; TIMSK4 = 0; TCCR4A = 0; TCCR4C = 0;  
  //  set Waveform Generation Mode Bits PWM4x and WGM41..0 : mit PWM4x=1, WGM41_40=00 --> Fast PWM
  TCCR4A |= (1<<PWM4A);   // PWM4A enable, Set PWM4A=1
  TCCR4C |= (1<<PWM4D);   // PWM4D enable, Set PWM4D=1
  TCCR4E |= (1<<ENHC4);   // Set Enhanced PWM Mode = TCCR4E.6
  //  set Output-Mode Bits COM4x1..0   : for PWM Modes --> should Output inverted, non-inverted, complemetary or disconnected
  TCCR4A |= (1<<COM4A0) | (1<<COM4A1); // Set PortA Operation Mode with COM4A1..0 : COM4A1..0=11 -> only Pin OC4A connected, inverted
  TCCR4C |= (1<<COM4D0) | (1<<COM4D1); // Set PortD Operation Mode with COM4D1..0 : COM4D1..0=11 -> only Pin OC4D connected, inverted
  TC4H = 0x3; OCR4C = 0xFF;   // set TOP Timer4 with OCR4C-Register for 11 Bit steps
  TIMSK4 =  B00000100;        // set Timer4 Overflow Interrupt Enable
  TCCR4B = 0;
}
ISR(TIMER4_OVF_vect)   { TCCR4B = 0;}        //  Stop Timer4 -> Prescaler=0

/**************************************************************************************/
/********** Mixes the Computed stabilize values to the Motors & Servos  ***************/
/**************************************************************************************/
void mixTable() {
  int16_t maxMotor;
  int16_t minMotor;
  int16_t useThrottle;
  uint8_t i;
  
  if(s3D == 1){
    if ((rcData[THROTTLE]) > conf.s3DMIDDLE){
      useThrottle = constrain(rcData[THROTTLE], conf.s3DMIDDLE+conf.MIDDLEDEADBAND, conf.MAXTHROTTLE); 
    }else{
      useThrottle = constrain(rcData[THROTTLE], conf.MINCOMMAND, conf.s3DMIDDLE-conf.MIDDLEDEADBAND); 
    }
    axisPID[ROLL] = axisPID[ROLL]/2;
    axisPID[PITCH] = axisPID[PITCH]/2;
    axisPID[YAW] = axisPID[YAW]/2;
  }else if(conf.F3D == 1){
    useThrottle = constrain(((rcCommand[THROTTLE]-1000)>>1)+conf.s3DMIDDLE, conf.s3DMIDDLE+((conf.MINCOMMAND-1000)>>1), conf.MAXTHROTTLE); 
    if(f.ACC_MODE){
      useThrottle = useThrottle + (Zadd/20);
    }
    axisPID[ROLL] = axisPID[ROLL]/2;
    axisPID[PITCH] = axisPID[PITCH]/2;
    axisPID[YAW] = axisPID[YAW]/2;    
  }else{
    if(f.ACC_MODE){
      useThrottle = rcCommand[THROTTLE] + (Zadd/10);
    }else{
      useThrottle = rcCommand[THROTTLE];
    }
  }
  
  if (NUMBER_MOTOR > 3){
    //prevent "yaw jump" during yaw correction
    axisPID[YAW] = constrain(axisPID[YAW],-100-abs(rcCommand[YAW]),+100+abs(rcCommand[YAW]));
  } 
  
  #define PIDMIX(X,Y,Z) useThrottle + axisPID[ROLL]*X + axisPID[PITCH]*Y + conf.YAW_DIRECTION * axisPID[YAW]*Z


  /****************                   main Mix Table                ******************/
  if(conf.copterType == 0){ // BI
    motor[0] = PIDMIX(+1, 0, 0); //LEFT
    motor[1] = PIDMIX(-1, 0, 0); //RIGHT        
    servo[4]  = constrain(conf.BILeftMiddle + ((conf.YAW_DIRECTION * axisPID[YAW]) + axisPID[PITCH])*conf.BiLeftDir, 900, 2100); //LEFT
    servo[5]  = constrain(conf.BIRightMiddle + ((conf.YAW_DIRECTION * axisPID[YAW]) - axisPID[PITCH])*conf.BiRightDir, 900, 2100); //RIGHT
  }else if(conf.copterType == 1){ // TRI
    motor[0] = PIDMIX( 0,+4/3, 0); //REAR
    motor[1] = PIDMIX(-1,-2/3, 0); //RIGHT
    motor[2] = PIDMIX(+1,-2/3, 0); //LEFT
    servo[5] = constrain(conf.TriYawMiddle + conf.YAW_DIRECTION * axisPID[YAW], 900, 2100); //REAR
  }else if(conf.copterType == 2){ // QUADP
    motor[0] = PIDMIX( 0,+1,-1); //REAR
    motor[1] = PIDMIX(-1, 0,+1); //RIGHT
    motor[2] = PIDMIX(+1, 0,+1); //LEFT
    motor[3] = PIDMIX( 0,-1,-1); //FRONT
  }else if(conf.copterType == 3){ // QUADX
    motor[0] = PIDMIX(-1,+1,-1); //REAR_R
    motor[1] = PIDMIX(-1,-1,+1); //FRONT_R
    motor[2] = PIDMIX(+1,+1,+1); //REAR_L
    motor[3] = PIDMIX(+1,-1,-1); //FRONT_L
  }else if(conf.copterType == 4){ // Y4
    motor[0] = PIDMIX(+0,+1,-1);   //REAR_1 CW
    motor[1] = PIDMIX(-1,-1, 0); //FRONT_R CCW
    motor[2] = PIDMIX(+0,+1,+1);   //REAR_2 CCW
    motor[3] = PIDMIX(+1,-1, 0); //FRONT_L CW
  }else if(conf.copterType == 5){ // Y6
    motor[0] = PIDMIX(+0,+4/3,+1); //REAR
    motor[1] = PIDMIX(-1,-2/3,-1); //RIGHT
    motor[2] = PIDMIX(+1,-2/3,-1); //LEFT
    motor[3] = PIDMIX(+0,+4/3,-1); //UNDER_REAR
    motor[4] = PIDMIX(-1,-2/3,+1); //UNDER_RIGHT
    motor[5] = PIDMIX(+1,-2/3,+1); //UNDER_LEFT  
  }else if(conf.copterType == 6){ // HEXFP
    motor[0] = PIDMIX(-7/8,+1/2,+1); //REAR_R
    motor[1] = PIDMIX(-7/8,-1/2,-1); //FRONT_R
    motor[2] = PIDMIX(+7/8,+1/2,+1); //REAR_L
    motor[3] = PIDMIX(+7/8,-1/2,-1); //FRONT_L
    motor[4] = PIDMIX(+0  ,-1  ,+1); //FRONT
    motor[5] = PIDMIX(+0  ,+1  ,-1); //REAR
  }else if(conf.copterType == 7){ // HEXFX
    motor[0] = PIDMIX(-1/2,+7/8,+1); //REAR_R
    motor[1] = PIDMIX(-1/2,-7/8,+1); //FRONT_R
    motor[2] = PIDMIX(+1/2,+7/8,-1); //REAR_L
    motor[3] = PIDMIX(+1/2,-7/8,-1); //FRONT_L
    motor[4] = PIDMIX(-1  ,+0  ,-1); //RIGHT
    motor[5] = PIDMIX(+1  ,+0  ,+1); //LEFT
  }else if(conf.copterType == 8){ // V Tail
    motor[0] = PIDMIX(+0,+1, +1); //REAR_R
    motor[1] = PIDMIX(-1, -1, +0); //FRONT_R
    motor[2] = PIDMIX(+0,+1, -1); //REAR_L
    motor[3] = PIDMIX(+1, -1, -0); //FRONT_L
  }
  
  /****************                Filter the Motors values                ******************/
  maxMotor=motor[0];
  minMotor=motor[0];
 
    
  if(s3D == 1){  
     for(i=1;i< NUMBER_MOTOR;i++){
      if (motor[i]>maxMotor) maxMotor=motor[i];
      if (motor[i]<minMotor) minMotor=motor[i];
    }
    for (i = 0; i < NUMBER_MOTOR; i++) {     
      if (maxMotor > conf.MAXTHROTTLE) // this is a way to still have good gyro corrections if at least one motor reaches its max.
        motor[i] -= maxMotor - conf.MAXTHROTTLE;    
        
      if (minMotor < conf.MINCOMMAND) // this is a way to still have good gyro corrections if at least one motor reaches its min.
        motor[i] += conf.MINCOMMAND - minMotor;   
  
      if ((rcData[THROTTLE]) > conf.s3DMIDDLE){
        motor[i] = constrain(motor[i], conf.s3DMIDDLE+conf.MIDDLEDEADBAND, conf.MAXTHROTTLE); 
      }else{
        motor[i] = constrain(motor[i], conf.MINCOMMAND, conf.s3DMIDDLE-conf.MIDDLEDEADBAND); 
      }
      if (!f.ARMED)
        motor[i] = conf.s3DMIDDLE;
    }
  }else if(conf.F3D == 1){
    for(i=1;i< NUMBER_MOTOR;i++)
      if (motor[i]>maxMotor) maxMotor=motor[i];
    for (i = 0; i < NUMBER_MOTOR; i++) {
      if (maxMotor > conf.MAXTHROTTLE) // this is a way to still have good gyro corrections if at least one motor reaches its max.
        motor[i] -= maxMotor - conf.MAXTHROTTLE;
        motor[i] = constrain(motor[i], conf.s3DMIDDLE+((conf.MINTHROTTLE-1000)>>1), conf.MAXTHROTTLE);    
      if ((rcData[THROTTLE]) < conf.MINCHECK)
        motor[i] = conf.s3DMIDDLE+((conf.MINTHROTTLE-1000)>>1);
      if (!f.ARMED)
        motor[i] = conf.s3DMIDDLE;
    }     
  }else{
    for(i=1;i< NUMBER_MOTOR;i++)
      if (motor[i]>maxMotor) maxMotor=motor[i];
    for (i = 0; i < NUMBER_MOTOR; i++) {
      if (maxMotor > conf.MAXTHROTTLE) // this is a way to still have good gyro corrections if at least one motor reaches its max.
        motor[i] -= maxMotor - conf.MAXTHROTTLE;
        motor[i] = constrain(motor[i], conf.MINTHROTTLE, conf.MAXTHROTTLE);    
      if ((rcData[THROTTLE]) < conf.MINCHECK)
        motor[i] = conf.MINTHROTTLE;
      if (!f.ARMED)
        motor[i] = conf.MINCOMMAND;
    }   
  }
  if(throttleTest){
    for(i=0;i< NUMBER_MOTOR;i++){
      if(rcData[THROTTLE] > 1500){
        motor[i] = conf.MAXTHROTTLE;
      }else if(rcData[THROTTLE] < 1500){
        motor[i] = conf.MINCOMMAND;
      }else
        motor[i] = rcData[THROTTLE];
    }
  }
}
