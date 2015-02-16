#include <intrinsics.h>
//#include <ST/iostm32L151xx.h>
#include "stm32l1xx.h"
#include "main.h"
#include "TestSDCard.h"
//#include "ff.h"
#include "libstony.h"
#include "stm32l152d_eval_sdio_sd.h"
#include "diskio.h"

static __IO uint32_t TimingDelay;
extern uint32_t sd_ptr;

volatile uint16_t time_start, time_total;

int main()
{  
  volatile uint16_t start, total;
  int result;
  
  if (SysTick_Config(SystemCoreClock / 1000)) {
    while (1);
  }
  
  config_us_delay();
  
//  stony_init(SMH_VREF_3V3, SMH_NBIAS_3V3, SMH_AOBIAS_3V3,
//            SMH_GAIN_3V3, SMH_SELAMP_3V3);
  
    stony_init(41, 50, 41,
            4, SMH_SELAMP_3V3);
  
  if (disk_initialize(0) != SD_OK)
    while(1);
  
  config_ms_timer();
  
  while(1) {
    if (stony_image_dual())
      while(1);
  }
  
//  stony_image_single();
//  uint8_t loc1[2] = {55, 56};
//  uint8_t loc2[2] = {100, 100};
  
//  pixel_mass_test(loc1);
  
//  single_pixel_test(loc1, 56, 0);
//  single_pixel_test(loc2, 56, 0);
//  single_line_test(55, 1);
//  
//  stony_image_single();
//  
//  ann_mask_test();
  
  return total;
}

int single_line_test(uint8_t rowcol_num, uint8_t rowcol_sel)
{
  uint8_t sd_buf[112 * 2 * 2];
  
  ADC_RegularChannelConfig(ADC1, SINGLE_PARAM(ADC_CHAN), 1, ADC_SampleTime_4Cycles);
  ADC_RegularChannelConfig(ADC1, SINGLE_PARAM(ADC_CHAN), 2, ADC_SampleTime_4Cycles);
  
  stony_cider_line(rowcol_num, sd_buf, rowcol_sel);
  stony_cider_line(rowcol_num - 1, sd_buf + 224, rowcol_sel);
  
  Delay(300);
  
  stony_cider_line(rowcol_num - 1, sd_buf + 224, rowcol_sel);
  stony_cider_line(rowcol_num, sd_buf + 224, rowcol_sel);
    
  if (disk_write_fast(0, sd_buf, sd_ptr, 1) != RES_OK)      return -1;
  f_finish_write();
  sd_ptr += 1;
  
  return 0;
}

int ann_mask_test()
{
  ADC_RegularChannelConfig(ADC1, SINGLE_PARAM(ADC_CHAN), 1, ADC_SampleTime_4Cycles);
  ADC_RegularChannelConfig(ADC1, SINGLE_PARAM(ADC_CHAN), 2, ADC_SampleTime_4Cycles);
  
  uint8_t row[] = {43, 54, 42, 54, 44, 46, 33, 39, 45, 46, 54, 32, 42, 41, 84, 12, 40, 42, 45, 53, 44, 46, 48, 53, 54, 78, 45, 48, 54, 69, 78, 79, 44, 46, 47, 48, 49, 50, 51, 52, 53, 66, 69, 70, 72, 79, 44, 48, 49, 50, 51, 70, 72, 73, 74, 77, 79, 48, 49, 50, 51, 52, 53, 70, 79, 92, 38, 48, 49, 50, 51, 52, 63, 69, 70, 72, 73, 77, 80, 81, 50, 51, 62, 66, 69, 70, 72, 74, 77, 81, 82, 83, 60, 63, 74, 76, 80, 81, 82, 83, 84, 61, 62, 63, 64, 65, 67, 69, 74, 75, 76, 77, 78, 81, 82, 83, 84, 102, 39, 41, 47, 60, 61, 62, 63, 64, 65, 66, 72, 74, 75, 77, 79, 80, 81, 82, 83, 84, 85, 86, 102, 61, 62, 63, 64, 65, 66, 67, 68, 74, 75, 77, 78, 79, 82, 83, 84, 85, 86, 95, 55, 56, 62, 63, 64, 66, 67, 68, 69, 82, 83, 84, 85, 86, 87, 52, 53, 55, 56, 62, 66, 67, 68, 70, 83, 84, 85, 86, 100, 53, 54, 55, 56, 57, 58, 61, 62, 63, 66, 67, 70, 84, 85, 86, 87, 100, 55, 56, 57, 62, 64, 65, 67, 78, 84, 85, 86, 87, 88, 104, 39, 42, 47, 49, 53, 54, 55, 56, 61, 64, 78, 84, 85, 86, 87, 88, 89, 103, 104, 40, 43, 46, 47, 48, 49, 52, 53, 54, 55, 56, 64, 66, 67, 85, 86, 87, 88, 89, 102, 103, 104, 38, 39, 40, 42, 47, 48, 49, 52, 53, 54, 55, 58, 59, 61, 63, 86, 87, 88, 89, 102, 103, 104, 105, 38, 39, 40, 41, 44, 46, 47, 48, 57, 59, 61, 62, 64, 65, 66, 68, 86, 87, 88, 89, 102, 37, 39, 41, 47, 48, 49, 52, 53, 57, 58, 59, 60, 61, 62, 63, 64, 67, 86, 87, 88, 89, 102, 108, 36, 38, 40, 41, 43, 44, 45, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 69, 70, 73, 87, 88, 89, 90, 91, 106, 108, 110, 36, 37, 38, 39, 40, 41, 42, 43, 44, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 70, 73, 77, 87, 88, 89, 90, 105, 106, 107, 108, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 87, 88, 89, 90, 91, 105, 106, 107, 108, 109, 36, 37, 38, 39, 40, 41, 42, 43, 44, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 77, 88, 89, 90, 91, 104, 105, 106, 107, 108, 109, 36, 37, 38, 39, 40, 41, 42, 43, 44, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 79, 88, 89, 90, 91, 92, 104, 105, 106, 107, 108, 109, 110, 36, 38, 39, 40, 41, 45, 47, 49, 50, 51, 52, 53, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 89, 90, 91, 104, 105, 106, 107, 108, 109, 36, 37, 38, 39, 40, 41, 42, 47, 48, 49, 50, 51, 52, 53, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 87, 89, 90, 91, 103, 105, 107, 108, 109, 110, 36, 37, 38, 39, 47, 49, 50, 51, 52, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 89, 90, 91, 92, 108, 109, 36, 37, 38, 39, 40, 41, 49, 50, 51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 65, 66, 67, 72, 73, 74, 75, 76, 77, 78, 79, 80, 89, 90, 91, 105, 106, 107, 108, 109, 110, 35, 36, 37, 38, 39, 40, 42, 53, 56, 57, 58, 59, 60, 61, 62, 64, 65, 66, 71, 72, 73, 74, 76, 77, 78, 79, 89, 90, 91, 92, 107, 108, 109, 110, 35, 36, 37, 38, 39, 40, 41, 49, 53, 55, 57, 58, 59, 60, 64, 65, 71, 72, 73, 75, 76, 77, 78, 79, 89, 90, 91, 106, 107, 108, 109, 110, 34, 35, 36, 37, 38, 39, 40, 41, 42, 53, 59, 60, 61, 74, 76, 77, 78, 79, 89, 90, 91, 92, 93, 107, 108, 109, 110, 111, 34, 36, 37, 38, 39, 40, 41, 51, 58, 59, 60, 61, 75, 76, 77, 78, 79, 89, 90, 91, 108, 109, 110, 111, 35, 36, 37, 38, 39, 40, 42, 50, 59, 74, 75, 76, 77, 78, 79, 80, 89, 90, 91, 92, 106, 107, 109, 110, 111, 36, 37, 38, 39, 40, 44, 47, 48, 49, 50, 51, 54, 62, 71, 75, 76, 77, 78, 81, 84, 89, 90, 91, 93, 108, 109, 110, 111, 19, 34, 35, 37, 38, 39, 40, 47, 48, 51, 72, 74, 75, 76, 77, 78, 79, 81, 89, 90, 91, 107, 108, 109, 110, 111, 35, 36, 37, 38, 39, 40, 41, 43, 48, 50, 75, 76, 77, 78, 79, 88, 89, 90, 91, 107, 108, 109, 110, 111, 34, 35, 36, 37, 38, 39, 40, 52, 53, 54, 72, 74, 75, 76, 77, 78, 89, 90, 91, 92, 93, 108, 109, 110, 111, 34, 36, 37, 38, 39, 40, 46, 50, 73, 74, 75, 76, 77, 78, 79, 80, 90, 91, 108, 109, 110, 111, 36, 37, 38, 39, 40, 42, 43, 44, 45, 53, 74, 76, 77, 78, 90, 91, 109, 110, 111, 33, 34, 35, 36, 37, 38, 39, 40, 42, 48, 50, 51, 52, 75, 76, 77, 78, 89, 90, 91, 110, 111, 34, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 47, 48, 51, 52, 53, 58, 59, 75, 76, 77, 78, 79, 89, 90, 91, 110, 111, 35, 36, 37, 38, 39, 40, 41, 42, 44, 52, 56, 57, 58, 59, 60, 61, 74, 75, 76, 77, 78, 89, 90, 91, 109, 110, 111, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 49, 51, 52, 53, 54, 56, 57, 58, 59, 60, 61, 74, 75, 76, 77, 78, 79, 88, 89, 90, 91, 109, 110, 111, 35, 36, 37, 38, 39, 40, 41, 43, 45, 47, 49, 51, 52, 53, 54, 56, 57, 58, 59, 60, 61, 62, 74, 75, 76, 77, 78, 79, 89, 90, 110, 111, 35, 36, 37, 38, 39, 40, 41, 43, 44, 46, 47, 48, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 74, 75, 76, 77, 79, 85, 87, 89, 90, 91, 111, 37, 38, 39, 40, 41, 42, 50, 51, 52, 53, 56, 57, 58, 59, 60, 61, 62, 63, 71, 72, 73, 74, 75, 76, 77, 78, 88, 89, 90, 91, 109, 110, 111, 38, 39, 40, 41, 44, 45, 47, 49, 50, 51, 52, 55, 56, 57, 58, 59, 60, 61, 62, 63, 71, 72, 73, 74, 75, 76, 77, 78, 88, 89, 90, 107, 109, 110, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 48, 49, 50, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 70, 72, 73, 74, 75, 76, 77, 78, 87, 88, 89, 90, 111, 33, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 53, 54, 55, 56, 57, 58, 60, 61, 62, 63, 64, 66, 70, 71, 72, 73, 74, 75, 76, 77, 78, 87, 88, 89, 92, 104, 111, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 71, 72, 74, 75, 76, 77, 78, 87, 88, 89, 105, 110, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 51, 52, 53, 54, 55, 56, 57, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 71, 73, 74, 75, 76, 77, 86, 87, 88, 89, 106, 108, 109, 38, 40, 42, 43, 44, 45, 46, 49, 50, 51, 52, 53, 54, 55, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 74, 75, 76, 85, 87, 105, 109, 37, 40, 42, 43, 44, 47, 50, 51, 52, 53, 54, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 73, 74, 75, 76, 81, 85, 87, 88, 106, 107, 108, 109, 110, 111, 41, 42, 43, 45, 46, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 63, 64, 65, 66, 67, 68, 71, 72, 73, 74, 75, 76, 77, 81, 87, 106, 107, 108, 109, 110, 111, 37, 38, 39, 40, 41, 42, 44, 45, 46, 48, 49, 50, 51, 52, 53, 54, 56, 57, 58, 59, 60, 61, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 80, 81, 87, 107, 108, 109, 110, 40, 41, 42, 43, 45, 47, 48, 49, 50, 51, 52, 53, 54, 57, 58, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 79, 81, 83, 88, 101, 107, 108, 109, 110, 111, 37, 39, 41, 42, 45, 46, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 62, 63, 64, 65, 66, 67, 68, 69, 71, 72, 73, 74, 75, 76, 77, 78, 79, 103, 108, 109, 110, 111, 38, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 62, 64, 65, 67, 68, 72, 73, 74, 76, 77, 78, 79, 106, 107, 108, 109, 110, 111, 38, 40, 41, 42, 43, 44, 45, 46, 47, 49, 50, 51, 52, 54, 55, 56, 57, 58, 59, 73, 74, 76, 79, 106, 107, 108, 109, 110, 111, 42, 46, 48, 49, 50, 54, 74, 75, 105, 107, 108, 110, 39, 40, 41, 42, 48, 49, 51, 56, 75, 76, 107, 110, 111, 38, 39, 40, 41, 42, 43, 44, 48, 51, 74, 75, 76, 78, 107, 108, 109, 110, 111, 37, 39, 40, 41, 42, 43, 45, 46, 48, 49, 50, 52, 74, 108, 110, 111, 41, 42, 44, 45, 49, 50, 56, 107, 108, 109, 110, 111, 40, 41, 42, 43, 45, 49, 100, 108, 109, 111, 41, 42, 44, 45, 48, 72, 110, 41, 42, 43, 44, 50, 100, 101, 102, 109, 111, 29, 41, 42, 43, 44, 45, 47, 48, 49, 60, 99, 100, 101, 102, 111, 40, 41, 42, 43, 44, 46, 47, 48, 100, 102, 104, 41, 43, 44, 45, 46, 48, 50, 103, 104, 105, 109, 110, 111, 45, 50, 52, 103, 104, 111, 41, 42, 48, 103, 30, 33, 41, 42, 43, 45, 50, 104, 106, 109, 40, 42, 50, 51, 106, 40, 41, 42, 48, 49, 103, 42, 43, 44, 48, 64, 107, 108, 43, 44, 48, 106, 107, 110, 33, 42, 44, 46, 61, 62, 63, 99, 104, 109, 111, 44, 46, 48, 62, 63, 109, 110, 111, 43, 45, 46, 48, 60, 61, 62, 63, 64, 107, 108, 38, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 53, 59, 60, 61, 62, 63, 78, 108, 111, 43, 44, 45, 46, 47, 48, 49, 50, 51, 53, 60, 62, 65, 101, 106, 107, 111, 39, 43, 44, 45, 46, 47, 49, 50, 51, 56, 62, 67, 93, 104, 39, 41, 42, 43, 44, 45, 46, 50, 51, 52, 53, 54, 55, 56, 59, 60, 61, 68, 82, 83, 106, 108, 110, 43, 44, 45, 46, 50, 51, 56, 57, 59, 72, 77, 83, 96, 98, 102, 103, 107, 43, 45, 47, 48, 50, 59, 60, 98, 44, 45, 46, 47, 48, 49, 50, 51, 59, 60, 47, 48, 49, 50, 51, 52, 57, 50, 51, 52, 59, 54, 55, 56, 59, 74, 81, 54, 55, 56, 72, 100, 55, 57, 66, 52, 54, 56, 59, 69, 108, 56, 62, 69, 61, 61, 63, 65};
  uint8_t col[] = {1, 2, 3, 3, 5, 5, 6, 6, 6, 6, 6, 10, 11, 12, 12, 13, 13, 14, 14, 14, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 19, 19, 19, 19, 19, 19, 19, 19, 19, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 81, 81, 81, 81, 81, 81, 81, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 86, 86, 86, 86, 86, 86, 87, 87, 87, 87, 88, 88, 88, 88, 88, 88, 88, 88, 88, 88, 89, 89, 89, 89, 89, 90, 90, 90, 90, 90, 90, 91, 91, 91, 91, 91, 91, 91, 92, 92, 92, 92, 92, 92, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 94, 94, 94, 94, 94, 94, 94, 94, 95, 95, 95, 95, 95, 95, 95, 95, 95, 95, 95, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 101, 101, 101, 101, 101, 101, 101, 101, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 103, 103, 103, 103, 103, 103, 103, 104, 104, 104, 104, 105, 105, 105, 105, 105, 105, 106, 106, 106, 106, 106, 107, 107, 107, 108, 108, 108, 108, 108, 108, 109, 109, 109, 110, 111, 111, 111};
  uint32_t num_pixels = 2087;
  
  uint8_t sd_buf[8704];
  uint16_t *buf16 = (uint16_t *)sd_buf;
  for (int i = 0; i < 4352; i++)
    buf16[i] = 3;
  
  stony_mask_fullsample(row, col, num_pixels, sd_buf);
  Delay(500);
  stony_mask_fullsample(row, col, num_pixels, (sd_buf + 4174));
//  stony_mask_fullsample(row, col, num_pixels, (sd_buf + 282));
  
  uint8_t num_blocks = 17;
  if (disk_write_fast(0, sd_buf, sd_ptr, num_blocks) != RES_OK)      return -1;
  f_finish_write();
  sd_ptr += num_blocks;;
}

int pixel_mass_test(uint8_t loc[2])
{
//   112 pixels per row, TX_ROWS rows per data transfer, 2 bytes per row, only 1 camera
//   Double-buffered (2-dim array)
  uint8_t sd_buf[112 * 112 * 2];
  uint8_t buf_idx = 0;
  
  ADC_RegularChannelConfig(ADC1, SINGLE_PARAM(ADC_CHAN), 1, ADC_SampleTime_4Cycles);
  ADC_RegularChannelConfig(ADC1, SINGLE_PARAM(ADC_CHAN), 2, ADC_SampleTime_4Cycles);
  
//  stony_single_pixel(loc, 112 * 56, 18, sd_buf);
//  stony_single_pixel(loc, 1, 0, sd_buf);
  
//  Delay(200);
  
  uint8_t loc2[2] = {55, 55};
//  stony_single_pixel(loc2, 112 * 56, 18, (sd_buf + 112 * 56 * 2));
  stony_single_pixel(loc2, 112 * 112, 0, sd_buf);

  for (int i = 0; i < 2; i++) {
    if (disk_write_fast(0, sd_buf + TX_BLOCKS / 2 * 512 * i, sd_ptr, TX_BLOCKS / 2) != RES_OK)      return -1;
    f_finish_write();
    sd_ptr += TX_BLOCKS / 2;  
  }
  
#if (112 % TX_ROWS != 0)
  f_finish_write();
  
  if (disk_write_fast(0, (uint8_t *)sd_buf + TX_BLOCKS * 512, sd_ptr, TX_MOD_BLOCKS / 2) != RES_OK)      return -1;
  sd_ptr += TX_MOD_BLOCKS / 2;
#endif
  
  f_finish_write();
  
  return 0;
}

int single_pixel_test(uint8_t loc[2], uint16_t lines, uint16_t delay)
{
  //   112 pixels per row, TX_ROWS rows per data transfer, 2 bytes per row, only 1 camera
//   Double-buffered (2-dim array)
  uint8_t sd_buf[2][TX_ROWS][112 * 2];
  uint8_t buf_idx = 0;
  
  ADC_RegularChannelConfig(ADC1, SINGLE_PARAM(ADC_CHAN), 1, ADC_SampleTime_4Cycles);
  ADC_RegularChannelConfig(ADC1, SINGLE_PARAM(ADC_CHAN), 2, ADC_SampleTime_4Cycles);
  
  for (int i = 0, buf_line = 1; i < lines; i++, buf_line++) {
    stony_single_pixel(loc, 112, delay, sd_buf[buf_idx][buf_line - 1]);
    
    if (buf_line == TX_ROWS) {
      buf_line = 0;
      if (i > TX_ROWS - 1) {
        f_finish_write();
      }
      
      if (disk_write_fast(0, (uint8_t *)sd_buf[buf_idx], sd_ptr, TX_BLOCKS / 2) != RES_OK)      return -1;
      
      buf_idx = !buf_idx;
      
      sd_ptr += TX_BLOCKS / 2;
//      if (row == 31)    return 0;
    }
  }
  
#if (112 % TX_ROWS != 0)
  f_finish_write();
  
  if (disk_write_fast(0, (uint8_t *)sd_buf[buf_idx], sd_ptr, TX_MOD_BLOCKS / 2) != RES_OK)      return -1;
  sd_ptr += TX_MOD_BLOCKS / 2;
#endif
  
  f_finish_write();
  
  return 0;
}

int cider_line_test(int8_t rowcol_num, int8_t rowcol_sel)
{
  //   112 pixels per row, TX_ROWS rows per data transfer, 2 bytes per row, only 1 camera
//   Double-buffered (2-dim array)
  uint8_t sd_buf[2][TX_ROWS][112 * 2];
  uint8_t buf_idx = 0;
  uint16_t this_rowcol;
  
  ADC_RegularChannelConfig(ADC1, SINGLE_PARAM(ADC_CHAN), 1, ADC_SampleTime_4Cycles);
  ADC_RegularChannelConfig(ADC1, SINGLE_PARAM(ADC_CHAN), 2, ADC_SampleTime_4Cycles);
  
  for (int i = 0, buf_line = 1; i < 112; i++, buf_line++) {
    this_rowcol = (rowcol_num < 0) ? (i) : (rowcol_num);
    stony_cider_line(this_rowcol, sd_buf[buf_idx][buf_line - 1], rowcol_sel);
    
    if (buf_line == TX_ROWS) {
      buf_line = 0;
      if (i > TX_ROWS - 1) {
        f_finish_write();
      }
      
      if (disk_write_fast(0, (uint8_t *)sd_buf[buf_idx], sd_ptr, TX_BLOCKS / 2) != RES_OK)      return -1;
      
      buf_idx = !buf_idx;
      
      sd_ptr += TX_BLOCKS / 2;
//      if (row == 31)    return 0;
    }
  }
  
#if (112 % TX_ROWS != 0)
  f_finish_write();
  
  if (disk_write_fast(0, (uint8_t *)sd_buf[buf_idx], sd_ptr, TX_MOD_BLOCKS / 2) != RES_OK)      return -1;
  sd_ptr += TX_MOD_BLOCKS / 2;
#endif
  
  f_finish_write();
  
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

#pragma inline=never
void delay_us(int delayTime)
{
//  uint16_t start = TIM5->CNT;
//  while((uint16_t)(TIM5->CNT - start) <= delayTime);
  
  for (int i = 0; i < delayTime; i++) {
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
  }
}