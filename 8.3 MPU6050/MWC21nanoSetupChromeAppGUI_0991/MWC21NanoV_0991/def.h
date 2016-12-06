
/**************************************************************************************/
/***************             Proc specific definitions             ********************/
/**************************************************************************************/
// Proc auto detection
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)
  #define PROMINI
#endif
#if defined(__AVR_ATmega32U4__) || defined(TEENSY20)
  #define PROMICRO
#endif
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega1281__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__)
  #define MEGA
#endif

  #if !defined(TEENSY20)
    #define LEDPIN_PINMODE             //
    #define LEDPIN_TOGGLE              PIND |= 1<<5;     //switch LEDPIN state (Port D5)
    #if !defined(PROMICRO10)
      #define LEDPIN_OFF                 PORTD |= (1<<5);
      #define LEDPIN_ON                  PORTD &= ~(1<<5);  
    #else
      #define LEDPIN_OFF                PORTD &= ~(1<<5);
      #define LEDPIN_ON                 PORTD |= (1<<5);
    #endif
  #else
    #define LEDPIN_PINMODE           DDRD |= (1<<6);
    #define LEDPIN_OFF               PORTD &= ~(1<<6);
    #define LEDPIN_ON                PORTD |= (1<<6);   
    #define LEDPIN_TOGGLE            PIND |= 1<<6;     //switch LEDPIN state (Port D6)  
  #endif
  #if defined(D8BUZZER)
    #define BUZZERPIN_PINMODE          DDRB |= (1<<4);
    #define BUZZERPIN_ON               PORTB |= 1<<4;
    #define BUZZERPIN_OFF              PORTB &= ~(1<<4); 
  #elif defined(A32U4ALLPINS)
    #define BUZZERPIN_PINMODE          DDRD |= (1<<4);
    #define BUZZERPIN_ON               PORTD |= 1<<4;
    #define BUZZERPIN_OFF              PORTD &= ~(1<<4);    
  #else
    #define BUZZERPIN_PINMODE          DDRD |= (1<<3);
    #define BUZZERPIN_ON               PORTD |= 1<<3;
    #define BUZZERPIN_OFF              PORTD &= ~(1<<3); 
  #endif
  #define POWERPIN_PINMODE           //
  #define POWERPIN_ON                //
  #define POWERPIN_OFF               //
  #define I2C_PULLUPS_ENABLE         PORTD |= 1<<0; PORTD |= 1<<1;   // PIN 2&3 (SDA&SCL)
  #define I2C_PULLUPS_DISABLE        PORTD &= ~(1<<0); PORTD &= ~(1<<1);
  #define PINMODE_LCD                DDRD |= (1<<2);
  #define LCDPIN_OFF                 PORTD &= ~1;
  #define LCDPIN_ON                  PORTD |= 1;
  #define STABLEPIN_PINMODE          ;
  #define STABLEPIN_ON               ;
  #define STABLEPIN_OFF              ;
  #define PPM_PIN_INTERRUPT          DDRE &= ~(1 << 6);PORTE |= (1 << 6);EIMSK |= (1 << INT6);EICRB |= (1 << ISC61)|(1 << ISC60);
  #define SPEK_SERIAL_VECT           USART1_RX_vect
  #define SPEK_DATA_REG              UDR1
  #define USB_CDC_TX                 3
  #define USB_CDC_RX                 2
  
  //soft PWM Pins  
  #define SOFT_PWM_1_PIN_HIGH        PORTD |= 1<<4;
  #define SOFT_PWM_1_PIN_LOW         PORTD &= ~(1<<4);
  #define SOFT_PWM_2_PIN_HIGH        PORTF |= 1<<5;
  #define SOFT_PWM_2_PIN_LOW         PORTF &= ~(1<<5);
  #if !defined(A32U4ALLPINS)
    #define SOFT_PWM_3_PIN_HIGH        PORTF |= 1<<7;
    #define SOFT_PWM_3_PIN_LOW         PORTF &= ~(1<<7);
    #define SOFT_PWM_4_PIN_HIGH        PORTF |= 1<<6;
    #define SOFT_PWM_4_PIN_LOW         PORTF &= ~(1<<6);
    #define SW_PWM_P3                  A1        
    #define SW_PWM_P4                  A0
  #else
    #define SOFT_PWM_3_PIN_HIGH        PORTF |= 1<<4;
    #define SOFT_PWM_3_PIN_LOW         PORTF &= ~(1<<4);
    #define SOFT_PWM_4_PIN_HIGH        PORTF |= 1<<5;
    #define SOFT_PWM_4_PIN_LOW         PORTF &= ~(1<<5); 
    #define SW_PWM_P3                  A2        
    #define SW_PWM_P4                  A3 
  #endif
  
  // Servos
  
  //Standart RX
  #define THROTTLEPIN                  3
  #if defined(A32U4ALLPINS)
    #define ROLLPIN                    6
    #define PITCHPIN                   2
    #define YAWPIN                     4
    #define AUX1PIN                    5
  #else
    #define ROLLPIN                    4
    #define PITCHPIN                   5
    #define YAWPIN                     2
    #define AUX1PIN                    6
  #endif
  #define AUX2PIN                      7 
  #define AUX3PIN                      1 // unused 
  #define AUX4PIN                      0 // unused 
  #define PCINT_PIN_COUNT              5 // one more bit (PB0) is added in RX code
  #define PCINT_RX_BITS                (1<<1),(1<<2),(1<<3),(1<<4),(1<<0)
  #define PCINT_RX_PORT                PORTB
  #define PCINT_RX_MASK                PCMSK0
  #define PCIR_PORT_BIT                (1<<0)
  #define RX_PC_INTERRUPT              PCINT0_vect
  #define RX_PCINT_PIN_PORT            PINB
  
  #define ISR_UART                    ISR(USART_UDRE_vect)
  #if !defined(A32U4ALLPINS) && !defined(TEENSY20)
    #define V_BATPIN                  A3    // Analog PIN 3
  #elif defined(A32U4ALLPINS)
    #define V_BATPIN                  A4    // Analog PIN 4
  #else
    #define V_BATPIN                  A2    // Analog PIN 3
  #endif
  #if !defined(TEENSY20)
    #define PSENSORPIN                A2    // Analog PIN 2 
  #else
    #define PSENSORPIN                A2    // Analog PIN 2 
  #endif


/**************************************************************************************/
/***************      IMU Orientations and Sensor definitions      ********************/
/**************************************************************************************/
  #define MPU6050
  #define ACC_ORIENTATION(X, Y, Z)  {accADC[ROLL]  = -Y; accADC[PITCH]  =  X; accADC[YAW]  =  Z;}
  #define GYRO_ORIENTATION(X, Y, Z) {gyroADC[ROLL] = -X; gyroADC[PITCH] = -Y; gyroADC[YAW] = -Z;}
  #undef INTERNAL_I2C_PULLUPS
  // move motor 7 & 8 to pin 4 & A2
  #define SOFT_PWM_3_PIN_HIGH        PORTD |= 1<<4;
  #define SOFT_PWM_3_PIN_LOW         PORTD &= ~(1<<4);
  #define SOFT_PWM_4_PIN_HIGH        PORTF |= 1<<5;
  #define SOFT_PWM_4_PIN_LOW         PORTF &= ~(1<<5);
  #define SW_PWM_P3                  4        
  #define SW_PWM_P4                  A2
  #define HWPWM6
  // move servo 3 & 4 to pin 13 & 11
  #define SERVO_3_PINMODE   DDRC |= (1<<7); // 13
  #define SERVO_3_PIN_HIGH  PORTC |= 1<<7;
  #define SERVO_3_PIN_LOW   PORTC &= ~(1<<7);
  #define SERVO_4_PINMODE   DDRB |= (1<<7); // 11
  #define SERVO_4_PIN_HIGH  PORTB |= 1<<7;
  #define SERVO_4_PIN_LOW   PORTB &= ~(1<<7);



/**************************************************************************************/
/***************              Sensor Type definitions              ********************/
/**************************************************************************************/

#define ACC 1
#define MAG 0
#define GYRO 1
#define BARO 0
#define SONAR 0
#define GPS 0




