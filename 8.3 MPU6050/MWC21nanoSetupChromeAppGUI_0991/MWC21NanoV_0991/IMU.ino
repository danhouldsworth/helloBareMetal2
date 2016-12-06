
void computeIMU () {
  uint8_t axis;
  static int16_t gyroADCprevious[3] = {0,0,0};
  int16_t gyroADCp[3];
  int16_t gyroADCinter[3];
  static uint32_t timeInterleave = 0;

  //we separate the 2 situations because reading gyro values with a gyro only setup can be acchieved at a higher rate
  //gyro+nunchuk: we must wait for a quite high delay betwwen 2 reads to get both WM+ and Nunchuk data. It works with 3ms
  //gyro only: the delay to read 2 consecutive values can be reduced to only 0.65ms
      ACC_getADC();
      getEstimatedAttitude();
      Gyro_getADC();
    for (axis = 0; axis < 3; axis++)
      gyroADCp[axis] =  gyroADC[axis];
    timeInterleave=micros();
    annexCode();
    if ((micros()-timeInterleave)>650) {
       annex650_overrun_count++;
    } else {
       while((micros()-timeInterleave)<650) ; //empirical, interleaving delay between 2 consecutive reads
    }
      Gyro_getADC();
    for (axis = 0; axis < 3; axis++) {
      gyroADCinter[axis] =  gyroADC[axis]+gyroADCp[axis];
      // empirical, we take a weighted value of the current and the previous values
      gyroData[axis] = (gyroADCinter[axis]+gyroADCprevious[axis])/3;
      gyroADCprevious[axis] = gyroADCinter[axis]/2;
      if (!ACC) accADC[axis]=0;
    }
  if(conf.copterType < 2){
    static int16_t gyroYawSmooth = 0;
    gyroData[YAW] = (gyroYawSmooth*2+gyroData[YAW])/3;
    gyroYawSmooth = gyroData[YAW];
  }
  if(conf.copterType == 1){
    static int16_t gyroPitchSmooth = 0;
    gyroData[PITCH] = (gyroPitchSmooth*2+gyroData[PITCH])/3;
    gyroPitchSmooth = gyroData[PITCH];
  }
}

// **************************************************
// Simplified IMU based on "Complementary Filter"
// Inspired by http://starlino.com/imu_guide.html
//
// adapted by ziss_dm : http://www.multiwii.com/forum/viewtopic.php?f=8&t=198
//
// The following ideas was used in this project:
// 1) Rotation matrix: http://en.wikipedia.org/wiki/Rotation_matrix
// 2) Small-angle approximation: http://en.wikipedia.org/wiki/Small-angle_approximation
// 3) C. Hastings approximation for atan2()
// 4) Optimization tricks: http://www.hackersdelight.org/
//
// Currently Magnetometer uses separate CF which is used only
// for heading approximation.
//
// **************************************************

//******  advanced users settings *******************
/* Set the Low Pass Filter factor for ACC */
/* Increasing this value would reduce ACC noise (visible in GUI), but would increase ACC lag time*/
/* Comment this if  you do not want filter at all.*/
#ifndef ACC_LPF_FACTOR
  #define ACC_LPF_FACTOR 100
#endif


/* Set the Gyro Weight for Gyro/Acc complementary filter */
/* Increasing this value would reduce and delay Acc influence on the output of the filter*/
#ifndef GYR_CMPF_FACTOR
  #define GYR_CMPF_FACTOR 400.0f
#endif

/* Set the Gyro Weight for Gyro/Magnetometer complementary filter */
/* Increasing this value would reduce and delay Magnetometer influence on the output of the filter*/
#ifndef GYR_CMPFM_FACTOR
  #define GYR_CMPFM_FACTOR 200.0f
#endif

//****** end of advanced users settings *************

#define INV_GYR_CMPF_FACTOR   (1.0f / (GYR_CMPF_FACTOR  + 1.0f))
#define INV_GYR_CMPFM_FACTOR  (1.0f / (GYR_CMPFM_FACTOR + 1.0f))
#if GYRO
  #define GYRO_SCALE ((1998 * PI)/((32767.0f / 4.0f ) * 180.0f * 1000000.0f)) //(MPU6050)
  //#define GYRO_SCALE ((2380 * PI)/((32767.0f / 4.0f ) * 180.0f * 1000000.0f)) //should be 2279.44 but 2380 gives better result
  // +-2000/sec deg scale
  //#define GYRO_SCALE ((200.0f * PI)/((32768.0f / 5.0f / 4.0f ) * 180.0f * 1000000.0f) * 1.5f)     
  // +- 200/sec deg scale
  // 1.5 is emperical, not sure what it means
  // should be in rad/sec
#else
  #define GYRO_SCALE (1.0f/200e6f)
  // empirical, depends on WMP on IDG datasheet, tied of deg/ms sensibility
  // !!!!should be adjusted to the rad/sec
#endif 
// Small angle approximation
#define ssin(val) (val)
#define scos(val) 1.0f

typedef struct fp_vector {
  float X;
  float Y;
  float Z;
} t_fp_vector_def;

typedef union {
  float   A[3];
  t_fp_vector_def V;
} t_fp_vector;

int16_t _atan2(float y, float x){
  #define fp_is_neg(val) ((((uint8_t*)&val)[3] & 0x80) != 0)
  float z = y / x;
  int16_t zi = abs(int16_t(z * 100)); 
  int8_t y_neg = fp_is_neg(y);
  if ( zi < 100 ){
    if (zi > 10) 
     z = z / (1.0f + 0.28f * z * z);
   if (fp_is_neg(x)) {
     if (y_neg) z -= PI;
     else z += PI;
   }
  } else {
   z = (PI / 2.0f) - z / (z * z + 0.28f);
   if (y_neg) z -= PI;
  }
  z *= (180.0f / PI * 10); 
  return z;
}

// Rotate Estimated vector(s) with small angle approximation, according to the gyro data
void rotateV(struct fp_vector *v,float* delta) {
  fp_vector v_tmp = *v;
  v->Z -= delta[ROLL]  * v_tmp.X + delta[PITCH] * v_tmp.Y;
  v->X += delta[ROLL]  * v_tmp.Z - delta[YAW]   * v_tmp.Y;
  v->Y += delta[PITCH] * v_tmp.Z + delta[YAW]   * v_tmp.X; 
}

void getEstimatedAttitude(){
  uint8_t axis;
  int32_t accMag = 0;
  static t_fp_vector EstG;
#if defined(MG_LPF_FACTOR)
  static int16_t mgSmooth[3]; 
#endif
#if defined(ACC_LPF_FACTOR)
  static float accLPF[3];
#endif
  static uint16_t previousT;
  uint16_t currentT = micros();
  float scale, deltaGyroAngle[3];

  scale = (currentT - previousT) * GYRO_SCALE;
  previousT = currentT;

  // Initialization
  for (axis = 0; axis < 3; axis++) {
    deltaGyroAngle[axis] = gyroADC[axis]  * scale;
    #if defined(ACC_LPF_FACTOR)
      accLPF[axis] = accLPF[axis] * (1.0f - (1.0f/ACC_LPF_FACTOR)) + accADC[axis] * (1.0f/ACC_LPF_FACTOR);
      accSmooth[axis] = accLPF[axis];
      #define ACC_VALUE accSmooth[axis]
    #else  
      accSmooth[axis] = accADC[axis];
      #define ACC_VALUE accADC[axis]
    #endif
//    accMag += (ACC_VALUE * 10 / (int16_t)acc_1G) * (ACC_VALUE * 10 / (int16_t)acc_1G);
    accMag += (int32_t)ACC_VALUE*ACC_VALUE ;
  }
  accMag = accMag*100/((int32_t)acc_1G*acc_1G);

  rotateV(&EstG.V,deltaGyroAngle);

  if ( abs(accSmooth[ROLL])<acc_25deg && abs(accSmooth[PITCH])<acc_25deg && accSmooth[YAW]>0) {
    f.SMALL_ANGLES_25 = 1;
  } else {
    f.SMALL_ANGLES_25 = 0;
  }

  // Apply complimentary filter (Gyro drift correction)
  // If accel magnitude >1.4G or <0.6G and ACC vector outside of the limit range => we neutralize the effect of accelerometers in the angle estimation.
  // To do that, we just skip filter, as EstV already rotated by Gyro
  if ( ( 36 < accMag && accMag < 196 ) || f.SMALL_ANGLES_25 )
    for (axis = 0; axis < 3; axis++) {
      int16_t acc = ACC_VALUE;
      EstG.A[axis] = (EstG.A[axis] * GYR_CMPF_FACTOR + acc) * INV_GYR_CMPF_FACTOR;
    }
  
  // Attitude of the estimated vector
  angle[ROLL]  =  _atan2(EstG.V.X , EstG.V.Z) ;
  angle[PITCH] =  _atan2(EstG.V.Y , EstG.V.Z) ;
  if(f.ACC_MODE){
    float anglec = EstG.V.Z/acc_1G;
    accZCal = ((accADC[ROLL]*EstG.V.X+accADC[PITCH]*EstG.V.Y+accADC[YAW]*EstG.V.Z)*InvSqrt(fsq(EstG.V.X)+fsq(EstG.V.Y)+fsq(EstG.V.Z)) - acc_1G)*anglec;
    if(accZCal > 10){
      Zadd = constrain(Zadd--,-200,+200);
    }
    if(accZCal < -10){
      Zadd = constrain(Zadd++,-200,+200);
    }
  }
}

float InvSqrt (float x){ 
  union{  
    int32_t i;  
    float   f; 
  } conv; 
  conv.f = x; 
  conv.i = 0x5f3759df - (conv.i >> 1); 
  return 0.5f * conv.f * (3.0f - x * conv.f * conv.f);
}
float fsq(float x){return x * x;}
