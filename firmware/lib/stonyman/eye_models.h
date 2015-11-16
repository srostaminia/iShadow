/*
 * File:   gaze_predict.h
 * Author: Addison
 *
 * Created on April 9, 2013, 11:05 AM
 */

#ifndef EYE_MODEL_H
#define	EYE_MODEL_H

#include "stm32l1xx.h"
#include "stonyman.h"
  
// #define NUM_SUBSAMPLE 2273
// #define NUM_HIDDEN 6

#define BH(X)           *((float*)(model_data + bh_offset + ((X) * 2)))  
#define BO(X)           *((float*)(model_data + bo_offset + ((X) * 2)))
#define MASK(X, Y)      model_data[mask_offset + ((X) * 2) + (Y)]
#define WHO(X, Y)       *((float*)(model_data + who_offset + ((X) * 4) + ((Y) * 2)))
#define WIH(X, Y)       *((float*)(model_data + wih_offset + ((X) * num_hidden * 2) + ((Y) * 2)))

#define MASK_ROW				0
#define MASK_COL				1

#ifdef COLUMN_COLLECT
 #define MASK_OUTER			MASK_COL
 #define MASK_INNER			MASK_ROW
#else
 #define MASK_OUTER			MASK_ROW
 #define MASK_INNER			MASK_COL
#endif // COLUMN_COLLECT

typedef struct StreamStats {
  float M, S;
  int k;
  uint32_t current_subsample;
  uint32_t pixel_sum;
} StreamStats;

float tanh_approx(float input);
float calc_std(uint16_t img[]);

void read_cider_params();
void init_streamstats(StreamStats *ss);
void update_streamstats(StreamStats *ss, uint16_t *pixels, uint16_t this_pixel, int i_outer, int j_inner);

void ann_predict(uint16_t *pixels, StreamStats *ss);

#endif	/* EYE_MODEL_H */
