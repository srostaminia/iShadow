/*
 * File:   gaze_predict.h
 * Author: Addison
 *
 * Created on April 9, 2013, 11:05 AM
 */

#ifndef GAZE_PREDICT_H
#define	GAZE_PREDICT_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "stm32l1xx.h"
  
#define NUM_SUBSAMPLE 387
#define NUM_HIDDEN 6

#define BH(X)           *((float*)(model_data + bh_offset + ((X) * 2)))  
#define BO(X)           *((float*)(model_data + bo_offset + ((X) * 2)))
#define MASK(X, Y)      model_data[mask_offset + ((X) * 2) + (Y)]
#define WHO(X, Y)       *((float*)(model_data + who_offset + ((X) * 4) + ((Y) * 2)))
#define WIH(X, Y)       *((float*)(model_data + wih_offset + ((X) * num_hidden * 2) + ((Y) * 2)))
#define ROW_FPN(X)          model_data[fpn_offset + (X)]
#define COL_FPN(X)      model_data[col_fpn_offset + (X)]
  
void predict_gaze(unsigned short subsamples[], uint16_t min, uint16_t max);
void predict_gaze_fullimg(unsigned short img[], uint16_t min, uint16_t max);
void predict_gaze_fullmean(uint16_t img[], float mean, float std);
int finish_predict(float ah[6]);

float tanh_approx(float input);
float calc_std(uint16_t img[]);

#ifdef	__cplusplus
}
#endif

#endif	/* GAZE_PREDICT_H */

