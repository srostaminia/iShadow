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

/* Data requested by client. */
#define PRINT_ACCEL     (0x01)
#define PRINT_GYRO      (0x02)
#define PRINT_QUAT      (0x04)
#define PRINT_COMPASS   (0x08)

#define ACCEL_ON        (0x01)
#define GYRO_ON         (0x02)

#define MOTION          (0)
#define NO_MOTION       (1)

/* Starting sampling rate. */
#define DEFAULT_MPU_HZ  (100)

struct hal_s {
    unsigned char sensors;
    unsigned char dmp_on;
    unsigned char wait_for_tap;
    volatile unsigned char new_gyro;
    unsigned short report;
    unsigned short dmp_features;
    unsigned char motion_int_mode;
};
static struct hal_s hal = {0};

static signed char gyro_orientation[9] = {-1, 0, 0,
                                           0,-1, 0,
                                           0, 0, 1};

static __IO uint32_t TimingDelay;

int main()
{
  int result;
  
  if (SysTick_Config(SystemCoreClock / 1000)) {
    while (1);
  }
  
  config_us_delay();
  
  unsigned char *data;
  //stm32_i2c_write(0x0C << 1, 0x0A, 1, data);
  
  Delay(10);
  
//  MPU9150_Config();
//  if(MPU9150_GetStatus() != SUCCESS)
//  {
//    while (1);
//  }
  
//  unsigned char data[1] = { 0 };
//  result = stm32_i2c_read(0xD0, 0x75, 1, data);
  
  result = imu_test();
  
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