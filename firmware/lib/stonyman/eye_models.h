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
#include "stdbool.h"
  
// #define NUM_SUBSAMPLE 2273
// #define NUM_HIDDEN 6

#define BH(X)           *((float*)(model_data + bh_offset + ((X) * 2)))  
#define BO(X)           *((float*)(model_data + bo_offset + ((X) * 2)))
#define MASK(X, Y)      model_data[mask_offset + ((X) * 2) + (Y)]
#define WHO(X, Y)       *((float*)(model_data + who_offset + ((X) * 4) + ((Y) * 2)))
#define WIH(X, Y)       *((float*)(model_data + wih_offset + ((X) * num_hidden * 2) + ((Y) * 2)))

#define MASK_ROW				0
#define MASK_COL				1

#define PRED_X					0
#define PRED_Y					1

#define CIDER_COL               0
#define CIDER_ROW               1

#ifdef COLUMN_MAJOR
 #define MASK_MAJOR			MASK_COL
 #define MASK_MINOR			MASK_ROW
#else
 #define MASK_MAJOR			MASK_ROW
 #define MASK_MINOR			MASK_COL
#endif // COLUMN_MAJOR

// CIDER parameters
#define SPEC_THRESH     150
#define CONV_OFFSET     4
#define CIDER_PERCENTILE        10      // Percentile value for cross model pixel clamping

typedef struct StreamStats {
  float M, S;
  int k;
  uint32_t current_subsample;
  uint32_t pixel_sum;
} StreamStats;

float tanh_approx(float input);
float calc_std(uint16_t img[]);

void read_cider_params();

void ann_predict(uint16_t *pixels);
void ann_predict2(uint16_t *pixels, StreamStats *ss);

// TODO: Decide whether to remove functions from eye_models based on stonyman_conf flags
void find_pupil_edge(uint8_t start_point, uint8_t* edges, uint16_t* pixels);
bool run_cider();
uint16_t quick_percentile(uint16_t *base_row);

#pragma inline=forced
void init_streamstats(StreamStats *ss)
{
    ss->pixel_sum = 0;
    ss->M = 0.0;
    ss->S = 0.0;
    ss->k = 1;
    ss->current_subsample = 0;
}

// #pragma inline=forced
// void update_streamstats(StreamStats *ss, uint16_t *pixels, uint16_t this_pixel, 
//                         int i_major, int j_minor, uint16_t mask_major, uint16_t mask_minor)
// {
//     float value, tmpM;

//     // Streaming mean / stdev computation on subsampled pixels
//     if (mask_major == i_major && mask_minor == j_minor)
//     {
//         pixels[ss->current_subsample] = this_pixel;
        
//        if (ss->current_subsample == 0)
//          tmpM = 0;

//        ss->pixel_sum += this_pixel;

//        // Standard deviation computation
//        value = (float)this_pixel;
//        tmpM = ss->M;
//        ss->M += (value - tmpM) / ss->k;
//        ss->S += (value - tmpM) * (value - ss->M);
//        ss->k++;
//        ss->current_subsample++;
//     }
// }

#pragma inline=forced
void update_streamstats(StreamStats *ss, uint16_t *pixels, uint16_t this_pixel)
{
    float value, tmpM;

    // Streaming mean / stdev computation on subsampled pixels
    pixels[ss->current_subsample] = this_pixel;

    ss->pixel_sum += this_pixel;

    // Standard deviation computation
    value = (float)this_pixel;
    tmpM = ss->M;
    ss->M += (value - tmpM) / ss->k;
    ss->S += (value - tmpM) * (value - ss->M);
    ss->k++;
    ss->current_subsample++;
}

#endif	/* EYE_MODEL_H */
