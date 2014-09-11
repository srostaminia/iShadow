#include <intrinsics.h>
//#include <ST/iostm32L151xx.h>
#include "stm32l1xx.h"
#include "main.h"
#include "stm32_CPAL_mpu9150.h"

#include <string.h>
#include "stm32_CPAL_mpu9150.h"
//#include "mpu9150_interrupts.h"
#include "utils.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "mpu9150_interrupts.h"

#include "math.h"

/* Data requested by client. */
#define PRINT_ACCEL     (0x01)
#define PRINT_GYRO      (0x02)
#define PRINT_QUAT      (0x04)
#define PRINT_COMPASS   (0x08)

#define ACCEL_ON        (0x01)
#define GYRO_ON         (0x02)

#define MOTION          (0)
#define NO_MOTION       (1)

#define QUAT_W                0                                                                                // scalar offset
#define QUAT_X                1                                                                                // x offset
#define QUAT_Y                2                                                                                // y offset
#define QUAT_Z                3                                                                                // z offset

#define VEC3_X                0                                                                                // x offset
#define VEC3_Y                1                                                                                // y offset
#define VEC3_Z                2                                                                                // z offset

/* Starting sampling rate. */
#define DEFAULT_MPU_HZ  (100)

//struct hal_s {
//    unsigned char sensors;
//    unsigned char dmp_on;
//    unsigned char wait_for_tap;
//    volatile unsigned char new_gyro;
//    unsigned short report;
//    unsigned short dmp_features;
//    unsigned char motion_int_mode;
//};
//static struct hal_s hal = {0}; // AMM
struct hal_s hal = {0};

// AMM
//static signed char gyro_orientation[9] = {-1, 0, 0,
//                                           0,-1, 0,
//                                           0, 0, 1};

static signed char gyro_orientation[9] = { 1, 0, 0,
                                           0, 1, 0,
                                           0, 0, 1};

static short accel_vals[6] = { -12520,          // min X
                               13548,           // max X
                               -14692,          // min Y
                               15654,           // max Y
                               5384,            // min Z
                               25922 };         // max Z

static __IO uint32_t TimingDelay;

int main()
{
  int result;
  
  if (SysTick_Config(SystemCoreClock / 1000)) {
    while (1);
  }
  
  config_us_delay();
  config_ms_timer();
  
  unsigned char *data;
  //stm32_i2c_write(0x0C << 1, 0x0A, 1, data);
  
  delay_ms(10);
  
  imu_test();
  
//  get_quat();
//  accel_cal();
  
  while(1);
  
  return 0;
}

/**
  * @brief  Inserts a delay time.
  * @param  nTime: specifies the delay time length, in ms.
  * @retval None
  */
void Delay(__IO uint32_t nTime)
{
  TimingDelay = nTime;

  while(TimingDelay != 0);
}

void config_ms_timer()
{
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
  
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); 
  TIM_TimeBaseStructure.TIM_Prescaler = (SystemCoreClock / 1000) - 1;
  TIM_TimeBaseStructure.TIM_Period = UINT16_MAX; 
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
  
  TIM_Cmd(TIM4, ENABLE);
}

#pragma inline=never
void delay_ms(int delayTime)
{
  TIM4->CNT = 0;
  while((uint16_t)(TIM4->CNT) <= delayTime);
}

/**
  * @brief  Decrements the TimingDelay variable.
  * @param  None
  * @retval None
  */
void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
  { 
    TimingDelay--;
  }
}

void config_us_delay()
{
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
  
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); 
  TIM_TimeBaseStructure.TIM_Prescaler = (SystemCoreClock / 1000000) - 1;
  TIM_TimeBaseStructure.TIM_Period = UINT16_MAX; 
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);
  
  TIM_Cmd(TIM5, ENABLE);
}

void delay_us(int delayTime)
{
  uint16_t start = TIM5->CNT;
  while((uint16_t)(TIM5->CNT - start) <= delayTime);
}

static inline unsigned short inv_row_2_scale(const signed char *row)
{
    unsigned short b;

    if (row[0] > 0)
        b = 0;
    else if (row[0] < 0)
        b = 4;
    else if (row[1] > 0)
        b = 1;
    else if (row[1] < 0)
        b = 5;
    else if (row[2] > 0)
        b = 2;
    else if (row[2] < 0)
        b = 6;
    else
        b = 7;      // error
    return b;
}

static inline unsigned short inv_orientation_matrix_to_scalar(
    const signed char *mtx)
{
    unsigned short scalar;

    /*
       XYZ  010_001_000 Identity Matrix
       XZY  001_010_000
       YXZ  010_000_001
       YZX  000_010_001
       ZXY  001_000_010
       ZYX  000_001_010
     */

    scalar = inv_row_2_scale(mtx);
    scalar |= inv_row_2_scale(mtx + 3) << 3;
    scalar |= inv_row_2_scale(mtx + 6) << 6;


    return scalar;
}
/* Handle sensor on/off combinations. */
static void setup_gyro(void)
{
    unsigned char mask = 0;
    if (hal.sensors & ACCEL_ON)
        mask |= INV_XYZ_ACCEL;
    if (hal.sensors & GYRO_ON)
        mask |= INV_XYZ_GYRO;
    /* If you need a power transition, this function should be called with a
     * mask of the sensors still enabled. The driver turns off any sensors
     * excluded from this mask.
     */
    mpu_set_sensors(mask);
    if (!hal.dmp_on)
        mpu_configure_fifo(mask);
}
static void tap_cb(unsigned char direction, unsigned char count)
{
}

static void android_orient_cb(unsigned char orientation)
{
}

int imu_test(void)
{
    int result;
    unsigned char accel_fsr;
    unsigned short gyro_rate, gyro_fsr;
    //unsigned long timestamp;
    struct int_param_s int_param;
    
    /************ 1. Initialize CPAL for I2C communication with MPU9150 *****/
    MPU9150_Config();
    if(MPU9150_GetStatus() != SUCCESS)
    {
      while (1);
    }
    /******** 2. Configure INT pin of STM32 for MPU9150 interrupts *********/
    //MPU9150_Interrupt_Init(MPU9150_INT_MODE_EXTI);
    
    result = 0;
    
    int_param.cb = NULL;
    int_param.pin = 0;
    int_param.lp_exit = 0;
    int_param.active_low = 1;
    // "int_param" structure is doing nothing here, just statisfying calling convention
    result += mpu_init(&int_param); 
    result += mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL | INV_XYZ_COMPASS);   
    result += mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL);   
    result += dmp_load_motion_driver_firmware();// load the DMP firmware
    result += dmp_set_orientation(inv_orientation_matrix_to_scalar(gyro_orientation));
    result += dmp_register_tap_cb(tap_cb);
    result += dmp_register_android_orient_cb(android_orient_cb);
    hal.dmp_features = DMP_FEATURE_6X_LP_QUAT | DMP_FEATURE_TAP |
        DMP_FEATURE_ANDROID_ORIENT | DMP_FEATURE_SEND_RAW_ACCEL | DMP_FEATURE_SEND_CAL_GYRO |
        DMP_FEATURE_GYRO_CAL;
    result += dmp_enable_feature(hal.dmp_features);
    result += dmp_set_fifo_rate(DEFAULT_MPU_HZ);
    //result += mpu_set_dmp_state(1);
		/* Initialize HAL state variables. */
    memset(&hal, 0, sizeof(hal));
    hal.sensors = ACCEL_ON | GYRO_ON;
    hal.report = PRINT_QUAT;
    result += mpu_set_sample_rate(DEFAULT_MPU_HZ);
    mpu_set_compass_sample_rate(100);       // set the compass update rate to match
//    result += mpu_set_dmp_state(1); // AMM
    /************** 3. Enable the Interrupt now **************************/
    //MPU9150_Interrupt_Cmd(ENABLE);
    hal.dmp_on = 1;
    
    long gyro[3], accel[3];
    result += mpu_run_self_test(gyro, accel);
    
    return result;
}

float MPUQuaternionNorm(float q[])
{
  return sqrt(q[QUAT_W] * q[QUAT_W] + q[QUAT_X] * q[QUAT_X] +  
      q[QUAT_Y] * q[QUAT_Y] + q[QUAT_Z] * q[QUAT_Z]);
}

void MPUQuaternionQuaternionToEuler(const float q[], float v[])
{
  float pole = (3.1415926535897932384626433832795028841)/2.0f - 0.05f;                           // fix roll near poles with this tolerance

  v[VEC3_Y] = asin(2.0f * (q[QUAT_W] * q[QUAT_Y] - q[QUAT_X] * q[QUAT_Z]));

  if ((v[VEC3_Y] < pole) && (v[VEC3_Y] > -pole))
          v[VEC3_X] = atan2(2.0f * (q[QUAT_Y] * q[QUAT_Z] + q[QUAT_W] * q[QUAT_X]),
                    1.0f - 2.0f * (q[QUAT_X] * q[QUAT_X] + q[QUAT_Y] * q[QUAT_Y]));
        
  v[VEC3_Z] = atan2(2.0f * (q[QUAT_X] * q[QUAT_Y] + q[QUAT_W] * q[QUAT_Z]),
                    1.0f - 2.0f * (q[QUAT_Y] * q[QUAT_Y] + q[QUAT_Z] * q[QUAT_Z]));
}

long* calc_bias(long accel_bias[3]) {
  accel_bias[0] = -((long)accel_vals[1] + (long)accel_vals[0]) / 2;
  accel_bias[1] = -((long)accel_vals[3] + (long)accel_vals[2]) / 2;
  accel_bias[2] = -((long)accel_vals[5] + (long)accel_vals[4]) / 2;
//  accel_bias[2] = 0;
  
  return accel_bias;
}

void get_quat()
{
    int result;
    unsigned char accel_fsr;
    unsigned short gyro_rate, gyro_fsr;
    //unsigned long timestamp;
    //struct int_param_s int_param;
    
    short gyro[3], accel[3], sensors;
    long accel_bias[3];
    short compass[3];
    unsigned char more;
    long quat[4];
    float f_quat[4], eulers[3];
    unsigned long sensor_timestamp;
    
    /************ 1. Initialize CPAL for I2C communication with MPU9150 *****/
    MPU9150_Config();
    if(MPU9150_GetStatus() != SUCCESS)
    {
      while(1);
    }
    /******** 2. Configure INT pin of STM32 for MPU9150 interrupts *********/
    MPU9150_Interrupt_Init(MPU9150_INT_MODE_EXTI); // AMM
    
    result = 0; // AMM
    
    /* If you're not using an MPU9150 AND you're not using DMP features, this
     * function will place all slaves on the primary bus.
     * mpu_set_bypass(1);
     */
    /* Get/set hardware configuration. Start gyro. */
    // AMM: There is an issue with these biases, I believe due to the gravity offset - leave this off for now
//    result += mpu_get_accel_bias(accel_bias);
//    
//    if (accel_bias[0] == 0xF9F8 && accel_bias[1] == 0xFD59 &&
//        accel_bias[2] == 0x174)
//    {
//      result += mpu_set_accel_bias(calc_bias(accel_bias));
//    }
    
    /* Wake up all sensors. */
    result += mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL);
    /* Push both gyro and accel data into the FIFO. */
    result += mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL);
    result += mpu_set_sample_rate(DEFAULT_MPU_HZ);
    result += mpu_set_gyro_fsr(2000);
    /* Read back configuration in case it was set improperly. */
    result += mpu_get_sample_rate(&gyro_rate);
    result += mpu_get_gyro_fsr(&gyro_fsr);
    result += mpu_get_accel_fsr(&accel_fsr);

    /* Initialize HAL state variables. */
    memset(&hal, 0, sizeof(hal));
    hal.sensors = ACCEL_ON | GYRO_ON;
    hal.report = PRINT_QUAT;

    /* To initialize the DMP:
     * 1. Call dmp_load_motion_driver_firmware(). This pushes the DMP image in
     *    inv_mpu_dmp_motion_driver.h into the MPU memory.
     * 2. Push the gyro and accel orientation matrix to the DMP.
     * 3. Register gesture callbacks. Don't worry, these callbacks won't be
     *    executed unless the corresponding feature is enabled.
     * 4. Call dmp_enable_feature(mask) to enable different features.
     * 5. Call dmp_set_fifo_rate(freq) to select a DMP output rate.
     * 6. Call any feature-specific control functions.
     *
     * To enable the DMP, just call mpu_set_dmp_state(1). This function can
     * be called repeatedly to enable and disable the DMP at runtime.
     *
     * The following is a short summary of the features supported in the DMP
     * image provided in inv_mpu_dmp_motion_driver.c:
     * DMP_FEATURE_LP_QUAT: Generate a gyro-only quaternion on the DMP at
     * 200Hz. Integrating the gyro data at higher rates reduces numerical
     * errors (compared to integration on the MCU at a lower sampling rate).
     * DMP_FEATURE_6X_LP_QUAT: Generate a gyro/accel quaternion on the DMP at
     * 200Hz. Cannot be used in combination with DMP_FEATURE_LP_QUAT.
     * DMP_FEATURE_TAP: Detect taps along the X, Y, and Z axes.
     * DMP_FEATURE_ANDROID_ORIENT: Google's screen rotation algorithm. Triggers
     * an event at the four orientations where the screen should rotate.
     * DMP_FEATURE_GYRO_CAL: Calibrates the gyro data after eight seconds of
     * no motion.
     * DMP_FEATURE_SEND_RAW_ACCEL: Add raw accelerometer data to the FIFO.
     * DMP_FEATURE_SEND_RAW_GYRO: Add raw gyro data to the FIFO.
     * DMP_FEATURE_SEND_CAL_GYRO: Add calibrated gyro data to the FIFO. Cannot
     * be used in combination with DMP_FEATURE_SEND_RAW_GYRO.
     */
    result += dmp_load_motion_driver_firmware();
    result += dmp_set_orientation(inv_orientation_matrix_to_scalar(gyro_orientation));
    result += dmp_register_tap_cb(tap_cb);
    result += dmp_register_android_orient_cb(android_orient_cb);
    hal.dmp_features = DMP_FEATURE_6X_LP_QUAT | DMP_FEATURE_TAP |
        DMP_FEATURE_ANDROID_ORIENT | DMP_FEATURE_SEND_RAW_ACCEL | DMP_FEATURE_SEND_CAL_GYRO |
        DMP_FEATURE_GYRO_CAL;
    result += dmp_enable_feature(hal.dmp_features);
    result += dmp_set_fifo_rate(DEFAULT_MPU_HZ);
    result += mpu_set_dmp_state(1);
    hal.dmp_on = 1;
		/************** 3. Enable the Interrupt now **************************/
    MPU9150_Interrupt_Cmd(ENABLE);
    
    while (1) {
      if (hal.new_gyro) {
        if (dmp_read_fifo(gyro, accel, quat, &sensor_timestamp, &sensors, &more) < 0)
          while(1);
        
        if (!more)
          hal.new_gyro = 0;
        
        for (int i = 0; i < 4; i++) {
          f_quat[i] = (float)quat[i];
        }
        
        float norm = MPUQuaternionNorm(f_quat);
        
        for (int i = 0; i < 4; i++) {
          f_quat[i] /= norm;
        }
        
        MPUQuaternionQuaternionToEuler(f_quat, eulers);
      }
    }
    return;
}

void accel_cal()
{
    int result;
    unsigned char accel_fsr;
    unsigned short gyro_rate, gyro_fsr;
    //unsigned long timestamp;
    //struct int_param_s int_param;
    
    short accelMinX = 0x7fff;                    // mag min x value
    short accelMaxX = 0x8000;                    // mag max x value
    short accelMinY = 0x7fff;                      // mag min y value
    short accelMaxY = 0x8000;                    // mag max y value
    short accelMinZ = 0x7fff;                      // mag min z value
    short accelMaxZ = 0x8000;                    // mag max z value
    
    short gyro[3], accel[3], sensors;
    short compass[3];
    unsigned char more;
    long quat[4];
    float f_quat[4];
    float eulers[3];
    unsigned long sensor_timestamp;
    
    /************ 1. Initialize CPAL for I2C communication with MPU9150 *****/
    MPU9150_Config();
    if(MPU9150_GetStatus() != SUCCESS)
    {
      while(1);
    }
    /******** 2. Configure INT pin of STM32 for MPU9150 interrupts *********/
    MPU9150_Interrupt_Init(MPU9150_INT_MODE_EXTI); // AMM
    
    result = 0; // AMM
    
    /* If you're not using an MPU9150 AND you're not using DMP features, this
     * function will place all slaves on the primary bus.
     * mpu_set_bypass(1);
     */
    /* Get/set hardware configuration. Start gyro. */
    /* Wake up all sensors. */
    result += mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL);
    /* Push both gyro and accel data into the FIFO. */
    result += mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL);
    result += mpu_set_sample_rate(DEFAULT_MPU_HZ);
    result += mpu_set_gyro_fsr(2000);
    /* Read back configuration in case it was set improperly. */
    result += mpu_get_sample_rate(&gyro_rate);
    result += mpu_get_gyro_fsr(&gyro_fsr);
    result += mpu_get_accel_fsr(&accel_fsr);

    /* Initialize HAL state variables. */
    memset(&hal, 0, sizeof(hal));
    hal.sensors = ACCEL_ON | GYRO_ON;
    hal.report = PRINT_QUAT;
    
    result += dmp_load_motion_driver_firmware();
    result += dmp_set_orientation(inv_orientation_matrix_to_scalar(gyro_orientation));
    result += dmp_register_tap_cb(tap_cb);
    result += dmp_register_android_orient_cb(android_orient_cb);
    hal.dmp_features = DMP_FEATURE_6X_LP_QUAT | DMP_FEATURE_TAP |
        DMP_FEATURE_ANDROID_ORIENT | DMP_FEATURE_SEND_RAW_ACCEL | DMP_FEATURE_SEND_CAL_GYRO |
        DMP_FEATURE_GYRO_CAL;
    result += dmp_enable_feature(hal.dmp_features);
    result += dmp_set_fifo_rate(DEFAULT_MPU_HZ);
    result += mpu_set_dmp_state(1);
    hal.dmp_on = 1;
		/************** 3. Enable the Interrupt now **************************/
    MPU9150_Interrupt_Cmd(ENABLE);
    
    while (1) {
      if (hal.new_gyro) {
        if (dmp_read_fifo(gyro, accel, quat, &sensor_timestamp, &sensors, &more) < 0)
          while(1);
        
        if (!more)
          hal.new_gyro = 0;
        
        if (accel[VEC3_X] < accelMinX)
          accelMinX = accel[VEC3_X];
        if (accel[VEC3_X] > accelMaxX)
          accelMaxX = accel[VEC3_X];
        if (accel[VEC3_Y] < accelMinY)
          accelMinY = accel[VEC3_Y];
        if (accel[VEC3_Y] > accelMaxY)
          accelMaxY = accel[VEC3_Y];
        if (accel[VEC3_Z] < accelMinZ)
          accelMinZ = accel[VEC3_Z];
        if (accel[VEC3_Z] > accelMaxZ)
          accelMaxZ = accel[VEC3_Z];
      }
    }
    return;
}